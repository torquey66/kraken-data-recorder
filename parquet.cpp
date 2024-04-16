/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include "parquet.hpp"

#include <boost/log/trivial.hpp>

using arrow::Compression;
using parquet::ParquetDataPageVersion;
using parquet::ParquetVersion;
using parquet::WriterProperties;

namespace {

std::shared_ptr<arrow::io::FileOutputStream>
open_sink_file(std::string sink_filename) {
  auto result = arrow::io::FileOutputStream::Open(sink_filename);
  if (result.ok()) {
    return result.ValueOrDie();
  }
  const auto msg = "failed to open: " + sink_filename +
                   " error: " + result.status().ToString();
  BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << msg;
  throw std::runtime_error(msg);
}

std::unique_ptr<parquet::arrow::FileWriter>
open_writer(std::shared_ptr<arrow::io::FileOutputStream> sink_file,
            std::shared_ptr<arrow::Schema> schema) {

  std::shared_ptr<WriterProperties> props =
      WriterProperties::Builder()
          .max_row_group_length(64 * 1024)
          ->created_by("Krakpot")
          ->version(ParquetVersion::PARQUET_2_6)
          ->data_page_version(ParquetDataPageVersion::V2)
          ->compression(Compression::SNAPPY)
          ->build();

  std::shared_ptr<parquet::ArrowWriterProperties> arrow_props =
      parquet::ArrowWriterProperties::Builder().store_schema()->build();

  arrow::Result<std::unique_ptr<parquet::arrow::FileWriter>> result =
      parquet::arrow::FileWriter::Open(*schema, arrow::default_memory_pool(),
                                       sink_file, props, arrow_props);

  return std::move(result.ValueOrDie()); // TODO this is all really hinky
}

} // namespace

