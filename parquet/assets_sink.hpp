/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "io.hpp"
#include "responses.hpp"

#include <arrow/api.h>

#include <memory>
#include <string>

namespace krakpot {
namespace pq {

struct assets_sink_t final {
  static constexpr char c_sink_name[] = "assets";

  assets_sink_t(std::string parquet_dir, sink_id_t);

  void accept(const response::header_t&, const std::vector<response::asset_t>&);

 private:
  static std::shared_ptr<arrow::Schema> schema();

  std::shared_ptr<arrow::Schema> m_schema;
  std::string m_sink_filename;
  writer_t m_writer;

  arrow::Int64Builder m_recv_tm_builder;
  arrow::BooleanBuilder m_borrowable_builder;
  arrow::DoubleBuilder m_collateral_value_builder;
  arrow::StringBuilder m_id_builder;
  arrow::DoubleBuilder m_margin_rate_builder;
  arrow::Int64Builder m_precision_builder;
  arrow::Int64Builder m_precision_display_builder;
  arrow::Int8Builder m_status_builder;
};

}  // namespace pq
}  // namespace krakpot
