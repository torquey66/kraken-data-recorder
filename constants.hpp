/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include <cstddef>
#include <limits>
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

static constexpr int64_t c_book_depth = 1000;

/** Websocket protocol strings */
static constexpr char c_header_recv_tm[] = "recv_tm";
static constexpr char c_header_channel[] = "channel";
static constexpr char c_header_type[] = "type";

static constexpr char c_response_data[] = "data";

static constexpr char c_response_channel[] = "channel";
static constexpr char c_response_method[] = "method";

static constexpr char c_channel_instrument[] = "instrument";
static constexpr char c_channel_book[] = "book";
static constexpr char c_channel_trade[] = "trade";
static constexpr char c_channel_heartbeat[] = "heartbeat";

static constexpr char c_method_pong[] = "pong";

static constexpr char c_instrument_assets[] = "assets";
static constexpr char c_instrument_pairs[] = "pairs";
static constexpr char c_instrument_snapshot[] = "snapshot";
static constexpr char c_instrument_update[] = "update";

static constexpr char c_asset_borrowable[] = "borrowable";
static constexpr char c_asset_collateral_value[] = "collateral_value";
static constexpr char c_asset_id[] = "id";
static constexpr char c_asset_margin_rate[] = "margin_rate";
static constexpr char c_asset_precision[] = "precision";
static constexpr char c_asset_precision_display[] = "precision_display";
static constexpr char c_asset_status[] = "status";

static constexpr char c_asset_status_depositonly[] = "depositonly";
static constexpr char c_asset_status_disabled[] = "disabled";
static constexpr char c_asset_status_enabled[] = "enabled";
static constexpr char c_asset_status_fundingtemporarilydisabled[] =
    "fundingtemporarilydisabled";
static constexpr char c_asset_status_invalid[] = "invalid";
static constexpr char c_asset_status_withdrawalonly[] = "withdrawalonly";
static constexpr char c_asset_status_workinprogress[] = "workinprogress";

static constexpr char c_pair_base[] = "base";
static constexpr char c_pair_cost_min[] = "cost_min";
static constexpr char c_pair_cost_precision[] = "cost_precision";
static constexpr char c_pair_has_index[] = "has_index";
static constexpr char c_pair_margin_initial[] = "margin_initial";
static constexpr char c_pair_marginable[] = "marginable";
static constexpr char c_pair_pair_marginable[] = "pair_marginable";
static constexpr char c_pair_position_limit_long[] = "position_limit_long";
static constexpr char c_pair_position_limit_short[] = "position_limit_short";
static constexpr char c_pair_price_increment[] = "price_increment";
static constexpr char c_pair_price_precision[] = "price_precision";
static constexpr char c_pair_qty_increment[] = "qty_increment";
static constexpr char c_pair_qty_min[] = "qty_min";
static constexpr char c_pair_qty_precision[] = "qty_precision";
static constexpr char c_pair_quote[] = "quote";
static constexpr char c_pair_status[] = "status";
static constexpr char c_pair_symbol[] = "symbol";

static constexpr char c_pair_status_cancel_only[] = "cancel_only";
static constexpr char c_pair_status_delisted[] = "delisted";
static constexpr char c_pair_status_invalid[] = "invalid";
static constexpr char c_pair_status_limit_only[] = "limit_only";
static constexpr char c_pair_status_maintenance[] = "maintenance";
static constexpr char c_pair_status_online[] = "online";
static constexpr char c_pair_status_post_only[] = "post_only";
static constexpr char c_pair_status_reduce_only[] = "reduce_only";
static constexpr char c_pair_status_work_in_progress[] = "work_in_progress";

static constexpr char c_book_type_snapshot[] = "snapshot";
static constexpr char c_book_type_update[] = "update";

static constexpr char c_book_asks[] = "asks";
static constexpr char c_book_bids[] = "bids";
static constexpr char c_book_checksum[] = "checksum";
static constexpr char c_book_price[] = "price";
static constexpr char c_book_qty[] = "qty";
static constexpr char c_book_side[] = "side";
static constexpr char c_book_symbol[] = "symbol";
static constexpr char c_book_timestamp[] = "timestamp";

static constexpr char c_trades_buy[] = "buy";
static constexpr char c_trades_limit[] = "limit";
static constexpr char c_trades_market[] = "market";
static constexpr char c_trades_ord_type[] = "ord_type";
static constexpr char c_trades_sell[] = "sell";
static constexpr char c_trades_side[] = "side";
static constexpr char c_trades_trade_id[] = "trade_id";

static constexpr char c_metrics_num_bytes[] = "num_bytes";
static constexpr char c_metrics_num_msgs[] = "num_msgs";

} // namespace krakpot
