/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include <doctest/doctest.h>

#include <config.hpp>

using namespace krakpot;

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
