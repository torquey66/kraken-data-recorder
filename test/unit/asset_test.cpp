/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include <doctest/doctest.h>

#include <simdjson.h>
#include <asset.hpp>
#include <constants.hpp>

using namespace krakpot::response;

TEST_CASE("asset_t equality") {
  const asset_t thing1;
  const asset_t thing2 = thing1;
  CHECK(thing1 == thing2);
  const asset_t thing3{thing2};
  CHECK(thing2 == thing3);
}

TEST_CASE("asset_t - inequality") {
  std::optional<double_t> no_margin_rate;
  std::optional<double_t> margin_rate_1 = std::make_optional<double>(1.1);
  std::optional<double_t> margin_rate_2 = std::make_optional<double>(1.2);

  CHECK(
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled} !=
      asset_t{false, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled});

  CHECK(
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled} !=
      asset_t{true, 0.2, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled});

  CHECK(
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled} !=
      asset_t{true, 0.1, "thing2", no_margin_rate, 8, 3, asset_t::e_enabled});

  CHECK(
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled} !=
      asset_t{true, 0.1, "thing1", margin_rate_1, 8, 3, asset_t::e_enabled});

  CHECK(asset_t{true, 0.1, "thing1", margin_rate_1, 8, 3, asset_t::e_enabled} !=
        asset_t{true, 0.1, "thing1", margin_rate_2, 8, 3, asset_t::e_enabled});

  CHECK(
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled} !=
      asset_t{true, 0.1, "thing1", no_margin_rate, 6, 3, asset_t::e_enabled});

  CHECK(
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled} !=
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 7, asset_t::e_enabled});

  CHECK(
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_enabled} !=
      asset_t{true, 0.1, "thing1", no_margin_rate, 8, 3, asset_t::e_disabled});
}

TEST_CASE("asset_t from_json") {
  static const std::string test_str = R"RESPONSE(
  [{"borrowable":true,"collateral_value":1.0,"id":"USD","margin_rate":0.025,"precision":4,"precision_display":2,"status":"enabled"}]
  )RESPONSE";

  const asset_t expected{true,
                         1.0,
                         "USD",
                         std::make_optional<double>(0.025),
                         4,
                         2,
                         asset_t::e_enabled};

  simdjson::ondemand::parser parser;
  simdjson::padded_string padded{test_str};
  simdjson::ondemand::document doc = parser.iterate(padded);
  for (simdjson::fallback::ondemand::object obj : doc) {
    const asset_t actual = asset_t::from_json(obj);
    CHECK(expected == actual);
  }
}
