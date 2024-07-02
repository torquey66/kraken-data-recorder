/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "io.hpp"
#include "responses.hpp"

#include <arrow/api.h>

#include <memory>
#include <string>

namespace krakpot {
namespace pq {

struct book_sink_t final {
  static constexpr char c_sink_name[] = "book";

  book_sink_t(std::string parquet_dir, sink_id_t);

  void accept(const response::book_t&);

 private:
  void reset_builders();

  static std::shared_ptr<arrow::DataType> quote_struct();
  static std::shared_ptr<arrow::Schema> schema();

  std::shared_ptr<arrow::Schema> m_schema;
  std::string m_sink_filename;
  writer_t m_writer;

  std::shared_ptr<arrow::Int64Builder> m_recv_tm_builder;
  std::shared_ptr<arrow::StringBuilder> m_type_builder;

  std::shared_ptr<arrow::StringBuilder> m_bid_price_builder;
  std::shared_ptr<arrow::StringBuilder> m_bid_qty_builder;
  std::shared_ptr<arrow::StructBuilder> m_bid_builder;
  std::shared_ptr<arrow::ListBuilder> m_bids_builder;

  std::shared_ptr<arrow::StringBuilder> m_ask_price_builder;
  std::shared_ptr<arrow::StringBuilder> m_ask_qty_builder;
  std::shared_ptr<arrow::StructBuilder> m_ask_builder;
  std::shared_ptr<arrow::ListBuilder> m_asks_builder;

  std::shared_ptr<arrow::UInt64Builder> m_crc32_builder;
  std::shared_ptr<arrow::StringBuilder> m_symbol_builder;
  std::shared_ptr<arrow::Int64Builder> m_timestamp_builder;
};

}  // namespace pq
}  // namespace krakpot
