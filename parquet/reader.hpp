#pragma once

#include <constants.hpp>

#include <parquet/arrow/reader.h>

#include <memory>
#include <string>

namespace krakpot {
namespace pq {

using sink_id_t = int64_t;

/* !@# TODO: combine with writer.hpp version */
inline std::string source_filename(std::string parquet_dir,
                                   std::string sink_name,
                                   sink_id_t id) {
  return parquet_dir + "/" + std::to_string(id) + "." + sink_name + ".pq";
}

struct reader_t {
  reader_t(std::string source_filename)
      : m_reader_properties{arrow::default_memory_pool()} {
    m_reader_properties.set_buffer_size(4096 * 4);
    m_reader_properties.enable_buffered_stream();
    m_arrow_reader_properties.set_batch_size(128 * 1024);

    m_reader_builder.memory_pool(arrow::default_memory_pool());
    m_reader_builder.properties(m_arrow_reader_properties);
    PARQUET_THROW_NOT_OK(
        m_reader_builder.OpenFile(source_filename, false, m_reader_properties));
    PARQUET_ASSIGN_OR_THROW(m_arrow_file_reader, m_reader_builder.Build());
    PARQUET_THROW_NOT_OK(
        m_arrow_file_reader->GetRecordBatchReader(&m_record_batch_reader));
  }

  std::shared_ptr<::arrow::RecordBatchReader> record_batch_reader() {
    return m_record_batch_reader;
  }

 private:
  parquet::ReaderProperties m_reader_properties;
  parquet::ArrowReaderProperties m_arrow_reader_properties;
  parquet::arrow::FileReaderBuilder m_reader_builder;
  std::unique_ptr<parquet::arrow::FileReader> m_arrow_file_reader;
  std::shared_ptr<::arrow::RecordBatchReader> m_record_batch_reader;
};

}  // namespace pq
}  // namespace krakpot
