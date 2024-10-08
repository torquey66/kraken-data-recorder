#include <doctest/doctest.h>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/io/memory.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>

#include <filesystem>
#include <iostream>

TEST_SUITE("parquet") {

  void dump_parquet(const std::string &filename) {
    auto pool = arrow::default_memory_pool();
    auto result = arrow::io::ReadableFile::Open(filename);
    REQUIRE(result.ok());
    std::shared_ptr<arrow::io::RandomAccessFile> input = result.ValueOrDie();

    std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
    PARQUET_THROW_NOT_OK(parquet::arrow::OpenFile(input, pool, &arrow_reader));

    std::shared_ptr<arrow::Table> table;
    PARQUET_THROW_NOT_OK(arrow_reader->ReadTable(&table));
    std::cout << table->ToString() << std::endl;
  }

  struct test_output_stream_t final {

    test_output_stream_t() {
      auto result = arrow::io::FileOutputStream::Open(
          std::filesystem::temp_directory_path() / "test.pq");
      REQUIRE(result.ok());
      m_os = result.ValueOrDie();
    }

    ~test_output_stream_t() {
      REQUIRE(m_os->Close().ok());
      REQUIRE(m_os->closed());
    }

    auto operator()() { return m_os; }
    auto operator()() const { return m_os; }

  private:
    std::shared_ptr<arrow::io::FileOutputStream> m_os;
  };

  struct mock_writer_t final {
    mock_writer_t(const arrow::Schema &schema)
        : m_writer_props{parquet::WriterProperties::Builder()
                             .max_row_group_length(64 * 1024)
                             ->created_by("KDR test")
                             ->version(parquet::ParquetVersion::PARQUET_2_6)
                             ->data_page_version(
                                 parquet::ParquetDataPageVersion::V2)
                             ->compression(parquet::Compression::SNAPPY)
                             ->build()},
          m_arrow_props{parquet::ArrowWriterProperties::Builder()
                            .store_schema()
                            ->build()},
          m_writer{parquet::arrow::FileWriter::Open(
                       schema, arrow::default_memory_pool(), m_tos(),
                       m_writer_props, m_arrow_props)
                       .ValueOrDie()} {}

    const parquet::arrow::FileWriter &writer() const { return *m_writer; }
    parquet::arrow::FileWriter &writer() { return *m_writer; }

    void write(const arrow::RecordBatch &batch) {
      const auto result = m_writer->WriteRecordBatch(batch);
      REQUIRE(result.ok());
    }

  private:
    test_output_stream_t m_tos;
    std::shared_ptr<parquet::WriterProperties> m_writer_props;
    std::shared_ptr<parquet::ArrowWriterProperties> m_arrow_props;
    std::unique_ptr<parquet::arrow::FileWriter> m_writer;
  };

  TEST_CASE("sanity check - simple scalar") {
    const auto fv = arrow::FieldVector{
        arrow::field("int_field", arrow::int64(), false),
    };
    const auto schema = std::make_shared<arrow::Schema>(fv);
    REQUIRE(schema);
    auto mw = mock_writer_t{*schema};

    auto fld_builder = std::make_shared<arrow::Int64Builder>();
    fld_builder->Reset();

    auto result = fld_builder->Append(42);
    REQUIRE(result.ok());

    std::shared_ptr<arrow::Array> fld_array;
    result = fld_builder->Finish(&fld_array);
    REQUIRE(result.ok());

    const auto columns = std::vector<std::shared_ptr<arrow::Array>>{fld_array};
    std::shared_ptr<arrow::RecordBatch> batch =
        arrow::RecordBatch::Make(schema, 1, columns);
    mw.write(*batch);
  }

  TEST_CASE("sanity check - two columns") {
    const auto fv = arrow::FieldVector{
        arrow::field("int_field", arrow::int64(), false),
        arrow::field("str_field", arrow::utf8(), false),
    };
    const auto schema = std::make_shared<arrow::Schema>(fv);
    REQUIRE(schema);
    auto mw = mock_writer_t{*schema};

    auto int_builder = std::make_shared<arrow::Int64Builder>();
    int_builder->Reset();

    auto result = int_builder->Append(42);
    REQUIRE(result.ok());

    std::shared_ptr<arrow::Array> int_array;
    result = int_builder->Finish(&int_array);
    REQUIRE(result.ok());

    auto str_builder = std::make_shared<arrow::StringBuilder>();
    str_builder->Reset();

    result = str_builder->Append("forty-two");
    REQUIRE(result.ok());

    std::shared_ptr<arrow::Array> str_array;
    result = str_builder->Finish(&str_array);
    REQUIRE(result.ok());

    const auto columns =
        std::vector<std::shared_ptr<arrow::Array>>{int_array, str_array};
    std::shared_ptr<arrow::RecordBatch> batch =
        arrow::RecordBatch::Make(schema, 1, columns);
    mw.write(*batch);
  }

  TEST_CASE("sanity check - struct") {
    const auto sfv =
        arrow::FieldVector{arrow::field("int_field", arrow::int64(), false),
                           arrow::field("str_field", arrow::utf8(), false)};
    const auto sfield = arrow::struct_(sfv);
    const auto fv = arrow::FieldVector{
        arrow::field("struct_field", sfield, false),
    };
    const auto schema = std::make_shared<arrow::Schema>(fv);
    REQUIRE(schema);

    auto mw = mock_writer_t{*schema};

    auto int_builder = std::make_shared<arrow::Int64Builder>();
    auto result = int_builder->Append(42);
    REQUIRE(result.ok());

    auto str_builder = std::make_shared<arrow::StringBuilder>();
    result = str_builder->Append("forty-two");
    REQUIRE(result.ok());

    auto struct_builder = std::make_shared<arrow::StructBuilder>(
        sfield, arrow::default_memory_pool(),
        std::vector<std::shared_ptr<arrow::ArrayBuilder>>{int_builder,
                                                          str_builder});
    PARQUET_THROW_NOT_OK(struct_builder->Append());
    PARQUET_THROW_NOT_OK(
        static_cast<arrow::Int64Builder *>(struct_builder->field_builder(0))
            ->Append(42));
    PARQUET_THROW_NOT_OK(
        static_cast<arrow::StringBuilder *>(struct_builder->field_builder(1))
            ->Append("forty-two"));

    std::shared_ptr<arrow::Array> struct_array;
    result = struct_builder->Finish(&struct_array);
    REQUIRE(result.ok());

    const auto columns =
        std::vector<std::shared_ptr<arrow::Array>>{struct_array};
    std::shared_ptr<arrow::RecordBatch> batch =
        arrow::RecordBatch::Make(schema, 1, columns);
    mw.write(*batch);
  }

  TEST_CASE("sanity check - list of struct") {
    const auto fv = arrow::FieldVector{
        arrow::field("list_field", arrow::list(arrow::int64()), false),
    };
    const auto schema = std::make_shared<arrow::Schema>(fv);
    REQUIRE(schema);

    auto mw = mock_writer_t{*schema};

    auto int_builder = std::make_shared<arrow::Int64Builder>();
    auto list_builder = std::make_shared<arrow::ListBuilder>(
        arrow::default_memory_pool(), int_builder);

    PARQUET_THROW_NOT_OK(list_builder->Reserve(1));
    PARQUET_THROW_NOT_OK(int_builder->Reserve(2));

    PARQUET_THROW_NOT_OK(list_builder->Append(2));
    PARQUET_THROW_NOT_OK(int_builder->Append(42));
    PARQUET_THROW_NOT_OK(int_builder->Append(43));

    std::shared_ptr<arrow::Array> result_array;
    PARQUET_THROW_NOT_OK(list_builder->Finish(&result_array));

    const auto columns =
        std::vector<std::shared_ptr<arrow::Array>>{result_array};
    std::shared_ptr<arrow::RecordBatch> batch =
        arrow::RecordBatch::Make(schema, 1, columns);
    mw.write(*batch);
  }

  TEST_CASE("sanity check - list of struct") {
    const auto sfv = arrow::FieldVector{
        arrow::field("int_field", arrow::int64(), false),
        arrow::field("double_field", arrow::float64(), false)};
    const auto sfield = arrow::struct_(sfv);

    const auto fv = arrow::FieldVector{
        arrow::field("list_field", arrow::list(sfield), false),
    };
    const auto schema = std::make_shared<arrow::Schema>(fv);
    REQUIRE(schema);
    auto mw = mock_writer_t{*schema};

    auto int_builder = std::make_shared<arrow::Int64Builder>();
    auto double_builder = std::make_shared<arrow::DoubleBuilder>();
    auto struct_builder = std::make_shared<arrow::StructBuilder>(
        sfield, arrow::default_memory_pool(),
        std::vector<std::shared_ptr<arrow::ArrayBuilder>>{int_builder,
                                                          double_builder});
    auto list_builder = std::make_shared<arrow::ListBuilder>(
        arrow::default_memory_pool(), struct_builder);

    PARQUET_THROW_NOT_OK(int_builder->Reserve(1));
    PARQUET_THROW_NOT_OK(double_builder->Reserve(1));
    PARQUET_THROW_NOT_OK(struct_builder->Reserve(1));
    PARQUET_THROW_NOT_OK(list_builder->Reserve(1));

    PARQUET_THROW_NOT_OK(list_builder->Append(1));
    PARQUET_THROW_NOT_OK(struct_builder->Append());
    PARQUET_THROW_NOT_OK(int_builder->Append(42));
    PARQUET_THROW_NOT_OK(double_builder->Append(42.42));

    std::shared_ptr<arrow::Array> list_array;
    PARQUET_THROW_NOT_OK(list_builder->Finish(&list_array));

    const auto columns = std::vector<std::shared_ptr<arrow::Array>>{list_array};
    std::shared_ptr<arrow::RecordBatch> batch =
        arrow::RecordBatch::Make(schema, 1, columns);
    mw.write(*batch);
  }
}
