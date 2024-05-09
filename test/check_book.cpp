#include "level_book.hpp"
#include "responses.hpp"
#include "types.hpp"

#include <arrow/array.h>
#include <arrow/io/api.h>
#include <arrow/record_batch.h>
#include <arrow/scalar.h>
#include <parquet/arrow/reader.h>

#include <iostream>
#include <string>

using namespace krakpot;

// std::vector<quote_t> extract(const arrow::ListArray &quote_col,
// arrow::int64_t idx) {
std::vector<quote_t> extract(const arrow::ListArray &quotes_array,
                             int64_t idx) {
  auto result = std::vector<quote_t>{};
  const auto slice = std::dynamic_pointer_cast<arrow::StructArray>(
      quotes_array.value_slice(idx));
  const auto price_array = std::dynamic_pointer_cast<arrow::StringArray>(
      slice->GetFieldByName("price"));
  const auto qty_array = std::dynamic_pointer_cast<arrow::StringArray>(
      slice->GetFieldByName("qty"));
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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <book parquet file>" << std::endl;
    return -1;
  }

  const auto filename = std::string{argv[1]};

  auto *pool = arrow::default_memory_pool();
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

  auto prev_level_book = model::level_book_t{};
  auto level_book = model::level_book_t{};

  for (arrow::Result<std::shared_ptr<arrow::RecordBatch>> maybe_batch :
       *rb_reader) {
    if (!maybe_batch.ok()) {
      std::cerr << "error: " << maybe_batch.status().ToString() << std::endl;
    }
    const auto &batch = *maybe_batch.ValueOrDie();
    const auto recv_tm_array = std::dynamic_pointer_cast<arrow::Int64Array>(
        batch.GetColumnByName("recv_tm"));
    const auto type_array = std::dynamic_pointer_cast<arrow::StringArray>(
        batch.GetColumnByName("type"));
    const auto bids_array = std::dynamic_pointer_cast<arrow::ListArray>(
        batch.GetColumnByName("bids"));
    const auto asks_array = std::dynamic_pointer_cast<arrow::ListArray>(
        batch.GetColumnByName("asks"));
    const auto crc32_array = std::dynamic_pointer_cast<arrow::UInt64Array>(
        batch.GetColumnByName("crc32"));
    const auto symbol_array = std::dynamic_pointer_cast<arrow::StringArray>(
        batch.GetColumnByName("symbol"));
    const auto timestamp_array = std::dynamic_pointer_cast<arrow::Int64Array>(
        batch.GetColumnByName("timestamp"));

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
        //        prev_level_book = level_book;
        level_book.accept(response);
      } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        //        std::cerr << "before: " << prev_level_book.str(symbol_str)
        //        << std::endl;
        std::cerr << response.str() << std::endl;
        std::cerr << " after: " << level_book.str(symbol_str) << std::endl;
        return -1;
      }
    }
  }
}
