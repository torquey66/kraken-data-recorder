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

/**
 * TODO: book_sink_t and trade_sink_t contain some duplicate code that will be
 * factored out in some fashion in future.
 */

struct book_sink_t final {
  book_sink_t(std::string parquet_dir);

  void accept(const response::book_t &);

private:
  void reset_builders();

  static std::shared_ptr<arrow::Schema> schema();

  // static std::shared_ptr<arrow::io::FileOutputStream>
  // open_book_file(std::string book_filename);

  //  static std::unique_ptr<parquet::arrow::FileWriter>
  //  open_writer(std::shared_ptr<arrow::io::FileOutputStream> book_file);

  std::shared_ptr<arrow::Schema> m_schema;

  std::string m_parquet_dir;
  std::string m_book_filename;
  std::shared_ptr<arrow::io::FileOutputStream> m_book_file;
  std::unique_ptr<parquet::arrow::FileWriter> m_os;

  arrow::Int64Builder m_recv_tm_builder;

  std::shared_ptr<arrow::DoubleBuilder> m_ask_px_builder;
  std::shared_ptr<arrow::DoubleBuilder> m_ask_qty_builder;
  arrow::MapBuilder m_asks_builder;

  arrow::UInt64Builder m_crc32_builder;
  arrow::StringBuilder m_symbol_builder;
  arrow::Int64Builder m_timestamp_builder;
};

struct trades_sink_t final {
  trades_sink_t(std::string parquet_dir);

  void accept(const response::trades_t &);

private:
  void reset_builders();

  static std::shared_ptr<arrow::Schema> schema();

  // static std::shared_ptr<arrow::io::FileOutputStream>
  // open_trades_file(std::string trades_filename);

  // static std::unique_ptr<parquet::arrow::FileWriter>
  // open_writer(std::shared_ptr<arrow::io::FileOutputStream> trades_file);

  std::shared_ptr<arrow::Schema> m_schema;

  std::string m_parquet_dir;
  std::string m_trades_filename;
  std::shared_ptr<arrow::io::FileOutputStream> m_trades_file;
  std::unique_ptr<parquet::arrow::FileWriter> m_os;

  arrow::Int64Builder m_recv_tm_builder;
  arrow::StringBuilder m_ord_type_builder;
  arrow::DoubleBuilder m_price_builder;
  arrow::DoubleBuilder m_qty_builder;
  arrow::StringBuilder m_side_builder;
  arrow::StringBuilder m_symbol_builder;
  arrow::Int64Builder m_timestamp_builder;
  arrow::UInt64Builder m_trade_id_builder;
};

} // namespace pq
} // namespace krakpot
