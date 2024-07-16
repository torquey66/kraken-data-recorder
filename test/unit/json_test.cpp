/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include <doctest/doctest.h>

#include <asset.hpp>
#include <config.hpp>
#include <pair.hpp>

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

const auto pair = response::pair_t{"EUR",
                                   krakpot::decimal_t{0.5},
                                   5,
                                   true,
                                   std::optional<double_t>{},
                                   false,
                                   std::optional<integer_t>{},
                                   std::optional<integer_t>{},
                                   krakpot::decimal_t{0.00001},
                                   5,
                                   krakpot::decimal_t{1E-8},
                                   krakpot::decimal_t{0.5},
                                   8,
                                   "USD",
                                   response::pair_t::status_t{4},
                                   "EUR/USD"};

TEST_CASE("pair_t obj roundtrip") {
  const auto pair_json_obj = pair.to_json_obj();
  const auto rt_pair = response::pair_t::from_json_obj(pair_json_obj);

  MESSAGE(pair.price_increment());

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
    CHECK(pair.cost_min() == decimal_t{"50"});
  }
}

/******************************************************************************/
