/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include <doctest/doctest.h>

#include <asset.hpp>
#include <config.hpp>
#include <pair.hpp>
#include <responses.hpp>

using namespace krakpot;

/******************************************************************************/

const auto pair_filter = config_t::symbol_filter_t{
    "BTC/USD",
    "ETH/BTC",
};

const auto config = config_t{13,
                             "ws.kraken.com",
                             "443",
                             pair_filter,
                             "/krakpot-data",
                             depth_t{25},
                             true,
                             true};

TEST_CASE("config_t obj roundtrip") {
  const auto config_json_obj = config.to_json_obj();
  const auto rt_config = config_t::from_json(config_json_obj);
  CHECK(config == rt_config);
}

TEST_CASE("config_t str roundtrip") {
  const auto config_json_str = config.str();
  const auto rt_config = config_t::from_json_str(config_json_str);
  CHECK(config == rt_config);
}

/******************************************************************************/

const auto asset =
    response::asset_t{true,
                      100.00,
                      "42",
                      3.34,
                      8,
                      3,
                      response::asset_t::e_fundingtemporarilydisabled};

TEST_CASE("asset_t obj roundtrip") {
  const auto asset_json_obj = asset.to_json_obj();
  const auto rt_asset = response::asset_t::from_json_obj(asset_json_obj);
  CHECK(asset == rt_asset);
}

/******************************************************************************/

const auto pair =
    response::pair_t{"EUR",
                     krakpot::decimal_t{0.5, krakpot::precision_t{5}},
                     5,
                     true,
                     std::optional<double_t>{},
                     false,
                     std::optional<integer_t>{},
                     std::optional<integer_t>{},
                     krakpot::decimal_t{0.00001, krakpot::precision_t{5}},
                     5,
                     krakpot::decimal_t{1E-8, krakpot::precision_t{8}},
                     krakpot::decimal_t{0.5, krakpot::precision_t{8}},
                     8,
                     "USD",
                     response::pair_t::status_t{4},
                     "EUR/USD"};

TEST_CASE("pair_t obj roundtrip") {
  const auto pair_json_obj = pair.to_json_obj();
  const auto rt_pair = response::pair_t::from_json_obj(pair_json_obj);

  CHECK(pair.base() == rt_pair.base());
  CHECK(pair.cost_min() == rt_pair.cost_min());
  CHECK(pair.cost_precision() == rt_pair.cost_precision());
  CHECK(pair.has_index() == rt_pair.has_index());
  CHECK(pair.margin_initial() == rt_pair.margin_initial());
  CHECK(pair.marginable() == rt_pair.marginable());
  CHECK(pair.position_limit_long() == rt_pair.position_limit_long());
  CHECK(pair.position_limit_short() == rt_pair.position_limit_short());
  CHECK(pair.price_increment() == rt_pair.price_increment());
  CHECK(pair.price_precision() == rt_pair.price_precision());
  CHECK(pair.qty_increment() == rt_pair.qty_increment());
  CHECK(pair.qty_min() == rt_pair.qty_min());
  CHECK(pair.qty_precision() == rt_pair.qty_precision());
  CHECK(pair.quote() == rt_pair.quote());
  CHECK(pair.status() == rt_pair.status());
  CHECK(pair.symbol() == rt_pair.symbol());

  CHECK(pair == rt_pair);
}

TEST_CASE("pair_t double parsing") {
  static const std::string json_with_int = R"RESPONSE(
{"symbol":"USD/JPY","base":"USD","quote":"JPY","status":"online","qty_precision":8,"qty_increment":1E-8,"price_precision":3,"cost_precision":3,"marginable":false,"has_index":true,"cost_min":50,"tick_size":1E-3,"price_increment":1E-3,"qty_min":5E0}
)RESPONSE";

  static const std::string json_with_double = R"RESPONSE(
{"symbol":"USD/JPY","base":"USD","quote":"JPY","status":"online","qty_precision":8,"qty_increment":1E-8,"price_precision":3,"cost_precision":3,"marginable":false,"has_index":true,"cost_min":50.0,"tick_size":1E-3,"price_increment":1E-3,"qty_min":5E0}
)RESPONSE";

  static const std::string json_with_string = R"RESPONSE(
{"symbol":"USD/JPY","base":"USD","quote":"JPY","status":"online","qty_precision":8,"qty_increment":1E-8,"price_precision":3,"cost_precision":3,"marginable":false,"has_index":true,"cost_min":"50.0","tick_size":1E-3,"price_increment":1E-3,"qty_min":5E0}
)RESPONSE";

  static const std::string json_with_foo = R"RESPONSE(
{"symbol":"BTC/JPY","base":"BTC","quote":"JPY","status":"online","qty_precision":8,"qty_increment":1E-8,"price_precision":0,"cost_precision":3,"marginable":false,"has_index":true,"cost_min":50,"tick_size":1,"price_increment":1,"qty_min":1E-4}
)RESPONSE";

  const std::vector<std::string> inputs = {json_with_int, json_with_double,
                                           json_with_string, json_with_foo};
  for (const auto& input : inputs) {
    const boost::json::object json_obj = boost::json::parse(input).as_object();
    const auto pair = response::pair_t::from_json_obj(json_obj);
    CHECK(pair.cost_min() == decimal_t{"50", pair.cost_precision()});
  }
}

