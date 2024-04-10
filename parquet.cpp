#include "parquet.hpp"

#include <arrow/util/type_fwd.h>
#include <boost/log/trivial.hpp>
#include <parquet/api/writer.h>
#include <parquet/arrow/writer.h>

namespace krakpot {
namespace pq {

trades_sink_t::trades_sink_t(std::string parquet_dir)
    : m_parquet_dir{parquet_dir},
      m_trades_filename{m_parquet_dir + "/trades.pq"},
      m_trades_file{open_trades_file(m_trades_filename)},
      m_os{open_stream(m_trades_file)} {}

void trades_sink_t::accept(const response::trades_t &trades) {
  for (const auto &trade : trades) {
    *m_os << trade.trade_id << parquet::EndRow;
  }
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

std::unique_ptr<parquet::StreamWriter> trades_sink_t::open_stream(
    std::shared_ptr<arrow::io::FileOutputStream> trades_file) {

  using arrow::Compression;
  using parquet::ParquetDataPageVersion;
  using parquet::ParquetVersion;
  using parquet::WriterProperties;

  ///
  // TODO: properties cribbed from
  // https://arrow.apache.org/docs/cpp/parquet.html#
  //
  // replace these with deliberately chosen ones
  //
  std::shared_ptr<WriterProperties> props =
      WriterProperties::Builder()
          .max_row_group_length(64 * 1024)
          ->created_by("Krakpot")
          ->version(ParquetVersion::PARQUET_2_6)
          ->data_page_version(ParquetDataPageVersion::V2)
          //          ->compression(Compression::SNAPPY)
          //          ->ArrowWriterProperties::store_schema()
          ->build();

  std::shared_ptr<parquet::schema::GroupNode> schema =
      std::static_pointer_cast<parquet::schema::GroupNode>(
          parquet::schema::GroupNode::Make(
              "schema", parquet::Repetition::REQUIRED,
              {
                  parquet::schema::PrimitiveNode::Make(
                      "col1", parquet::Repetition::REQUIRED,
                      parquet::Type::INT64, parquet::ConvertedType::INT_64),
              }));

  return std::make_unique<parquet::StreamWriter>(
      parquet::ParquetFileWriter::Open(trades_file, schema, props));
}

} // namespace pq
} // namespace krakpot
