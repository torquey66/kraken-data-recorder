/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "session.hpp"

#include "constants.hpp"
#include "requests.hpp"

#include <boost/log/trivial.hpp>

#include <chrono>
#include <format>

/**
 * Note that this code was initially derived from the boost::beast
 * websocket examples, with a little advice from ChatGPT on the
 * side. I intended it as a foray in to coroutine-flavored asio
 * programming and in my view the jury's still out on whether the
 * result is easier to read than the vanilla callback approach.
 *
 * Furthermore, the coordination between the ping() and process()
 * operations is clumsy at best. I thought coroutines were supposed to
 * make things of this nature easier...
 *
 * TODO: figure out a better way
 */
namespace asio = boost::asio;
namespace bst = boost::beast;
namespace ws = bst::websocket;

namespace {
void fail(bst::error_code ec, char const *what) {
  BOOST_LOG_TRIVIAL(error) << what << ": " << ec.message();
  throw std::runtime_error(what);
}
} // namespace

namespace krakpot {

session_t::session_t(ioc_t &ioc, ssl_context_t &ssl_context,
                     const config_t &config)
    : m_ioc{ioc}, m_resolver{m_ioc}, m_ws{ioc, ssl_context},
      m_ping_timer{m_ioc}, m_config{config} {

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";

  m_resolver.async_resolve(config.kraken_host(), config.kraken_port(),
                           [this](error_code ec, resolver::results_type rt) {
                             this->on_resolve(ec, rt);
                           });
}

void session_t::start_processing(const recv_cb_t &handle_recv) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";

  // TODO: clean this up - perhaps by adding a timeout to the interface
  while (!m_keep_processing) {
    m_ioc.run_one();
  }

  m_read_buffer.clear();
  m_ws.async_read(m_read_buffer,
                  [this, &handle_recv](error_code ec, size_t size) {
                    this->on_read(ec, size, handle_recv);
                  });
}

void session_t::send(msg_t msg) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << ": " << msg;
  const auto num_bytes_written =
      m_ws.write(asio::buffer(std::string(msg.data(), msg.size())));
  if (num_bytes_written != msg.size()) {
    const std::string message = std::string(__FUNCTION__) + " short write: " +
                                "expected: " + std::to_string(msg.size()) +
                                " actual: " + std::to_string(num_bytes_written);
    throw std::runtime_error(message);
  }

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " returning";
}

void session_t::on_resolve(error_code ec, resolver::results_type results) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }
  boost::beast::get_lowest_layer(m_ws).expires_after(
      std::chrono::seconds(30)); // TODO: constants
  boost::beast::get_lowest_layer(m_ws).async_connect(
      results,
      [this](error_code ec, resolver::results_type::endpoint_type endpoint) {
        this->on_connect(ec, endpoint);
      });

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded";
}

void session_t::on_connect(error_code ec,
                           resolver::results_type::endpoint_type endpoint) {

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }

  auto host = m_config.kraken_host();

  boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
  if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(),
                                host.c_str())) {
    ec = boost::beast::error_code(static_cast<int>(::ERR_get_error()),
                                  asio::error::get_ssl_category());
    fail(ec, __FUNCTION__);
  }

  host += ':' + std::to_string(endpoint.port());
  m_ws.next_layer().async_handshake(
      asio::ssl::stream_base::client,
      [this](error_code ec) { this->on_ssl_handshake(ec); });

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded";
}

void session_t::on_ssl_handshake(error_code ec) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }

  boost::beast::get_lowest_layer(m_ws).expires_never();

  m_ws.set_option(
      ws::stream_base::timeout::suggested(boost::beast::role_type::client));

  m_ws.set_option(ws::stream_base::decorator([](ws::request_type &req) {
    req.set(boost::beast::http::field::user_agent,
            std::string(BOOST_BEAST_VERSION_STRING) +
                " websocket-client-async-ssl");
  }));

  m_ws.async_handshake(m_config.kraken_host(), "/v2",
                       [this](error_code ec) { this->on_handshake(ec); });

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded";
}

void session_t::on_handshake(error_code ec) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }

  m_keep_processing = true;
  on_ping_timer(ec);

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded";
}

void session_t::on_ping_timer(error_code ec) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }

  if (!m_keep_processing) {
    return;
  }

  try {
    const auto ping = request::ping_t{++m_req_id};
    const auto msg = ping.str();
    send(msg);
    ++m_req_id;

    m_ping_timer.expires_from_now(
        boost::posix_time::seconds(m_config.ping_interval_secs()));
    m_ping_timer.async_wait([this](error_code ec) { this->on_ping_timer(ec); });

  } catch (const std::exception &ex) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " " << ex.what();
    m_keep_processing = false;
  }

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded";
}

void session_t::on_write(error_code ec, size_t size) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded size: " << size;
}

void session_t::on_read(error_code ec, size_t size,
                        const recv_cb_t &handle_recv) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }

  try {
    // !@# TODO - see whether we can eliminate this copy by connecting
    // simdjson's iterator directly to buffer sequence
    m_read_msg_str = boost::beast::buffers_to_string(m_read_buffer.data());
    m_read_buffer.consume(size);
    if (!handle_recv(std::string_view(m_read_msg_str))) {
      BOOST_LOG_TRIVIAL(error)
          << __FUNCTION__ << "handle_recv() returned false -- stop processing";
      m_keep_processing = false;
    }
  } catch (const std::exception &ex) {
    BOOST_LOG_TRIVIAL(error) << ex.what();
    m_keep_processing = false;
  }

  if (m_keep_processing) {
    m_read_msg_str.clear();
    m_ws.async_read(m_read_buffer,
                    [this, &handle_recv](error_code ec, size_t size) {
                      this->on_read(ec, size, handle_recv);
                    });
  }

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded size: " << size;
}

void session_t::on_close(error_code ec) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded";
}

} // namespace krakpot
