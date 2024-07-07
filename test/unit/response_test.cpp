/* Copyright (C) 2024 John C. Finley - All rights reserved */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <responses.hpp>

#include <nlohmann/json.hpp>
#include <simdjson.h>

#include <sstream>

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

TEST_CASE("book_t parse") {
  static const std::string test_str = R"RESPONSE(
{"channel":"book","data":[{"asks":[{"price":34726.4,"qty":0},{"price":34739.7,"qty":2.1541}],"bids":[],"checksum":4022926185,"symbol":"BTC/USD","timestamp":"2022-06-13T08:09:10.123456Z"}],"type":"update"}
  )RESPONSE";

  simdjson::ondemand::parser parser;
  simdjson::padded_string test_response{test_str};
  simdjson::ondemand::document doc = parser.iterate(test_response);
  const auto book = krakpot::response::book_t::from_json(doc);

  const auto result_str = book.str(1, 8);
  const auto result_json = nlohmann::json::parse(result_str);
  const auto test_json = nlohmann::json::parse(test_str);
  CHECK(test_json == result_json);
}
