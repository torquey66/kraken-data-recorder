#include "assets_sink.hpp"

namespace krakpot {
namespace pq {

assets_sink_t::assets_sink_t(std::string parquet_dir, sink_id_t id)
    : m_schema{schema()}, m_parquet_dir{parquet_dir},
      m_assets_filename{sink_filename(parquet_dir, c_sink_name, id)},
      m_assets_file{open_sink_file(m_assets_filename)},
      m_os{open_writer(m_assets_file, m_schema)} {}

void assets_sink_t::accept(const response::header_t& header,
                           const std::vector<response::asset_t>& assets) {
  reset_builders();

  for (const auto& asset : assets) {
    PARQUET_THROW_NOT_OK(m_recv_tm_builder.Append(header.recv_tm().micros()));
    PARQUET_THROW_NOT_OK(m_borrowable_builder.Append(asset.borrowable()));
    PARQUET_THROW_NOT_OK(
        m_collateral_value_builder.Append(asset.collateral_value()));
    PARQUET_THROW_NOT_OK(m_id_builder.Append(asset.id()));
    if (asset.margin_rate()) {
      PARQUET_THROW_NOT_OK(m_margin_rate_builder.Append(*asset.margin_rate()));
    } else {
      PARQUET_THROW_NOT_OK(m_margin_rate_builder.AppendNull());
    }
    PARQUET_THROW_NOT_OK(m_precision_builder.Append(asset.precision()));
    PARQUET_THROW_NOT_OK(
        m_precision_display_builder.Append(asset.precision_display()));
    PARQUET_THROW_NOT_OK(m_status_builder.Append(asset.status()));
  }

  std::shared_ptr<arrow::Array> recv_tm_array;
  std::shared_ptr<arrow::Array> borrowable_array;
  std::shared_ptr<arrow::Array> collateral_value_array;
  std::shared_ptr<arrow::Array> id_array;
  std::shared_ptr<arrow::Array> margin_rate_array;
  std::shared_ptr<arrow::Array> precision_array;
  std::shared_ptr<arrow::Array> precision_display_array;
  std::shared_ptr<arrow::Array> status_array;

  PARQUET_THROW_NOT_OK(m_recv_tm_builder.Finish(&recv_tm_array));
  PARQUET_THROW_NOT_OK(m_borrowable_builder.Finish(&borrowable_array));
  PARQUET_THROW_NOT_OK(
      m_collateral_value_builder.Finish(&collateral_value_array));
  PARQUET_THROW_NOT_OK(m_id_builder.Finish(&id_array));
  PARQUET_THROW_NOT_OK(m_margin_rate_builder.Finish(&margin_rate_array));
  PARQUET_THROW_NOT_OK(m_precision_builder.Finish(&precision_array));
  PARQUET_THROW_NOT_OK(
      m_precision_display_builder.Finish(&precision_display_array));
  PARQUET_THROW_NOT_OK(m_status_builder.Finish(&status_array));

  auto columns = std::vector<std::shared_ptr<arrow::Array>>{
      recv_tm_array,           borrowable_array,
      collateral_value_array,  id_array,
      margin_rate_array,       precision_array,
      precision_display_array, status_array};

  std::shared_ptr<arrow::RecordBatch> batch =
      arrow::RecordBatch::Make(m_schema, assets.size(), columns);
  PARQUET_THROW_NOT_OK(m_os->WriteRecordBatch(*batch));
}

void assets_sink_t::reset_builders() {
  m_recv_tm_builder.Reset();
  m_borrowable_builder.Reset();
  m_collateral_value_builder.Reset();
  m_id_builder.Reset();
  m_margin_rate_builder.Reset();
  m_precision_builder.Reset();
  m_precision_display_builder.Reset();
  m_status_builder.Reset();
}

std::shared_ptr<arrow::Schema> assets_sink_t::schema() {
  // TODO: add KeyValueMetadata for enum fields

  auto field_vector = arrow::FieldVector{
      arrow::field(c_header_recv_tm, arrow::int64(),
                   false), // TODO: replace with timestamp type
      arrow::field(c_asset_borrowable, arrow::boolean(), false),
      arrow::field(c_asset_collateral_value, arrow::float64(), false),
      arrow::field(c_asset_id, arrow::utf8(), false),
      arrow::field(c_asset_margin_rate, arrow::float64(), true),
      arrow::field(c_asset_precision, arrow::int64(), false),
      arrow::field(c_asset_precision_display, arrow::int64(), false),
      arrow::field(c_asset_status, arrow::int8(), false),
  };
  return arrow::schema(field_vector);
}

} // namespace pq
} // namespace krakpot