/******************************************************************************/

TEST_CASE("book_t parse") {
  static const std::string pair_str = R"RESPONSE(
{"recv_tm":1721400663780544,"base":"BTC","cost_min":"0.50000","cost_precision":5,"has_index":true,"margin_initial":0.2,"marginable":true,"position_limit_long":300,"position_limit_short":300,"price_increment":"0.1","price_precision":1,"qty_increment":"0.00000001","qty_min":"0.00010000","qty_precision":8,"quote":"USD","status":"online","symbol":"BTC/USD"}
  )RESPONSE";

  const boost::json::object pair_obj = boost::json::parse(pair_str).as_object();
  const auto pair = krakpot::response::pair_t::from_json_obj(pair_obj);

  static const std::string book_str = R"RESPONSE(
{"channel":"book","data":[{"asks":[{"price":34726.4,"qty":0E0},{"price":34739.7,"qty":2.1541}],"bids":[],"checksum":4022926185,"symbol":"BTC/USD","timestamp":"2022-06-13T08:09:10.123456Z"}],"type":"update"}
  )RESPONSE";

  const boost::json::object book_obj = boost::json::parse(book_str).as_object();
  const auto book = krakpot::response::book_t::from_json_obj(book_obj, pair);

  const auto result_str = book.str();
  const auto result_json = boost::json::parse(result_str);
  const auto test_json = boost::json::parse(book_str);
  CHECK(test_json == result_json);
}

TEST_CASE("book_t double parsing") {
  static const std::string pair_str = R"RESPONSE(
{"recv_tm":1721422121075613,"base":"LUNA","cost_min":"0.50000","cost_precision":5,"has_index":false,"marginable":false,"price_increment":"0.00000001","price_precision":8,"qty_increment":"0.00000001","qty_min":"45000.00000000","qty_precision":8,"quote":"USD","status":"online","symbol":"LUNA/USD"}
  )RESPONSE";

  const boost::json::object pair_obj = boost::json::parse(pair_str).as_object();
  const auto pair = krakpot::response::pair_t::from_json_obj(pair_obj);

  static const std::string book_str = R"RESPONSE(
{"channel":"book","type":"snapshot","data":[{"symbol":"LUNA/USD","bids":[{"price":0.00009208,"qty":189199.70059805},{"price":0.00009207,"qty":1080000.00000000},{"price":0.00009205,"qty":32590983.16132537},{"price":0.00009204,"qty":23935926.19056932},{"price":0.00009137,"qty":38776166.69502590},{"price":0.00009136,"qty":4932179.44164000},{"price":0.00009132,"qty":7240859.18028000},{"price":0.00009116,"qty":49100.13613783},{"price":0.00009106,"qty":87282.35395073},{"price":0.00009103,"qty":117218485.35700187}],"asks":[{"price":0.00009310,"qty":173066.50010456},{"price":0.00009355,"qty":1080000.00000000},{"price":0.00009364,"qty":35033.06230468},{"price":0.00009366,"qty":237807.20195099},{"price":0.00009400,"qty":450000.00000000},{"price":0.00009478,"qty":48825.64031974},{"price":0.00009491,"qty":56284.43346312},{"price":0.00009492,"qty":307613.15034360},{"price":0.00009495,"qty":7445079.90885100},{"price":0.00009496,"qty":10930605.26462782}],"checksum":2095654264}]}
  )RESPONSE";

  const boost::json::object book_obj = boost::json::parse(book_str).as_object();
  const auto book = krakpot::response::book_t::from_json_obj(book_obj, pair);

  const auto result_str = book.str();
  const auto result_json = boost::json::parse(result_str);
  const auto test_json = boost::json::parse(book_str);

  const auto b1 =
      krakpot::response::book_t::from_json_obj(result_json.as_object(), pair);
  const auto b2 =
      krakpot::response::book_t::from_json_obj(test_json.as_object(), pair);
  CHECK(b1 == b2);

  //  CHECK(test_json == result_json);
}