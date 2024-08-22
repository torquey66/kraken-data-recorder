#include "session.hpp"

#include "constants.hpp"
#include "requests.hpp"

#include <boost/log/trivial.hpp>

#include <chrono>
#include <format>

namespace asio = boost::asio;
namespace bst = boost::beast;
namespace ws = bst::websocket;

namespace kdr {

session_t::session_t(ioc_t &ioc, ssl_context_t &ssl_context,
                     const config_t &config)
    : m_ioc{ioc}, m_resolver{m_ioc}, m_ws{ioc, ssl_context},
      m_ping_timer{m_ioc}, m_config{config} {
  if (m_config.ping_interval_secs() < 1) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__
        << " ping_interval_secs (=" << m_config.ping_interval_secs()
        << ") must be at least one";
  }
}

void session_t::fail(bst::error_code ec, char const *what) {
  BOOST_LOG_TRIVIAL(error) << what << ": " << ec.message();
  m_keep_processing = false;
}

void session_t::start_processing(const recv_cb_t &handle_recv) {
  m_handle_recv = handle_recv;
  m_keep_processing = true;
  m_resolver.async_resolve(m_config.kraken_host(), m_config.kraken_port(),
                           [this](error_code ec, resolver::results_type rt) {
                             this->on_resolve(ec, rt);
                           });
}

void session_t::send(msg_t msg) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << ": " << msg;
  const auto num_bytes_written =
      m_ws.write(asio::buffer(std::string(msg.data(), msg.size())));
  if (num_bytes_written != msg.size()) {
    const std::string message = std::string(__FUNCTION__) + " short write: " +
                                "expected: " + std::to_string(msg.size()) +
                                " actual: " + std::to_string(num_bytes_written);
    throw std::runtime_error(message);
  }
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

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded";
  m_keep_processing = true;
  m_read_buffer.clear();
  m_ws.async_read(m_read_buffer, [this](error_code ec, size_t size) {
    this->on_read(ec, size);
  });

  m_ping_timer.expires_from_now(
      boost::posix_time::seconds(m_config.ping_interval_secs()));
  m_ping_timer.async_wait([this](error_code ec) { this->on_ping_timer(ec); });
}

void session_t::on_ping_timer(error_code ec) {
  if (ec) {
    fail(ec, __FUNCTION__);
  }

  if (!m_keep_processing) {
    BOOST_LOG_TRIVIAL(debug)
        << __FUNCTION__
        << " returning -  m_keep_processing: " << m_keep_processing;
    ;
    return;
  }

  try {
    const auto ping = request::ping_t{++m_req_id};
    const auto msg = ping.str();
    send(msg);

    m_ping_timer.expires_from_now(
        boost::posix_time::seconds(m_config.ping_interval_secs()));
    m_ping_timer.async_wait([this](error_code ec) { this->on_ping_timer(ec); });

  } catch (const std::exception &ex) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " " << ex.what();
    m_keep_processing = false;
  }
}

void session_t::on_write(error_code ec, size_t size) {
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " size: " << size;
    fail(ec, __FUNCTION__);
  }
}

void session_t::on_read(error_code ec, size_t size) {
  if (ec) {
    fail(ec, __FUNCTION__);
  }

  try {
    // !@# TODO - see whether we can eliminate this copy by connecting
    // simdjson's iterator directly to buffer sequence
    m_read_msg_str = boost::beast::buffers_to_string(m_read_buffer.data());
    if (m_read_msg_str.size() != size) {
      BOOST_LOG_TRIVIAL(error)
          << __FUNCTION__ << "read expected size: " << size
          << " but received size: " << m_read_msg_str.size();
      stop_processing();
      return;
    }
    if (size == 0) {
      // !@# TODO: count number of occurrances and escalate if they accumulate
      BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << ": received empty message";
      return;
    }
    //    m_read_buffer.consume(size);
    m_read_buffer.consume(m_read_msg_str.size());
    if (!m_handle_recv(std::string_view(m_read_msg_str))) {
      BOOST_LOG_TRIVIAL(error)
          << __FUNCTION__ << "handle_recv() returned false -- stop processing";
      stop_processing();
      return;
    }
  } catch (const std::exception &ex) {
    BOOST_LOG_TRIVIAL(error) << ex.what();
    BOOST_LOG_TRIVIAL(error) << "msg: " << m_read_msg_str;
    stop_processing();
    return;
  }

  if (m_keep_processing) {
    m_read_msg_str.clear();
    m_ws.async_read(m_read_buffer, [this](error_code ec, size_t size) {
      this->on_read(ec, size);
    });
  }
}

void session_t::on_close(error_code ec) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " entered";
  if (ec) {
    fail(ec, __FUNCTION__);
  }
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " succeeded";
}

} // namespace kdr
