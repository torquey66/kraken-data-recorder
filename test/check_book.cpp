#include <arrow/array.h>
#include <arrow/io/api.h>
#include <arrow/record_batch.h>
#include <arrow/scalar.h>
#include <parquet/arrow/reader.h>

#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <book parquet file>" << std::endl;
    return -1;
  }

  const auto filename = std::string{argv[1]};

  auto *pool = arrow::default_memory_pool();
  auto reader_properties = parquet::ReaderProperties(pool);
  reader_properties.set_buffer_size(4096 * 4);
  reader_properties.enable_buffered_stream();

  auto arrow_reader_props = parquet::ArrowReaderProperties();
  arrow_reader_props.set_batch_size(128 * 1024);

  parquet::arrow::FileReaderBuilder reader_builder;
  PARQUET_THROW_NOT_OK(
      reader_builder.OpenFile(filename, false, reader_properties));
  reader_builder.memory_pool(pool);
  reader_builder.properties(arrow_reader_props);

  std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
  PARQUET_ASSIGN_OR_THROW(arrow_reader, reader_builder.Build());

  std::shared_ptr<::arrow::RecordBatchReader> rb_reader;
  PARQUET_THROW_NOT_OK(arrow_reader->GetRecordBatchReader(&rb_reader));

  for (arrow::Result<std::shared_ptr<arrow::RecordBatch>> maybe_batch :
       *rb_reader) {
    if (!maybe_batch.ok()) {
      std::cerr << "error: " << maybe_batch.status().ToString() << std::endl;
    }
    const auto &batch = *maybe_batch.ValueOrDie();
    const auto symbol_col = batch.GetColumnByName("symbol");
    for (auto idx = 0; idx < batch.num_rows(); ++idx) {
      std::shared_ptr<::arrow::Scalar> symbol_val;
      PARQUET_ASSIGN_OR_THROW(symbol_val, symbol_col->GetScalar(idx));
      //      std::cout << symbol_val->ToString() << std::endl;
      std::cout
          << static_cast<const ::arrow::StringScalar &>(*symbol_val).view()
          << std::endl;
    }
  }
}
