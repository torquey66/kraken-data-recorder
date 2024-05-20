/* Copyright (C) 2024 John C. Finley - All rights reserved */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <level_book.hpp>
#include <responses.hpp>

#include <simdjson.h>

#include <sstream>

/**
 * See https://docs.kraken.com/api/docs/websocket-v2/book for examples
 */

static const std::string example_snapshot_str = R"RESPONSE(
{
   "channel": "book",
   "type": "snapshot",
   "data": [
      {
         "symbol": "BTC/USD",
         "bids": [
            {
               "price": 45283.5,
               "qty": 0.10000000
            },
            {
               "price": 45283.4,
               "qty": 1.54582015
            },
            {
               "price": 45282.1,
               "qty": 0.10000000
            },
            {
               "price": 45281.0,
               "qty": 0.10000000
            },
            {
               "price": 45280.3,
               "qty": 1.54592586
            },
            {
               "price": 45279.0,
               "qty": 0.07990000
            },
            {
               "price": 45277.6,
               "qty": 0.03310103
            },
            {
               "price": 45277.5,
               "qty": 0.30000000
            },
            {
               "price": 45277.3,
               "qty": 1.54602737
            },
            {
               "price": 45276.6,
               "qty": 0.15445238
            }
         ],
         "asks": [
            {
               "price": 45285.2,
               "qty": 0.00100000
            },
            {
               "price": 45286.4,
               "qty": 1.54571953
            },
            {
               "price": 45286.6,
               "qty": 1.54571109
            },
            {
               "price": 45289.6,
               "qty": 1.54560911
            },
            {
               "price": 45290.2,
               "qty": 0.15890660
            },
            {
               "price": 45291.8,
               "qty": 1.54553491
            },
            {
               "price": 45294.7,
               "qty": 0.04454749
            },
            {
               "price": 45296.1,
               "qty": 0.35380000
            },
            {
               "price": 45297.5,
               "qty": 0.09945542
            },
            {
               "price": 45299.5,
               "qty": 0.18772827
            }
         ],
         "checksum": 3310070434
      }
   ]
}
)RESPONSE";

static const std::string example_update_str = R"RESPONSE(
  {
    "channel": "book",
    "type": "update",
    "data": [
        {
            "symbol": "MATIC/USD",
            "bids": [
                {
                    "price": 0.5657,
                    "qty": 1098.3947558
                }
            ],
            "asks": [],
            "checksum": 2114181697,
            "timestamp": "2023-10-06T17:35:55.440295Z"
        }
    ]
  }
  )RESPONSE";

/*
  45285210000045286415457195345286615457110945289615456091145290215890660452918154553491452947445474945296135380000452975994554245299518772827
*/

TEST_CASE("book_t doc example snapshot") {
  simdjson::ondemand::parser parser;
  simdjson::padded_string snap_response{example_snapshot_str};
  simdjson::ondemand::document snap_doc = parser.iterate(snap_response);
  const auto snap = krakpot::response::book_t::from_json(snap_doc);
  CHECK(snap.crc32() == 3310070434);

  auto book = krakpot::model::level_book_t{krakpot::e_100};
  book.accept(snap);
  CHECK(book.crc32("BTC/USD") == snap.crc32());
}

/*
TEST_CASE("book_t doc example update") {
  simdjson::ondemand::parser parser;
  simdjson::padded_string snap_response{example_snapshot_str};
  simdjson::ondemand::document snap_doc = parser.iterate(snap_response);
  auto snap = krakpot::response::book_t::from_json(snap_doc);

  simdjson::padded_string update_response{example_update_str};
  simdjson::ondemand::document update_doc = parser.iterate(update_response);

  CHECK(book.crc32() == 2439117997);
}
*/
