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

/*
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
*/

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

static const std::string pair_str = R"RESPONSE(
[
  {
    "recv_tm": 1719859326224708,
    "base": "GST",
    "cost_min": 0.5,
    "cost_precision": 5,
    "has_index": false,
    "marginable": false,
    "price_increment": 0.001,
    "price_precision": 3,
    "qty_increment": 1E-8,
    "qty_min": 200.0,
    "qty_precision": 8,
    "quote": "USD",
    "status": "online",
    "symbol": "GST/USD"
  }
]
)RESPONSE";

static const std::string snapshot_str = R"RESPONSE(
{
  "channel": "book",
  "type": "snapshot",
  "data": [
    {
      "symbol": "GST/USD",
      "bids": [
        {
          "price": 0.016,
          "qty": 255965.95133811
        },
        {
          "price": 0.015,
          "qty": 264465.46682136
        },
        {
          "price": 0.014,
          "qty": 198234.50375152
        },
        {
          "price": 0.013,
          "qty": 263077.71115063
        },
        {
          "price": 0.012,
          "qty": 135283.23181445
        },
        {
          "price": 0.011,
          "qty": 232726.34707055
        },
        {
          "price": 0.010,
          "qty": 211909.56878553
        },
        {
          "price": 0.009,
          "qty": 16666.66666666
        },
        {
          "price": 0.008,
          "qty": 13600.00000000
        },
        {
          "price": 0.007,
          "qty": 1000.00000000
        }
      ],
      "asks": [
        {
          "price": 0.017,
          "qty": 94510.50669693
        },
        {
          "price": 0.018,
          "qty": 232489.98702916
        },
        {
          "price": 0.019,
          "qty": 244770.01655926
        },
        {
          "price": 0.020,
          "qty": 103394.23779803
        },
        {
          "price": 0.021,
          "qty": 120226.44704447
        },
        {
          "price": 0.022,
          "qty": 122811.44535027
        },
        {
          "price": 0.023,
          "qty": 185766.68965043
        },
        {
          "price": 0.024,
          "qty": 95339.83830809
        },
        {
          "price": 0.025,
          "qty": 32960.86333331
        },
        {
          "price": 0.026,
          "qty": 86326.77204454
        }
      ],
      "checksum": 1931231958
    }
  ]
}
)RESPONSE";

TEST_CASE("book_t doc example snapshot") {
  auto book = krakpot::model::level_book_t{krakpot::e_10};

  simdjson::ondemand::parser parser;

  simdjson::padded_string pair_response{pair_str};
  simdjson::ondemand::document pair_doc = parser.iterate(pair_response);
  for (simdjson::fallback::ondemand::object pair_obj : pair_doc) {
    const auto pair = krakpot::response::pair_t::from_json(pair_obj);
    book.accept(pair);
  }

  simdjson::padded_string snap_response{snapshot_str};
  simdjson::ondemand::document snap_doc = parser.iterate(snap_response);
  const auto snap = krakpot::response::book_t::from_json(snap_doc);
  book.accept(snap);
}
