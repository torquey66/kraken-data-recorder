#include <doctest/doctest.h>

#include <level_book.hpp>

#include <simdjson.h>

#include <sstream>

TEST_SUITE("level_book_t") {

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
{"channel":"book","type":"snapshot","data":[{"symbol":"GST/USD","bids":[{"price":0.016,"qty":255965.95133811},{"price":0.015,"qty":264465.46682136},{"price":0.014,"qty":198234.50375152},{"price":0.013,"qty":263077.71115063},{"price":0.012,"qty":135283.23181445},{"price":0.011,"qty":232726.34707055},{"price":0.010,"qty":211909.56878553},{"price":0.009,"qty":16666.66666666},{"price":0.008,"qty":13600.00000000},{"price":0.007,"qty":1000.00000000}],"asks":[{"price":0.017,"qty":94510.50669693},{"price":0.018,"qty":232489.98702916},{"price":0.019,"qty":244770.01655926},{"price":0.020,"qty":103394.23779803},{"price":0.021,"qty":120226.44704447},{"price":0.022,"qty":122811.44535027},{"price":0.023,"qty":185766.68965043},{"price":0.024,"qty":95339.83830809},{"price":0.025,"qty":32960.86333331},{"price":0.026,"qty":86326.77204454}],"checksum":1931231958}]}
)RESPONSE";

  TEST_CASE("book_t doc example snapshot") {
    auto book = kdr::model::level_book_t{kdr::model::depth_10};

    simdjson::ondemand::parser parser;

    simdjson::padded_string pair_response{pair_str};
    simdjson::ondemand::document pair_doc = parser.iterate(pair_response);
    for (simdjson::fallback::ondemand::object pair_obj : pair_doc) {
      const auto pair = kdr::model::pair_t::from_json(pair_obj);
      book.accept(pair);
    }

    simdjson::padded_string snap_response{snapshot_str};
    simdjson::ondemand::document snap_doc = parser.iterate(snap_response);
    const auto snap = kdr::response::book_t::from_json(snap_doc);
    book.accept(snap);
  }
}
