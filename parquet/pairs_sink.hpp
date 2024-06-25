/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "responses.hpp"
#include "writer.hpp"

#include <arrow/api.h>

#include <memory>
#include <string>

namespace krakpot {
namespace pq {

struct pairs_sink_t final {
  static constexpr char c_sink_name[] = "pairs";

  pairs_sink_t(std::string parquet_dir, sink_id_t);

  void accept(const response::header_t&, const std::vector<response::pair_t>&);

 private:
  void reset_builders();

  static std::shared_ptr<arrow::Schema> schema();

  std::shared_ptr<arrow::Schema> m_schema;

  std::string m_parquet_dir;
  std::string m_pairs_filename;
  std::shared_ptr<arrow::io::FileOutputStream> m_pairs_file;
  std::unique_ptr<parquet::arrow::FileWriter> m_os;

  arrow::Int64Builder m_recv_tm_builder;
  arrow::StringBuilder m_base_builder;
  arrow::DoubleBuilder m_cost_min_builder;
  arrow::Int64Builder m_cost_precision_builder;
  arrow::BooleanBuilder m_has_index_builder;
  arrow::DoubleBuilder m_margin_initial_builder;
  arrow::BooleanBuilder m_marginable_builder;
  arrow::Int64Builder m_position_limit_long_builder;
  arrow::Int64Builder m_position_limit_short_builder;
  arrow::DoubleBuilder m_price_increment_builder;
  arrow::Int64Builder m_price_precision_builder;
  arrow::DoubleBuilder m_qty_increment_builder;
  arrow::DoubleBuilder m_qty_min_builder;
  arrow::Int64Builder m_qty_precision_builder;
  arrow::StringBuilder m_quote_builder;
  arrow::Int8Builder m_status_builder;
  arrow::StringBuilder m_symbol_builder;
};

} // namespace pq
} // namespace krakpot
