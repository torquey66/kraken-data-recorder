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
                                   0.5,
                                   5,
                                   true,
                                   std::optional<double_t>{},
                                   false,
                                   std::optional<integer_t>{},
                                   std::optional<integer_t>{},
                                   0.00001,
                                   5,
                                   1E-8,
                                   0.5,
                                   8,
                                   "USD",
                                   response::pair_t::status_t{4},
                                   "EUR/USD"};

TEST_CASE("pair_t obj roundtrip") {
  const auto pair_json_obj = pair.to_json_obj();
  const auto rt_pair = response::pair_t::from_json_obj(pair_json_obj);
  CHECK(pair == rt_pair);
}
