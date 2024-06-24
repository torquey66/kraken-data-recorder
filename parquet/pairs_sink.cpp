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
  m_status_builder_builder.Reset();
  m_symbol_builder.Reset();
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
