#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

/**
 * Some of these constants are candidates for future configurability,
 * but for now we require a rebuild to change them.
 */
namespace kdr {

static constexpr char c_license[] =
    "kraken-data-recorder © 2023 by John C. Finley is licensed under Creative "
    "Commons Attribution-NoDerivatives 4.0 International. To view a copy of "
    "this license, visit https://creativecommons.org/licenses/by-nd/4.0/";

static constexpr char c_app_name[] = "kdr";

static constexpr size_t c_expected_cacheline_size = 64;

/** Websocket protocol strings */
static constexpr char c_request_channel[] = "channel";
static constexpr char c_request_method[] = "method";
static constexpr char c_request_params[] = "params";
static constexpr char c_request_req_id[] = "req_id";

static constexpr char c_param_depth[] = "depth";
static constexpr char c_param_snapshot[] = "snapshot";
static constexpr char c_param_symbol[] = "symbol";

static constexpr char c_response_data[] = "data";
static constexpr char c_response_channel[] = "channel";
static constexpr char c_response_method[] = "method";

static constexpr char c_channel_instrument[] = "instrument";
static constexpr char c_channel_book[] = "book";
static constexpr char c_channel_trade[] = "trade";
static constexpr char c_channel_heartbeat[] = "heartbeat";

static constexpr char c_method_ping[] = "ping";
static constexpr char c_method_pong[] = "pong";
static constexpr char c_method_subscribe[] = "subscribe";

static constexpr char c_instrument_assets[] = "assets";
static constexpr char c_instrument_pairs[] = "pairs";
static constexpr char c_instrument_snapshot[] = "snapshot";
static constexpr char c_instrument_update[] = "update";

} // namespace kdr
