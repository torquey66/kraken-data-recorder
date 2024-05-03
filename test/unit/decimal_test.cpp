/* Copyright (C) 2024 John C. Finley - All rights reserved */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <decimal.hpp>

#include <iostream>
#include <map>

TEST_CASE("sanity check - zero my hero") {
  const auto decimal = krakpot::decimal_t{0.0, std::string{"0.0"}};
  CHECK(decimal.value() == 0.0);
  CHECK(decimal.str() == "0.0");
}

TEST_CASE("sanity check - zero string_view") {
  const auto zero = std::string{"0.0"};
  const auto decimal = krakpot::decimal_t{0.0, std::string_view{zero}};
  CHECK(decimal.value() == 0.0);
  CHECK(decimal.str() == "0.0");
}

TEST_CASE("sanity check - comparison") {
  const auto d1 = krakpot::decimal_t{18.82, std::string{"18.82"}};
  const auto d2 = krakpot::decimal_t{19.82, std::string{"19.82"}};
  CHECK(d1 != d2);
  CHECK(d1 < d2);
  CHECK(d2 > d1);

  const auto d3 = krakpot::decimal_t{19.82, std::string{"19.82"}};
  CHECK(d2 == d3);
}

TEST_CASE("sanity check - token trimming") {
  const auto samples = std::map<std::string, std::string> {
    { "0.10000000", "10000000" },
    { "0.10000000", "10000000" },
    { "1.54582015", "154582015" },
    { "0.07990000", "7990000" },
    { "45283.4", "452834" },
    { "45281.0", "452810" },
    { "45280.3", "452803"},
  };
  for (const auto& sample: samples) {
    const auto& [raw, trimmed] = sample;
    const auto token = krakpot::token_t{raw};
    CHECK(token.trimmed() == trimmed);
  }
}
