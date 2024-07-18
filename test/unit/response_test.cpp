/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include <doctest/doctest.h>

#include <responses.hpp>

#include <boost/json.hpp>

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

/*

TEST_CASE("book_t parse") {
  static const std::string test_str = R"RESPONSE(
{"channel":"book","data":[{"asks":[{"price":34726.4,"qty":0E0},{"price":34739.7,"qty":2.1541}],"bids":[],"checksum":4022926185,"symbol":"BTC/USD","timestamp":"2022-06-13T08:09:10.123456Z"}],"type":"update"}
  )RESPONSE";

  const boost::json::object book_obj = boost::json::parse(test_str).as_object();
  const auto book = krakpot::response::book_t::from_json_obj(book_obj);

  const auto result_str = book.str(3, 8);
  const auto result_json = boost::json::parse(result_str);
  const auto test_json = boost::json::parse(test_str);
  CHECK(test_json == result_json);
}

*/