namespace krakpot {
namespace pq {

/*                       */
/* b o o k _ s i n k _ t */
/*                       */

book_sink_t::book_sink_t(std::string parquet_dir)
    : m_schema{schema()}, m_parquet_dir{parquet_dir},
      m_book_filename{m_parquet_dir + "/book.pq"},
      m_book_file{open_sink_file(m_book_filename)},
      m_os{open_writer(m_book_file, m_schema)},
      m_ask_px_builder{std::make_shared<arrow::DoubleBuilder>()},
      m_ask_qty_builder{std::make_shared<arrow::DoubleBuilder>()},
      m_asks_builder{arrow::default_memory_pool(), m_ask_px_builder,
                     m_ask_qty_builder, true} {}

void book_sink_t::accept(const response::book_t &book) {
  reset_builders();

  PARQUET_THROW_NOT_OK(
      m_recv_tm_builder.Append(book.header().recv_tm().micros()));

  PARQUET_THROW_NOT_OK(m_asks_builder.Append());
  for (const auto &ask : book.asks()) {
    PARQUET_THROW_NOT_OK(m_ask_px_builder->Append(ask.first));
    PARQUET_THROW_NOT_OK(m_ask_qty_builder->Append(ask.second));
  }

  PARQUET_THROW_NOT_OK(m_crc32_builder.Append(book.crc32()));
  PARQUET_THROW_NOT_OK(m_symbol_builder.Append(book.symbol()));
  PARQUET_THROW_NOT_OK(m_timestamp_builder.Append(book.timestamp().micros()));

  std::shared_ptr<arrow::Array> recv_tm_array;
  std::shared_ptr<arrow::Array> ask_px_array;
  std::shared_ptr<arrow::Array> ask_qty_array;
  std::shared_ptr<arrow::Array> asks_array;
  std::shared_ptr<arrow::Array> crc32_array;
  std::shared_ptr<arrow::Array> symbol_array;
  std::shared_ptr<arrow::Array> timestamp_array;

  PARQUET_THROW_NOT_OK(m_recv_tm_builder.Finish(&recv_tm_array));
  PARQUET_THROW_NOT_OK(m_ask_px_builder->Finish(&ask_px_array));
  PARQUET_THROW_NOT_OK(m_ask_qty_builder->Finish(&ask_qty_array));
  PARQUET_THROW_NOT_OK(m_asks_builder.Finish(&asks_array));
  PARQUET_THROW_NOT_OK(m_crc32_builder.Finish(&crc32_array));
  PARQUET_THROW_NOT_OK(m_symbol_builder.Finish(&symbol_array));
  PARQUET_THROW_NOT_OK(m_timestamp_builder.Finish(&timestamp_array));

  auto columns = std::vector<std::shared_ptr<arrow::Array>>{
      recv_tm_array, asks_array, crc32_array, symbol_array, timestamp_array};

  std::shared_ptr<arrow::RecordBatch> batch =
      arrow::RecordBatch::Make(m_schema, 1, columns);
  PARQUET_THROW_NOT_OK(m_os->WriteRecordBatch(*batch));
}

void book_sink_t::reset_builders() {
  m_recv_tm_builder.Reset();
  m_ask_px_builder->Reset();
  m_ask_qty_builder->Reset();
  m_asks_builder.Reset();
  m_crc32_builder.Reset();
  m_symbol_builder.Reset();
  m_timestamp_builder.Reset();
}

std::shared_ptr<arrow::Schema> book_sink_t::schema() {
  // TODO: add KeyValueMetadata for enum fields

  auto field_vector = arrow::FieldVector{
      arrow::field("recv_tm", arrow::int64(),
                   false), // TODO: replace with timestamp type
      arrow::field("asks", arrow::map(arrow::float64(), arrow::float64(), true),
                   false),
      arrow::field("crc32", arrow::uint64(), false),
      arrow::field("symbol", arrow::utf8(), false),
      arrow::field("timestamp", arrow::int64(),
                   false), // TODO: replace with timestamp type
  };
  return arrow::schema(field_vector);
}

/*                           */
/* t r a d e s _ s i n k _ t */
/*                           */

trades_sink_t::trades_sink_t(std::string parquet_dir)
    : m_schema{schema()}, m_parquet_dir{parquet_dir},
      m_trades_filename{m_parquet_dir + "/trades.pq"},
      m_trades_file{open_sink_file(m_trades_filename)},
      m_os{open_writer(m_trades_file, m_schema)} {}

void trades_sink_t::accept(const response::trades_t &trades) {
  reset_builders();

  for (const auto &trade : trades) {
    PARQUET_THROW_NOT_OK(
        m_recv_tm_builder.Append(trades.header().recv_tm().micros()));
    PARQUET_THROW_NOT_OK(
        m_ord_type_builder.Append(std::string(1, trade.ord_type)));
    PARQUET_THROW_NOT_OK(m_price_builder.Append(trade.price));
    PARQUET_THROW_NOT_OK(m_qty_builder.Append(trade.qty));
    PARQUET_THROW_NOT_OK(m_side_builder.Append(std::string(1, trade.side)));
    PARQUET_THROW_NOT_OK(m_symbol_builder.Append(trade.symbol));
    PARQUET_THROW_NOT_OK(m_timestamp_builder.Append(trade.timestamp.micros()));
    PARQUET_THROW_NOT_OK(m_trade_id_builder.Append(trade.trade_id));
  }

  std::shared_ptr<arrow::Array> recv_tm_array;
  std::shared_ptr<arrow::Array> ord_type_array;
  std::shared_ptr<arrow::Array> price_array;
  std::shared_ptr<arrow::Array> qty_array;
  std::shared_ptr<arrow::Array> side_array;
  std::shared_ptr<arrow::Array> symbol_array;
  std::shared_ptr<arrow::Array> timestamp_array;
  std::shared_ptr<arrow::Array> trade_id_array;

  PARQUET_THROW_NOT_OK(m_recv_tm_builder.Finish(&recv_tm_array));
  PARQUET_THROW_NOT_OK(m_ord_type_builder.Finish(&ord_type_array));
  PARQUET_THROW_NOT_OK(m_price_builder.Finish(&price_array));
  PARQUET_THROW_NOT_OK(m_qty_builder.Finish(&qty_array));
  PARQUET_THROW_NOT_OK(m_side_builder.Finish(&side_array));
  PARQUET_THROW_NOT_OK(m_symbol_builder.Finish(&symbol_array));
  PARQUET_THROW_NOT_OK(m_timestamp_builder.Finish(&timestamp_array));
  PARQUET_THROW_NOT_OK(m_trade_id_builder.Finish(&trade_id_array));

  auto columns = std::vector<std::shared_ptr<arrow::Array>>{
      recv_tm_array, ord_type_array, price_array,     qty_array,
      side_array,    symbol_array,   timestamp_array, trade_id_array,
  };

  std::shared_ptr<arrow::RecordBatch> batch =
      arrow::RecordBatch::Make(m_schema, trades.size(), columns);
  PARQUET_THROW_NOT_OK(m_os->WriteRecordBatch(*batch));
}

void trades_sink_t::reset_builders() {
  m_recv_tm_builder.Reset();
  m_ord_type_builder.Reset();
  m_price_builder.Reset();
  m_qty_builder.Reset();
  m_side_builder.Reset();
  m_symbol_builder.Reset();
  m_timestamp_builder.Reset();
  m_trade_id_builder.Reset();
}

std::shared_ptr<arrow::Schema> trades_sink_t::schema() {
  // TODO: add KeyValueMetadata for enum fields

  auto field_vector = arrow::FieldVector{
      arrow::field("recv_tm", arrow::int64(),
                   false), // TODO: replace with timestamp type
      arrow::field("ord_type", arrow::utf8(), false),
      arrow::field("price", arrow::float64(), false),
      arrow::field("qty", arrow::float64(), false),
      arrow::field("side", arrow::utf8(), false),
      arrow::field("symbol", arrow::utf8(), false),
      arrow::field("timestamp", arrow::int64(),
                   false), // TODO: replace with timestamp type
      arrow::field("trade_id", arrow::uint64(), false),
  };
  return arrow::schema(field_vector);
}

  } // namespace pq
} // namespace krakpot
