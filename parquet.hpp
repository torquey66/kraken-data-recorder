#pragma once

#include "responses.hpp"

#include <arrow/api.h>
#include <arrow/io/file.h>
#include <arrow/util/type_fwd.h>
#include <parquet/api/writer.h>
#include <parquet/arrow/writer.h>
#include <parquet/stream_writer.h>

#include <memory>
#include <string>

namespace krakpot {
namespace pq {

struct trades_sink_t final {
  trades_sink_t(std::string parquet_dir);

  void accept(const response::trades_t &);

private:
  static std::shared_ptr<arrow::Schema> schema();

  static std::shared_ptr<arrow::io::FileOutputStream>
  open_trades_file(std::string trades_filename);

  static std::unique_ptr<parquet::arrow::FileWriter>
  open_writer(std::shared_ptr<arrow::io::FileOutputStream> trades_file);

  std::string m_parquet_dir;
  std::string m_trades_filename;
  std::shared_ptr<arrow::io::FileOutputStream> m_trades_file;
  std::unique_ptr<parquet::arrow::FileWriter> m_os;

  arrow::UInt8Builder m_ord_type_builder;
  arrow::DoubleBuilder m_price_builder;
  arrow::DoubleBuilder m_qty_builder;
  arrow::UInt8Builder m_side_builder;
  arrow::StringBuilder m_symbol_builder;
  arrow::UInt64Builder m_timestamp_builder;
  arrow::UInt64Builder m_trade_id_builder;
};

} // namespace pq
} // namespace krakpot
