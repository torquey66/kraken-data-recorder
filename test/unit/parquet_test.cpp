/* Copyright (C) 2024 John C. Finley - All rights reserved */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <parquet.hpp>
#include <responses.hpp>

#include <nlohmann/json.hpp>
#include <simdjson.h>

#include <sstream>

TEST_CASE("sanity check") { CHECK(1 == 0); }
