#include "constants.hpp"
#include "level_book.hpp"
#include "responses.hpp"
#include "types.hpp"

#include <arrow/array.h>
#include <arrow/io/api.h>
#include <arrow/record_batch.h>
#include <arrow/scalar.h>
#include <parquet/arrow/reader.h>
#include <nlohmann/json.hpp>

#include <iomanip>
#include <iostream>
#include <string>

using namespace krakpot;

std::vector<quote_t> extract(const arrow::ListArray& quotes_array,
                             int64_t idx) {
  auto result = std::vector<quote_t>{};
  const auto slice = std::dynamic_pointer_cast<arrow::StructArray>(
      quotes_array.value_slice(idx));
  const auto price_array = std::dynamic_pointer_cast<arrow::StringArray>(
      slice->GetFieldByName(c_book_price));
  const auto qty_array = std::dynamic_pointer_cast<arrow::StringArray>(
      slice->GetFieldByName(c_book_qty));
  for (auto sidx = 0; sidx < slice->length(); ++sidx) {
    const auto price_view = price_array->Value(sidx);
    const auto qty_view = qty_array->Value(sidx);
    const auto price_str = std::string(price_view.begin(), price_view.end());
    const auto qty_str = std::string(qty_view.begin(), qty_view.end());
    const auto price = std::stod(price_str);
    const auto qty = std::stod(qty_str);
    const auto quote =
        std::make_pair(decimal_t(price, price_str), decimal_t(qty, qty_str));
    result.push_back(quote);
  }
  return result;
}

void dump_sides(const model::sides_t& sides, const size_t max_depth = 10) {
  const auto& bids = sides.bids();
  const auto& asks = sides.asks();
  auto bid_it = bids.begin();
  auto ask_it = asks.begin();
  for (size_t depth = 0; depth < max_depth; ++depth) {
    const auto bid_px = bid_it != bids.end() ? bid_it->first.value() : c_NaN;
    const auto bid_qty = bid_it != bids.end() ? bid_it->second.value() : c_NaN;
    const auto ask_px = ask_it != asks.end() ? ask_it->first.value() : c_NaN;
    const auto ask_qty = ask_it != asks.end() ? ask_it->second.value() : c_NaN;
    std::cerr << std::setprecision(8) << bid_qty << " @ " << bid_px << "     "
              << ask_qty << " @ " << ask_px << std::endl;
    ++bid_it;
    ++ask_it;
  }
}

