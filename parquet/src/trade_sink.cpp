#include "trade_sink.hpp"

#include <arrow/scalar.h>
#include <boost/log/trivial.hpp>

namespace kdr {
namespace pq {

trades_sink_t::trades_sink_t(std::string parquet_dir, sink_id_t id)
    : m_schema{schema()},
      m_sink_filename{parquet_filename(parquet_dir, c_sink_name, id)},
      m_writer{m_sink_filename, m_schema} {}

trades_sink_t::~trades_sink_t() {
  try {
    flush();
  } catch (const std::exception& ex) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " failed to flush() sink";
  }
}

void trades_sink_t::accept(const response::trades_t& trades,
                           const model::refdata_t& refdata) {
  for (const auto& trade : trades) {
    const std::optional<model::refdata_t::pair_precision_t> precision{
        refdata.pair_precision(trade.symbol())};
    if (!precision) {
      const std::string msg{"cannot find refdata for symbol: " +
                            trade.symbol()};
      BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << msg;
      throw std::runtime_error{msg};
    }

    PARQUET_THROW_NOT_OK(
        m_recv_tm_builder.Append(trades.header().recv_tm().micros()));
    PARQUET_THROW_NOT_OK(
        m_ord_type_builder.Append(std::string(1, trade.ord_type())));
    PARQUET_THROW_NOT_OK(
        m_price_builder.Append(trade.price().str(precision->price_precision)));
    PARQUET_THROW_NOT_OK(
        m_qty_builder.Append(trade.qty().str(precision->qty_precision)));
    PARQUET_THROW_NOT_OK(m_side_builder.Append(std::string(1, trade.side())));
    PARQUET_THROW_NOT_OK(m_symbol_builder.Append(trade.symbol()));
    PARQUET_THROW_NOT_OK(
        m_timestamp_builder.Append(trade.timestamp().micros()));
    PARQUET_THROW_NOT_OK(m_trade_id_builder.Append(trade.trade_id()));

    ++m_num_rows;
  }

  if (m_num_rows >= c_flush_threshold) {
    flush();
  }
}

void trades_sink_t::flush() {
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
      arrow::RecordBatch::Make(m_schema, m_num_rows, columns);
  PARQUET_THROW_NOT_OK(m_writer.arrow_file_writer().WriteRecordBatch(*batch));

  m_num_rows = 0;
}

std::shared_ptr<arrow::Schema> trades_sink_t::schema() {
  // TODO: add KeyValueMetadata for enum fields

  auto field_vector = arrow::FieldVector{
      arrow::field(c_header_recv_tm, arrow::int64(),
                   false),  // TODO: replace with timestamp type
      arrow::field(std::string{model::trade_t::c_ord_type}, arrow::utf8(),
                   false),
      arrow::field(std::string{model::trade_t::c_price}, arrow::utf8(), false),
      arrow::field(std::string{model::trade_t::c_qty}, arrow::utf8(), false),
      arrow::field(std::string{model::trade_t::c_side}, arrow::utf8(), false),
      arrow::field(std::string{model::trade_t::c_symbol}, arrow::utf8(), false),
      arrow::field(std::string{model::trade_t::c_timestamp}, arrow::int64(),
                   false),  // TODO: replace with timestamp type
      arrow::field(std::string{model::trade_t::c_trade_id}, arrow::uint64(),
                   false),
  };
  return arrow::schema(field_vector);
}

}  // namespace pq
}  // namespace kdr
