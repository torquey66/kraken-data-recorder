#pragma once

#include <constants.hpp>

#include <arrow/io/file.h>
#include <parquet/api/reader.h>
#include <parquet/api/writer.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>

#include <memory>
#include <string>

namespace kdr {
namespace pq {

using sink_id_t = int64_t;

/**
 * Utility for constructing a canonical parquet filename.
 */
inline std::string parquet_filename(std::string parquet_dir,
                                    std::string sink_name,
                                    sink_id_t id) {
  // !@# TODO: replace with std::filesystem constructs
  return parquet_dir + "/" + std::to_string(id) + "." + sink_name + ".pq";
}

/**
 * RAII wrapper for the Arrow machinery necessary to read parquet files.
 */
struct reader_t final {
  reader_t(std::string parquet_filename);

  std::shared_ptr<::arrow::RecordBatchReader> record_batch_reader() {
    return m_record_batch_reader;
  }

  std::shared_ptr<::arrow::Schema> get_schema() const {
    std::shared_ptr<::arrow::Schema> result;
    PARQUET_THROW_NOT_OK(m_arrow_file_reader->GetSchema(&result));
    return result;
  }

 private:
  parquet::ReaderProperties m_reader_properties;
  parquet::ArrowReaderProperties m_arrow_reader_properties;
  parquet::arrow::FileReaderBuilder m_reader_builder;
  std::unique_ptr<parquet::arrow::FileReader> m_arrow_file_reader;
  std::shared_ptr<::arrow::RecordBatchReader> m_record_batch_reader;
};

/**
 * RAII wrapper for the Arrow machinery necessary to write parquet files.
 */
struct writer_t final {
  writer_t(std::string parquet_filename, std::shared_ptr<arrow::Schema> schema);

  parquet::arrow::FileWriter& arrow_file_writer() {
    return *m_arrow_file_writer;
  }

 private:
  std::shared_ptr<parquet::WriterProperties> m_writer_properties;
  std::shared_ptr<parquet::ArrowWriterProperties> m_arrow_writer_properties;
  std::shared_ptr<arrow::io::FileOutputStream> m_file_output_stream;
  std::unique_ptr<parquet::arrow::FileWriter> m_arrow_file_writer;
};

/******************** I M P L E M E N T A T I O N ********************/

inline reader_t::reader_t(std::string parquet_filename)
    : m_reader_properties{arrow::default_memory_pool()} {
  m_reader_properties.set_buffer_size(4096 * 4);
  m_reader_properties.enable_buffered_stream();
  m_arrow_reader_properties.set_batch_size(128 * 1024);

  m_reader_builder.memory_pool(arrow::default_memory_pool());
  m_reader_builder.properties(m_arrow_reader_properties);
  PARQUET_THROW_NOT_OK(
      m_reader_builder.OpenFile(parquet_filename, false, m_reader_properties));
  PARQUET_ASSIGN_OR_THROW(m_arrow_file_reader, m_reader_builder.Build());
  PARQUET_THROW_NOT_OK(
      m_arrow_file_reader->GetRecordBatchReader(&m_record_batch_reader));
}

inline writer_t::writer_t(std::string parquet_filename,
                          std::shared_ptr<arrow::Schema> schema)
    : m_writer_properties{parquet::WriterProperties::Builder()
                              .max_row_group_length(64 * 1024)
                              ->created_by(c_app_name)
                              ->version(parquet::ParquetVersion::PARQUET_2_6)
                              ->data_page_version(
                                  parquet::ParquetDataPageVersion::V2)
                              ->compression(arrow::Compression::SNAPPY)
                              ->build()},
      m_arrow_writer_properties{
          parquet::ArrowWriterProperties::Builder().store_schema()->build()},
      m_file_output_stream{
          arrow::io::FileOutputStream::Open(parquet_filename).ValueOrDie()},
      m_arrow_file_writer{
          parquet::arrow::FileWriter::Open(*schema,
                                           arrow::default_memory_pool(),
                                           m_file_output_stream,
                                           m_writer_properties,
                                           m_arrow_writer_properties)
              .ValueOrDie()} {}

}  // namespace pq
}  // namespace kdr