void process_pairs(std::string pairs_filename,
                   model::level_book_t& level_book) {
  auto* pool = arrow::default_memory_pool();
  auto reader_properties = parquet::ReaderProperties(pool);
  reader_properties.set_buffer_size(4096 * 4);
  reader_properties.enable_buffered_stream();

  auto arrow_reader_props = parquet::ArrowReaderProperties();
  arrow_reader_props.set_batch_size(128 * 1024);

  parquet::arrow::FileReaderBuilder reader_builder;
  PARQUET_THROW_NOT_OK(
      reader_builder.OpenFile(pairs_filename, false, reader_properties));
  reader_builder.memory_pool(pool);
  reader_builder.properties(arrow_reader_props);

  std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
  PARQUET_ASSIGN_OR_THROW(arrow_reader, reader_builder.Build());

  std::shared_ptr<::arrow::RecordBatchReader> rb_reader;
  PARQUET_THROW_NOT_OK(arrow_reader->GetRecordBatchReader(&rb_reader));

  for (arrow::Result<std::shared_ptr<arrow::RecordBatch>> maybe_batch :
       *rb_reader) {
    if (!maybe_batch.ok()) {
      std::cerr << "error: " << maybe_batch.status().ToString() << std::endl;
    }

    const auto& batch = *maybe_batch.ValueOrDie();
    // const auto recv_tm_array = std::dynamic_pointer_cast<arrow::Int64Array>(
    //     batch.GetColumnByName(c_header_recv_tm));
    const auto base_array = std::dynamic_pointer_cast<arrow::StringArray>(
        batch.GetColumnByName(c_pair_base));
    const auto cost_min_array = std::dynamic_pointer_cast<arrow::DoubleArray>(
        batch.GetColumnByName(c_pair_cost_min));
    const auto cost_precision_array =
        std::dynamic_pointer_cast<arrow::Int64Array>(
            batch.GetColumnByName(c_pair_cost_precision));
    const auto has_index_array = std::dynamic_pointer_cast<arrow::BooleanArray>(
        batch.GetColumnByName(c_pair_has_index));
    const auto margin_initial_array =
        std::dynamic_pointer_cast<arrow::DoubleArray>(
            batch.GetColumnByName(c_pair_margin_initial));
    const auto marginable_array =
        std::dynamic_pointer_cast<arrow::BooleanArray>(
            batch.GetColumnByName(c_pair_marginable));
    const auto position_limit_long_array =
        std::dynamic_pointer_cast<arrow::Int64Array>(
            batch.GetColumnByName(c_pair_position_limit_long));
    const auto position_limit_short_array =
        std::dynamic_pointer_cast<arrow::Int64Array>(
            batch.GetColumnByName(c_pair_position_limit_short));
    const auto price_increment_array =
        std::dynamic_pointer_cast<arrow::DoubleArray>(
            batch.GetColumnByName(c_pair_price_increment));
    const auto price_precision_array =
        std::dynamic_pointer_cast<arrow::Int64Array>(
            batch.GetColumnByName(c_pair_price_precision));
    const auto qty_increment_array =
        std::dynamic_pointer_cast<arrow::DoubleArray>(
            batch.GetColumnByName(c_pair_qty_increment));
    const auto qty_min_array = std::dynamic_pointer_cast<arrow::DoubleArray>(
        batch.GetColumnByName(c_pair_qty_min));
    const auto qty_precision_array =
        std::dynamic_pointer_cast<arrow::Int64Array>(
            batch.GetColumnByName(c_pair_qty_precision));
    const auto quote_array = std::dynamic_pointer_cast<arrow::StringArray>(
        batch.GetColumnByName(c_pair_quote));
    const auto status_array = std::dynamic_pointer_cast<arrow::Int8Array>(
        batch.GetColumnByName(c_pair_status));
    const auto symbol_array = std::dynamic_pointer_cast<arrow::StringArray>(
        batch.GetColumnByName(c_pair_symbol));

    for (auto idx = 0; idx < batch.num_rows(); ++idx) {
      //      const auto recv_tm = recv_tm_array->Value(idx);
      const auto base = base_array->Value(idx);
      const auto cost_min = cost_min_array->Value(idx);
      const auto cost_precision = cost_precision_array->Value(idx);
      const auto has_index = has_index_array->Value(idx);
      const auto margin_initial = margin_initial_array->Value(idx);
      const auto marginable = marginable_array->Value(idx);
      const auto position_limit_long = position_limit_long_array->Value(idx);
      const auto position_limit_short = position_limit_short_array->Value(idx);
      const auto price_increment = price_increment_array->Value(idx);
      const auto price_precision = price_precision_array->Value(idx);
      const auto qty_increment = qty_increment_array->Value(idx);
      const auto qty_min = qty_min_array->Value(idx);
      const auto qty_precision = qty_precision_array->Value(idx);
      const auto quote = quote_array->Value(idx);
      const auto status = status_array->Value(idx);
      const auto symbol = symbol_array->Value(idx);

      const auto pair =
          response::pair_t{std::string{base.begin(), base.end()},
                           cost_min,
                           cost_precision,
                           has_index,
                           margin_initial,
                           marginable,
                           position_limit_long,
                           position_limit_short,
                           price_increment,
                           price_precision,
                           qty_increment,
                           qty_min,
                           qty_precision,
                           std::string{quote.begin(), quote.end()},
                           response::pair_t::status_t{status},
                           std::string{symbol.begin(), symbol.end()}};
      level_book.accept(pair);
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "usage: " << argv[0] << " <pairs parquet file>"
              << " <book parquet file>" << std::endl;
    return -1;
  }

  const auto pairs_filename = std::string{argv[1]};
  const auto filename = std::string{argv[2]};

  auto level_book = model::level_book_t{e_1000};
  process_pairs(pairs_filename, level_book);

  auto* pool = arrow::default_memory_pool();
  auto reader_properties = parquet::ReaderProperties(pool);
  reader_properties.set_buffer_size(4096 * 4);
  reader_properties.enable_buffered_stream();

  auto arrow_reader_props = parquet::ArrowReaderProperties();
  arrow_reader_props.set_batch_size(128 * 1024);

  parquet::arrow::FileReaderBuilder reader_builder;
  PARQUET_THROW_NOT_OK(
      reader_builder.OpenFile(filename, false, reader_properties));
  reader_builder.memory_pool(pool);
  reader_builder.properties(arrow_reader_props);

  std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
  PARQUET_ASSIGN_OR_THROW(arrow_reader, reader_builder.Build());

  std::shared_ptr<::arrow::RecordBatchReader> rb_reader;
  PARQUET_THROW_NOT_OK(arrow_reader->GetRecordBatchReader(&rb_reader));

  for (arrow::Result<std::shared_ptr<arrow::RecordBatch>> maybe_batch :
       *rb_reader) {
    if (!maybe_batch.ok()) {
      std::cerr << "error: " << maybe_batch.status().ToString() << std::endl;
    }
    const auto& batch = *maybe_batch.ValueOrDie();
    const auto recv_tm_array = std::dynamic_pointer_cast<arrow::Int64Array>(
        batch.GetColumnByName(c_header_recv_tm));
    const auto type_array = std::dynamic_pointer_cast<arrow::StringArray>(
        batch.GetColumnByName(c_header_type));
    const auto bids_array = std::dynamic_pointer_cast<arrow::ListArray>(
        batch.GetColumnByName(c_book_bids));
    const auto asks_array = std::dynamic_pointer_cast<arrow::ListArray>(
        batch.GetColumnByName(c_book_asks));
    const auto crc32_array = std::dynamic_pointer_cast<arrow::UInt64Array>(
        batch.GetColumnByName(c_book_checksum));
    const auto symbol_array = std::dynamic_pointer_cast<arrow::StringArray>(
        batch.GetColumnByName(c_book_symbol));
    const auto timestamp_array = std::dynamic_pointer_cast<arrow::Int64Array>(
        batch.GetColumnByName(c_book_timestamp));

    for (auto idx = 0; idx < batch.num_rows(); ++idx) {
      const auto recv_tm = recv_tm_array->Value(idx);
      const auto type = type_array->Value(idx);
      const auto bids = extract(*bids_array, idx);
      const auto asks = extract(*asks_array, idx);
      const auto crc32 = crc32_array->Value(idx);
      const auto symbol = symbol_array->Value(idx);
      const auto timestamp = timestamp_array->Value(idx);
      const auto header = response::header_t{
          recv_tm, "book", std::string{type.begin(), type.end()}};
      const auto symbol_str = std::string{symbol.begin(), symbol.end()};
      const auto response =
          response::book_t{header, asks, bids, crc32, symbol_str, timestamp};
      try {
        level_book.accept(response);
      } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        std::cerr << response.str() << std::endl;
        dump_sides(level_book.sides(symbol_str));
        return -1;
      }
    }
  }

  /*
     handle_msg: bogus crc32 expected: 2542650875 actual: 1140764732 msg:
     {"channel":"book","type":"update","data":[{"symbol":"SOL/USD","bids":[{"price":146.91,"qty":917.15439679}],"asks":[],"checksum":2542650875,"timestamp":"2024-07-01T19:30:11.454069Z"}]}
  */

  const std::string bad_msg = R"MSG(
  {
    "channel": "book",
    "type": "update",
    "data": [
      {
        "symbol": "SOL/USD",
        "bids": [
           {"price":146.91,"qty":917.15439679}
        ],
        "asks": [],
        "checksum": 2542650875,
        "timestamp":"2024-07-01T19:30:11.454069Z"
      }
    ]
  }
  )MSG";
  simdjson::ondemand::parser parser;
  simdjson::padded_string bad_response{bad_msg};
  simdjson::ondemand::document bad_doc = parser.iterate(bad_response);
  const auto bad_update = krakpot::response::book_t::from_json(bad_doc);
  try {
    dump_sides(level_book.sides(bad_update.symbol()));
    std::cout << "checksum: " << level_book.crc32(bad_update.symbol())
              << std::endl;
    std::cout << "-------------------------------------------------------------"
                 "-------------------"
              << std::endl;
    level_book.accept(bad_update);
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    std::cerr << bad_update.str() << std::endl;
    dump_sides(level_book.sides(bad_update.symbol()));
    std::cout << "checksum: " << level_book.crc32(bad_update.symbol())
              << std::endl;
    return -1;
  }
}
