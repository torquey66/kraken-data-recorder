/* Copyright (C) 2024 John C. Finley - All rights reserved */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <decimal.hpp>

#include <iostream>

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
