/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "io.hpp"
#include "responses.hpp"

#include <arrow/api.h>

#include <memory>
#include <string>

namespace krakpot {
namespace pq {

struct trades_sink_t final {
  static constexpr char c_sink_name[] = "trades";

  trades_sink_t(std::string parquet_dir, sink_id_t);

  void accept(const response::trades_t&);

 private:
  void reset_builders();

  static std::shared_ptr<arrow::Schema> schema();

  std::shared_ptr<arrow::Schema> m_schema;
  std::string m_sink_filename;
  writer_t m_writer;

  arrow::Int64Builder m_recv_tm_builder;
  arrow::StringBuilder m_ord_type_builder;
  arrow::StringBuilder m_price_builder;
  arrow::StringBuilder m_qty_builder;
  arrow::StringBuilder m_side_builder;
  arrow::StringBuilder m_symbol_builder;
  arrow::Int64Builder m_timestamp_builder;
  arrow::UInt64Builder m_trade_id_builder;
};

}  // namespace pq
}  // namespace krakpot
