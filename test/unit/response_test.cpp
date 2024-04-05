/* Copyright (C) 2024 John C. Finley - All rights reserved */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../../responses.hpp" // !@# TODO: fix this when we put responses.cpp into a library

#include <nlohmann/json.hpp>
#include <simdjson.h>

TEST_CASE("book_t parse") {
  static const std::string test_str = R"RESPONSE(
  {
    "channel": "book",
    "type": "update",
    "data": [
      {
        "symbol": "ETH/AUD",
        "bids": [],
        "asks": [
          {
            "price": 5061.98,
            "qty": 0E-8
          },
         {
            "price": 5061.96,
            "qty": 0.59920000
          }
        ],
        "checksum": 745923772,
        "timestamp": "2024-04-04T22:01:57.362980Z"
      }
    ]
  }
  )RESPONSE";

  simdjson::ondemand::parser parser;
  simdjson::padded_string test_response{test_str};
  simdjson::ondemand::document doc = parser.iterate(test_response);

  const auto book = krakpot::response::book_t::from_json(doc);
  const auto result_str = book.str();
  const auto result_json = nlohmann::json::parse(result_str);

  const auto test_json = nlohmann::json::parse(test_str);

  CHECK(test_json == result_json);
}
