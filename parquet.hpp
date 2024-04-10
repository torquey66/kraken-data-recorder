#pragma once

#include "responses.hpp"

#include <arrow/io/file.h>
#include <parquet/stream_writer.h>

#include <memory>
#include <string>

namespace krakpot {
namespace pq {

struct trades_sink_t final {
  trades_sink_t(std::string parquet_dir);

  void accept(const response::trades_t &);

private:
  static std::shared_ptr<arrow::io::FileOutputStream>
  open_trades_file(std::string trades_filename);

  static std::unique_ptr<parquet::StreamWriter>
  open_stream(std::shared_ptr<arrow::io::FileOutputStream> trades_file);

  std::string m_parquet_dir;
  std::string m_trades_filename;
  std::shared_ptr<arrow::io::FileOutputStream> m_trades_file;
  std::unique_ptr<parquet::StreamWriter> m_os;
};

} // namespace pq
} // namespace krakpot
