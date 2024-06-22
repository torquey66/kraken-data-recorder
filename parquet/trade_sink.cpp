/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include "trade_sink.hpp"

#include <arrow/scalar.h>
#include <boost/log/trivial.hpp>

namespace krakpot {
namespace pq {

trades_sink_t::trades_sink_t(std::string parquet_dir, sink_id_t id)
    : m_schema{schema()}, m_parquet_dir{parquet_dir},
      m_trades_filename{sink_filename(parquet_dir, c_sink_name, id)},
      m_trades_file{open_sink_file(m_trades_filename)},
      m_os{open_writer(m_trades_file, m_schema)} {}

void trades_sink_t::accept(const response::trades_t &trades) {
  reset_builders();

  for (const auto &trade : trades) {
    PARQUET_THROW_NOT_OK(
        m_recv_tm_builder.Append(trades.header().recv_tm().micros()));
    PARQUET_THROW_NOT_OK(
        m_ord_type_builder.Append(std::string(1, trade.ord_type)));
    PARQUET_THROW_NOT_OK(m_price_builder.Append(trade.price.value()));
    PARQUET_THROW_NOT_OK(m_qty_builder.Append(trade.qty.value()));
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
      arrow::field(c_header_recv_tm, arrow::int64(),
                   false), // TODO: replace with timestamp type
      arrow::field(c_trade_ord_type, arrow::utf8(), false),
      arrow::field(c_trade_price, arrow::float64(), false),
      arrow::field(c_trade_qty, arrow::float64(), false),
      arrow::field(c_trade_side, arrow::utf8(), false),
      arrow::field(c_trade_symbol, arrow::utf8(), false),
      arrow::field(c_trade_timestamp, arrow::int64(),
                   false), // TODO: replace with timestamp type
      arrow::field(c_trade_trade_id, arrow::uint64(), false),
  };
  return arrow::schema(field_vector);
}

} // namespace pq
} // namespace krakpot
