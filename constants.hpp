/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include <utility>

/**
 * Some of these constants are candidates for future configurability,
 * but for now we require a rebuild to change them.
 */
namespace krakpot {

static constexpr double c_NaN = std::numeric_limits<double>::signaling_NaN();
static constexpr char c_NaN_str[] = "NaN";

static constexpr size_t c_ping_interval_secs = 30;

static constexpr char c_kraken_host[] = "ws.kraken.com";
static constexpr char c_kraken_port[] = "443";

// TODO: make this a configurable option
static constexpr char c_parquet_dir[] = "/tmp/krakpot_parquet";

static constexpr char c_book_type_snapshot[] = "snapshot";
static constexpr char c_book_type_update[] = "update";

static constexpr int64_t c_book_depth = 1000;

} // namespace krakpot
