#include <doctest/doctest.h>

#include <decimal.hpp>
#include <types.hpp>

#include <map>
#include <tuple>
#include <vector>

using kdr::decimal_t;

TEST_SUITE("decimal_t") {

  TEST_CASE("ctor") {

    CHECK(decimal_t().str() == "0.0");
    CHECK(decimal_t(std::string("0")).str() == "0.0");
    CHECK(decimal_t(std::string("0.0")).str() == "0.0");
    CHECK(decimal_t(std::string("0.0000")).str() == "0.0");
    CHECK(decimal_t(std::string("0000.0")).str() == "0.0");
    CHECK(decimal_t(std::string("0000.0000")).str() == "0.0");

    const auto too_long = std::string(
        "0000000000000000000000000000000000000000000000000000000000000000");
    CHECK_THROWS_WITH(decimal_t(too_long).str(),
                      "decimal_t str size: 64 exceeds max allowed size: 48");

    CHECK(decimal_t(std::string("1234.567")).str() == "1234.567");
    CHECK(decimal_t(std::string("   1234.567")).str() == "1234.567");
    CHECK(decimal_t(std::string("1234.567   ")).str() == "1234.567");
    CHECK(decimal_t(std::string("13600.00000000")).str() == "13600");
  }

  TEST_CASE("assignment operator") {
    const auto src = decimal_t(std::string("1234.567"));
    auto dst = decimal_t();
    CHECK(dst == decimal_t());
    dst = src;
    CHECK(dst == src);
    CHECK(dst.str_view(3) == src.str_view(3));
  }

  TEST_CASE("str with precision") {
    CHECK(decimal_t().str(3) == std::string("0.000"));
    CHECK(decimal_t(std::string("123")).str(2) == std::string("123.00"));
    CHECK(decimal_t(std::string("123.345")).str(3) == std::string("123.345"));
    CHECK(decimal_t(std::string("0123.345")).str(3) == std::string("123.345"));
    CHECK(decimal_t(std::string("123.345")).str(4) == std::string("123.3450"));
    CHECK(decimal_t(std::string("123.3450")).str(3) == std::string("123.345"));
    CHECK(decimal_t(std::string("00123.345")).str(4) ==
          std::string("123.3450"));
    CHECK(decimal_t(std::string("00123.3450")).str(3) ==
          std::string("123.345"));
  }

  TEST_CASE("comparisons") {
    CHECK(decimal_t(std::string("123")) == decimal_t(std::string("123")));
    CHECK(decimal_t(std::string("123.0")) == decimal_t(std::string("123")));
    CHECK(decimal_t(std::string("00123.0")) == decimal_t(std::string("123")));

    CHECK(decimal_t(std::string("123")) != decimal_t(std::string("124")));
    CHECK(decimal_t(std::string("123")) < decimal_t(std::string("124")));

    CHECK(decimal_t(std::string("124")) != decimal_t(std::string("123")));
    CHECK(decimal_t(std::string("124")) > decimal_t(std::string("123")));

    CHECK(decimal_t(std::string("123")) != decimal_t(std::string("123.1")));
    CHECK(decimal_t(std::string("123")) < decimal_t(std::string("123.1")));

    CHECK(decimal_t(std::string("123.1")) != decimal_t(std::string("123")));
    CHECK(decimal_t(std::string("123.1")) > decimal_t(std::string("123")));
  }

  TEST_CASE("decimal_t - crc32") {
    using row_t =
        std::tuple<double, std::string, kdr::integer_t, kdr::integer_t>;
    std::vector<row_t> rows = {
        {94510.50669693, "94510.50669693", 8, 3977769420},
        {232489.98702916, "232489.98702916", 8, 2038959249},
        {244770.01655926, "244770.01655926", 8, 270355881},
        {103394.23779803, "103394.23779803", 8, 1187982405},
        {120226.44704447, "120226.44704447", 8, 1863093490},
        {122811.44535027, "122811.44535027", 8, 222900167},
        {185766.68965043, "185766.68965043", 8, 1029399429},
        {95339.83830809, "95339.83830809", 8, 685228061},
        {32960.86333331, "32960.86333331", 8, 3453759157},
        {86326.77204454, "86326.77204454", 8, 4233029189},
        {255965.95133811, "255965.95133811", 8, 1235801247},
        {264465.46682136, "264465.46682136", 8, 2136218647},
        {198234.50375152, "198234.50375152", 8, 2310003932},
        {263077.71115063, "263077.71115063", 8, 2954165314},
        {135283.23181445, "135283.23181445", 8, 1270741290},
        {232726.34707055, "232726.34707055", 8, 3153796151},
        {211909.56878553, "211909.56878553", 8, 2152615557},
        {16666.66666666, "16666.66666666", 8, 298152451},
        {13600.00000000, "13600.00000000", 8, 2501576451},
        {1000.00000000, "1000.00000000", 8, 1071376280},
    };
    for (const auto &row : rows) {
      const auto [value, token, precision, expected] = row;
      const kdr::decimal_t decimal{token};
      CHECK(decimal.str(precision) == token);

      const boost::crc_32_type in_crc;
      auto actual_crc = in_crc;
      decimal.process(actual_crc, precision);
      CHECK(actual_crc() == expected);
    }
  }
}
