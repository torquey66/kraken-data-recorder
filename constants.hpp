/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

/**
 * Some of these constants are candidates for future configurability,
 * but for now we require a rebuild to change them.
 */
namespace krakpot {

static constexpr size_t c_ping_interval_secs = 60;

static constexpr char c_kraken_host[] = "ws.kraken.com";
static constexpr char c_kraken_port[] = "443";

} // namespace krakpot
