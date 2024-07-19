/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include <doctest/doctest.h>

#include <types.hpp>

#include <boost/json.hpp>

TEST_CASE("timestamp_t from_iso_8691") {
  const auto tp_str = "2024-04-13T18:10:04.220677Z";
  const auto micros = krakpot::timestamp_t::from_iso_8601(tp_str);
  CHECK(micros == 1713031804220677);
}

TEST_CASE("timestamp_t to_iso_8601") {
  const auto micros = 1713031804220677;
  const auto tp_str = krakpot::timestamp_t::to_iso_8601(micros);
  CHECK(tp_str == "2024-04-13T18:10:04.220677Z");
}

TEST_CASE("timestamp_t roundtrip iso8601") {
  const auto orig_str = std::string{"2024-04-04T22:01:57.362980Z"};
  const auto micros = krakpot::timestamp_t::from_iso_8601(orig_str);
  const auto tstamp = krakpot::timestamp_t{micros};
  const auto rt_str = tstamp.str();
  CHECK(rt_str == orig_str);
}

