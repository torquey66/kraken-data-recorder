/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include "book_sink.hpp"

#include <arrow/scalar.h>
#include <boost/log/trivial.hpp>

namespace krakpot {
namespace pq {

book_sink_t::book_sink_t(std::string parquet_dir,
                         sink_id_t id,
                         integer_t book_depth)
    : m_schema{schema(book_depth)},
      m_sink_filename{parquet_filename(parquet_dir, c_sink_name, id)},
      m_writer{m_sink_filename, m_schema},
      m_recv_tm_builder{std::make_shared<arrow::Int64Builder>()},
      m_type_builder{std::make_shared<arrow::StringBuilder>()},
      m_bid_price_builder{std::make_shared<arrow::StringBuilder>()},
      m_bid_qty_builder{std::make_shared<arrow::StringBuilder>()},
      m_bid_builder{std::make_shared<arrow::StructBuilder>(
          quote_struct(),
          arrow::default_memory_pool(),
          std::vector<std::shared_ptr<arrow::ArrayBuilder>>{
              m_bid_price_builder, m_bid_qty_builder})},
      m_bids_builder{
          std::make_shared<arrow::ListBuilder>(arrow::default_memory_pool(),
                                               m_bid_builder)},
      m_ask_price_builder{std::make_shared<arrow::StringBuilder>()},
      m_ask_qty_builder{std::make_shared<arrow::StringBuilder>()},
      m_ask_builder{std::make_shared<arrow::StructBuilder>(
          quote_struct(),
          arrow::default_memory_pool(),
          std::vector<std::shared_ptr<arrow::ArrayBuilder>>{
              m_ask_price_builder, m_ask_qty_builder})},
      m_asks_builder{
          std::make_shared<arrow::ListBuilder>(arrow::default_memory_pool(),
                                               m_ask_builder)},
      m_crc32_builder{std::make_shared<arrow::UInt64Builder>()},
      m_symbol_builder{std::make_shared<arrow::StringBuilder>()},
      m_timestamp_builder{std::make_shared<arrow::Int64Builder>()} {}

book_sink_t::~book_sink_t() {
  try {
    flush();
  } catch (const std::exception& ex) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " failed to flush() sink";
  }
}

void book_sink_t::accept(const response::book_t& book,
                         const model::refdata_t& refdata) {
  const std::optional<model::refdata_t::pair_precision_t> precision{
      refdata.pair_precision(book.symbol())};
  if (!precision) {
    const std::string msg{"cannot find refdata for symbol: " + book.symbol()};
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << msg;
    throw std::runtime_error{msg};
  }

  PARQUET_THROW_NOT_OK(
      m_recv_tm_builder->Append(book.header().recv_tm().micros()));
  PARQUET_THROW_NOT_OK(m_type_builder->Append(book.header().type()));
  PARQUET_THROW_NOT_OK(m_crc32_builder->Append(book.crc32()));

  PARQUET_THROW_NOT_OK(m_bid_price_builder->Reserve(book.bids().size()));
  PARQUET_THROW_NOT_OK(m_bid_qty_builder->Reserve(book.bids().size()));
  PARQUET_THROW_NOT_OK(m_bid_builder->Reserve(book.bids().size()));
  PARQUET_THROW_NOT_OK(m_bids_builder->Reserve(1));

  PARQUET_THROW_NOT_OK(m_bids_builder->Append(book.bids().size()));
  for (const auto& bid : book.bids()) {
    PARQUET_THROW_NOT_OK(m_bid_builder->Append());
    PARQUET_THROW_NOT_OK(m_bid_price_builder->Append(bid.first.str()));
    PARQUET_THROW_NOT_OK(m_bid_qty_builder->Append(bid.second.str()));
  }

  PARQUET_THROW_NOT_OK(m_ask_price_builder->Reserve(book.asks().size()));
  PARQUET_THROW_NOT_OK(m_ask_qty_builder->Reserve(book.asks().size()));
  PARQUET_THROW_NOT_OK(m_ask_builder->Reserve(book.asks().size()));
  PARQUET_THROW_NOT_OK(m_asks_builder->Reserve(1));

  PARQUET_THROW_NOT_OK(m_asks_builder->Append(book.asks().size()));
  for (const auto& ask : book.asks()) {
    PARQUET_THROW_NOT_OK(m_ask_builder->Append());
    PARQUET_THROW_NOT_OK(m_ask_price_builder->Append(ask.first.str()));
    PARQUET_THROW_NOT_OK(m_ask_qty_builder->Append(ask.second.str()));
  }

  PARQUET_THROW_NOT_OK(m_symbol_builder->Append(book.symbol()));
  PARQUET_THROW_NOT_OK(m_timestamp_builder->Append(book.timestamp().micros()));

  if (++m_num_rows >= c_flush_threshold) {
    flush();
  }
}

void book_sink_t::flush() {
  std::shared_ptr<arrow::Array> recv_tm_array;
  std::shared_ptr<arrow::Array> type_array;
  std::shared_ptr<arrow::Array> bids_array;
  std::shared_ptr<arrow::Array> asks_array;
  std::shared_ptr<arrow::Array> crc32_array;
  std::shared_ptr<arrow::Array> symbol_array;
  std::shared_ptr<arrow::Array> timestamp_array;

  PARQUET_THROW_NOT_OK(m_recv_tm_builder->Finish(&recv_tm_array));
  PARQUET_THROW_NOT_OK(m_type_builder->Finish(&type_array));
  PARQUET_THROW_NOT_OK(m_bids_builder->Finish(&bids_array));
  PARQUET_THROW_NOT_OK(m_asks_builder->Finish(&asks_array));
  PARQUET_THROW_NOT_OK(m_crc32_builder->Finish(&crc32_array));
  PARQUET_THROW_NOT_OK(m_symbol_builder->Finish(&symbol_array));
  PARQUET_THROW_NOT_OK(m_timestamp_builder->Finish(&timestamp_array));

  auto columns = std::vector<std::shared_ptr<arrow::Array>>{
      recv_tm_array, type_array,   bids_array,     asks_array,
      crc32_array,   symbol_array, timestamp_array};

  std::shared_ptr<arrow::RecordBatch> batch =
      arrow::RecordBatch::Make(m_schema, m_num_rows, columns);
  PARQUET_THROW_NOT_OK(m_writer.arrow_file_writer().WriteRecordBatch(*batch));

  m_num_rows = 0;
}

std::shared_ptr<arrow::DataType> book_sink_t::quote_struct() {
  const auto field_vector =
      arrow::FieldVector{arrow::field("price", arrow::utf8(), false),
                         arrow::field("qty", arrow::utf8(), false)};
  return arrow::struct_(field_vector);
}

std::shared_ptr<arrow::Schema> book_sink_t::schema(integer_t book_depth) {
  auto metadata = std::make_shared<arrow::KeyValueMetadata>();
  metadata->Append("book_depth", std::to_string(book_depth));

  // TODO: add KeyValueMetadata for enum fields

  auto field_vector = arrow::FieldVector{
      arrow::field(c_header_recv_tm, arrow::int64(),
                   false),  // TODO: replace with timestamp type?
      arrow::field(c_header_type, arrow::utf8(), false),
      arrow::field(c_book_bids, arrow::list(quote_struct()), false),
      arrow::field(c_book_asks, arrow::list(quote_struct()), false),
      arrow::field(c_book_checksum, arrow::uint64(), false),
      arrow::field(c_book_symbol, arrow::utf8(), false),
      arrow::field(c_book_timestamp, arrow::int64(),
                   false),  // TODO: replace with timestamp type?
  };
  return arrow::schema(field_vector)->WithMetadata(metadata);
  ;
}

}  // namespace pq
}  // namespace krakpot
