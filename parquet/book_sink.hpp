/* Copyright (C) 2024 John C. Finley - All rights reserved */
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

struct book_sink_t final {
  book_sink_t(std::string parquet_dir); // TODO: filesystem::path?

  void accept(const response::book_t &);

private:
  void reset_builders();

  static std::shared_ptr<arrow::DataType> quote_struct();
  static std::shared_ptr<arrow::Schema> schema();

  std::shared_ptr<arrow::Schema> m_schema;

  std::string m_parquet_dir;
  std::string m_book_filename;
  std::shared_ptr<arrow::io::FileOutputStream> m_book_file;
  std::unique_ptr<parquet::arrow::FileWriter> m_os;

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

} // namespace pq
} // namespace krakpot
