#include "pairs_sink.hpp"

namespace krakpot {
namespace pq {

pairs_sink_t::pairs_sink_t(std::string parquet_dir, sink_id_t id)
    : m_schema{schema()},
      m_parquet_dir{parquet_dir},
      m_pairs_filename{sink_filename(parquet_dir, c_sink_name, id)},
      m_pairs_file{open_sink_file(m_pairs_filename)},
      m_os{open_writer(m_pairs_file, m_schema)} {}

void pairs_sink_t::reset_builders() {
  m_recv_tm_builder.Reset();
  m_base_builder.Reset();
  m_cost_min_builder.Reset();
  m_cost_precision_builder.Reset();
  m_has_index_builder.Reset();
  m_margin_initial_builder.Reset();
  m_marginable_builder.Reset();
  m_position_limit_long_builder.Reset();
  m_position_limit_short_builder.Reset();
  m_price_increment_builder.Reset();
  m_price_precision_builder.Reset();
  m_qty_increment_builder.Reset();
  m_qty_min_builder.Reset();
  m_qty_precision_builder.Reset();
  m_quote_builder.Reset();
  m_status_builder.Reset();
  m_symbol_builder.Reset();
}

void pairs_sink_t::accept(const response::header_t& header,
                          const std::vector<response::pair_t>& pairs) {
  reset_builders();

  for (const auto& pair : pairs) {
    PARQUET_THROW_NOT_OK(m_recv_tm_builder.Append(header.recv_tm().micros()));
    PARQUET_THROW_NOT_OK(m_base_builder.Append(pair.base()));
    PARQUET_THROW_NOT_OK(m_cost_min_builder.Append(pair.cost_min()));
    PARQUET_THROW_NOT_OK(
        m_cost_precision_builder.Append(pair.cost_precision()));
    PARQUET_THROW_NOT_OK(m_has_index_builder.Append(pair.has_index()));
    if (pair.margin_initial()) {
      PARQUET_THROW_NOT_OK(
          m_margin_initial_builder.Append(*pair.margin_initial()));
    } else {
      PARQUET_THROW_NOT_OK(m_margin_initial_builder.AppendNull());
    }
    PARQUET_THROW_NOT_OK(m_marginable_builder.Append(pair.marginable()));
    if (pair.position_limit_long()) {
      PARQUET_THROW_NOT_OK(
          m_position_limit_long_builder.Append(*pair.position_limit_long()));
    } else {
      PARQUET_THROW_NOT_OK(m_position_limit_long_builder.AppendNull());
    }
    if (pair.position_limit_short()) {
      PARQUET_THROW_NOT_OK(
          m_position_limit_short_builder.Append(*pair.position_limit_short()));
    } else {
      PARQUET_THROW_NOT_OK(m_position_limit_short_builder.AppendNull());
    }
    PARQUET_THROW_NOT_OK(
        m_price_increment_builder.Append(pair.price_increment()));
    PARQUET_THROW_NOT_OK(
        m_price_precision_builder.Append(pair.price_precision()));
    PARQUET_THROW_NOT_OK(m_qty_increment_builder.Append(pair.qty_increment()));
    PARQUET_THROW_NOT_OK(m_qty_min_builder.Append(pair.qty_min()));
    PARQUET_THROW_NOT_OK(m_qty_precision_builder.Append(pair.qty_precision()));
    PARQUET_THROW_NOT_OK(m_quote_builder.Append(pair.quote()));
    PARQUET_THROW_NOT_OK(m_status_builder.Append(pair.status()));
    PARQUET_THROW_NOT_OK(m_symbol_builder.Append(pair.symbol()));
  }

  std::shared_ptr<arrow::Array> recv_tm_array;
  std::shared_ptr<arrow::Array> base_array;
  std::shared_ptr<arrow::Array> cost_min_array;
  std::shared_ptr<arrow::Array> cost_precision_array;
  std::shared_ptr<arrow::Array> has_index_array;
  std::shared_ptr<arrow::Array> margin_initial_array;
  std::shared_ptr<arrow::Array> marginable_array;
  std::shared_ptr<arrow::Array> position_limit_long_array;
  std::shared_ptr<arrow::Array> position_limit_short_array;
  std::shared_ptr<arrow::Array> price_increment_array;
  std::shared_ptr<arrow::Array> price_precision_array;
  std::shared_ptr<arrow::Array> qty_increment_array;
  std::shared_ptr<arrow::Array> qty_min_array;
  std::shared_ptr<arrow::Array> qty_precision_array;
  std::shared_ptr<arrow::Array> quote_array;
  std::shared_ptr<arrow::Array> status_builder_array;
  std::shared_ptr<arrow::Array> symbol_array;

  PARQUET_THROW_NOT_OK(m_recv_tm_builder.Finish(&recv_tm_array));
  PARQUET_THROW_NOT_OK(m_base_builder.Finish(&base_array));
  PARQUET_THROW_NOT_OK(m_cost_min_builder.Finish(&cost_min_array));
  PARQUET_THROW_NOT_OK(m_cost_precision_builder.Finish(&cost_precision_array));
  PARQUET_THROW_NOT_OK(m_has_index_builder.Finish(&has_index_array));
  PARQUET_THROW_NOT_OK(m_margin_initial_builder.Finish(&margin_initial_array));
  PARQUET_THROW_NOT_OK(m_marginable_builder.Finish(&marginable_array));
  PARQUET_THROW_NOT_OK(
      m_position_limit_long_builder.Finish(&position_limit_long_array));
  PARQUET_THROW_NOT_OK(
      m_position_limit_short_builder.Finish(&position_limit_short_array));
  PARQUET_THROW_NOT_OK(
      m_price_increment_builder.Finish(&price_increment_array));
  PARQUET_THROW_NOT_OK(
      m_price_precision_builder.Finish(&price_precision_array));
  PARQUET_THROW_NOT_OK(m_qty_increment_builder.Finish(&qty_increment_array));
  PARQUET_THROW_NOT_OK(m_qty_min_builder.Finish(&qty_min_array));
  PARQUET_THROW_NOT_OK(m_qty_precision_builder.Finish(&qty_precision_array));
  PARQUET_THROW_NOT_OK(m_quote_builder.Finish(&quote_array));
  PARQUET_THROW_NOT_OK(m_status_builder.Finish(&status_builder_array));
  PARQUET_THROW_NOT_OK(m_symbol_builder.Finish(&symbol_array));

  auto columns = std::vector<std::shared_ptr<arrow::Array>>{
      recv_tm_array,
      base_array,
      cost_min_array,
      cost_precision_array,
      has_index_array,
      margin_initial_array,
      marginable_array,
      position_limit_long_array,
      position_limit_short_array,
      price_increment_array,
      price_precision_array,
      qty_increment_array,
      qty_min_array,
      qty_precision_array,
      quote_array,
      status_builder_array,
      symbol_array,
  };

  std::shared_ptr<arrow::RecordBatch> batch =
      arrow::RecordBatch::Make(m_schema, pairs.size(), columns);
  PARQUET_THROW_NOT_OK(m_os->WriteRecordBatch(*batch));
}

std::shared_ptr<arrow::Schema> pairs_sink_t::schema() {
  // TODO: add KeyValueMetadata for enum fields

  auto field_vector = arrow::FieldVector{
      arrow::field(c_header_recv_tm, arrow::int64(),
                   false),  // TODO: replace with timestamp type
      arrow::field(c_pair_base, arrow::utf8(), false),
      arrow::field(c_pair_cost_min, arrow::float64(), false),
      arrow::field(c_pair_cost_precision, arrow::int64(), false),
      arrow::field(c_pair_has_index, arrow::boolean(), false),
      arrow::field(c_pair_margin_initial, arrow::float64(), true),
      arrow::field(c_pair_marginable, arrow::boolean(), false),
      arrow::field(c_pair_position_limit_long, arrow::int64(), true),
      arrow::field(c_pair_position_limit_short, arrow::int64(), true),
      arrow::field(c_pair_price_increment, arrow::float64(), false),
      arrow::field(c_pair_price_precision, arrow::int64(), false),
      arrow::field(c_pair_qty_increment, arrow::float64(), false),
      arrow::field(c_pair_qty_min, arrow::float64(), false),
      arrow::field(c_pair_qty_precision, arrow::int64(), false),
      arrow::field(c_pair_quote, arrow::utf8(), false),
      arrow::field(c_pair_status, arrow::int8(), false),
      arrow::field(c_pair_symbol, arrow::utf8(), false),
  };
  return arrow::schema(field_vector);
}

}  // namespace pq
}  // namespace krakpot
