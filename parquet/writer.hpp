#pragma once

#include <constants.hpp>

#include <arrow/io/file.h>
#include <parquet/api/writer.h>
#include <parquet/arrow/writer.h>

#include <memory>
#include <string>

namespace krakpot {
namespace pq {

using sink_id_t = int64_t;

inline std::string sink_filename(std::string parquet_dir, std::string sink_name,
                                 sink_id_t id) {
  return parquet_dir + "/" + std::to_string(id) + "." + sink_name + ".pq";
}

inline std::shared_ptr<arrow::io::FileOutputStream>
open_sink_file(std::string sink_filename) {
  auto result = arrow::io::FileOutputStream::Open(sink_filename);
  if (result.ok()) {
    return result.ValueOrDie();
  }
  const auto msg = "failed to open: " + sink_filename +
                   " error: " + result.status().ToString();
  // !@#  BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << msg;
  throw std::runtime_error(msg);
}

inline std::unique_ptr<parquet::arrow::FileWriter>
open_writer(std::shared_ptr<arrow::io::FileOutputStream> sink_file,
            std::shared_ptr<arrow::Schema> schema) {

  std::shared_ptr<parquet::WriterProperties> props =
      parquet::WriterProperties::Builder()
          .max_row_group_length(64 * 1024)
          ->created_by(c_app_name)
          ->version(parquet::ParquetVersion::PARQUET_2_6)
          ->data_page_version(parquet::ParquetDataPageVersion::V2)
          ->compression(arrow::Compression::SNAPPY)
          ->build();

  std::shared_ptr<parquet::ArrowWriterProperties> arrow_props =
      parquet::ArrowWriterProperties::Builder().store_schema()->build();

  arrow::Result<std::unique_ptr<parquet::arrow::FileWriter>> result =
      parquet::arrow::FileWriter::Open(*schema, arrow::default_memory_pool(),
                                       sink_file, props, arrow_props);

  return std::move(result.ValueOrDie()); // TODO this is all really hinky
}

} // namespace pq
} // namespace krakpot
