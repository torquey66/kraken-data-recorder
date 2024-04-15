#include "parquet.hpp"

#include <boost/log/trivial.hpp>

namespace krakpot {
namespace pq {

using arrow::Compression;
using parquet::ParquetDataPageVersion;
using parquet::ParquetVersion;
using parquet::WriterProperties;

trades_sink_t::trades_sink_t(std::string parquet_dir)
    : m_parquet_dir{parquet_dir},
      m_trades_filename{m_parquet_dir + "/trades.pq"},
      m_trades_file{open_trades_file(m_trades_filename)},
      m_os{open_writer(m_trades_file)}, m_schema{schema()} {}

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

std::shared_ptr<arrow::io::FileOutputStream>
trades_sink_t::open_trades_file(std::string trades_filename) {
  auto result = arrow::io::FileOutputStream::Open(trades_filename);
  if (result.ok()) {
    return result.ValueOrDie();
  }
  const auto msg = "failed to open: " + trades_filename +
                   " error: " + result.status().ToString();
  BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << msg;
  throw std::runtime_error(msg);
}

std::unique_ptr<parquet::arrow::FileWriter> trades_sink_t::open_writer(
    std::shared_ptr<arrow::io::FileOutputStream> trades_file) {

  std::shared_ptr<WriterProperties> props =
      WriterProperties::Builder()
          .max_row_group_length(32 * 1024)
          ->created_by("Krakpot")
          ->version(ParquetVersion::PARQUET_2_6)
          ->data_page_version(ParquetDataPageVersion::V2)
          ->compression(Compression::SNAPPY)
          ->build();

  std::shared_ptr<parquet::ArrowWriterProperties> arrow_props =
      parquet::ArrowWriterProperties::Builder().store_schema()->build();

  arrow::Result<std::unique_ptr<parquet::arrow::FileWriter>> result =
      parquet::arrow::FileWriter::Open(*schema(), arrow::default_memory_pool(),
                                       trades_file, props, arrow_props);

  return std::move(result.ValueOrDie()); // TODO this is all really hinky
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
