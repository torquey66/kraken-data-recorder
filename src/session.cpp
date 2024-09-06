#include "session.hpp"

#include "constants.hpp"
#include "requests.hpp"

#include <boost/log/trivial.hpp>

#include <chrono>
#include <format>
#include <memory>

namespace asio = boost::asio;
namespace bst = boost::beast;
namespace ws = bst::websocket;

namespace kdr {

session_t::session_t(ssl_context_t &ssl_context, const config_t &config)
    : m_resolver{m_ioc}, m_ws{m_ioc, ssl_context}, m_config{config} {}

void session_t::fail(bst::error_code ec, char const *what) {
  BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << what << ": " << ec.message();
  m_keep_processing = false;
}

void session_t::start_processing(const connected_cb_t &handle_connected,
                                 const recv_cb_t &handle_recv) {
  m_handle_connected = handle_connected;
  m_handle_recv = handle_recv;
  m_keep_processing = true;
  m_resolver.async_resolve(m_config.kraken_host(), m_config.kraken_port(),
                           [this](error_code ec, resolver::results_type rt) {
                             if (!ec) {
                               BOOST_LOG_TRIVIAL(info) << "resolve suceeded";
                             }
                             this->on_resolve(ec, rt);
                           });
}

void session_t::send(msg_t msg) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << ": " << msg;
  error_code ec;
  const auto num_bytes_written =
      m_ws.write(asio::buffer(msg.data(), msg.size()), ec);
  if (ec) {
    fail(ec, "session_t::send async_write failed");
  }
  if (num_bytes_written != msg.size()) {
    const std::string message = std::string(__FUNCTION__) + " short write: " +
                                "expected: " + std::to_string(msg.size()) +
                                " actual: " + std::to_string(num_bytes_written);
    fail(ec, message.c_str());
  }
}

void session_t::on_resolve(error_code ec, resolver::results_type results) {
  if (ec) {
    fail(ec, __FUNCTION__);
  }
  boost::beast::get_lowest_layer(m_ws).expires_after(
      std::chrono::seconds(30)); // TODO: constants
  boost::beast::get_lowest_layer(m_ws).async_connect(
      results,
      [this](error_code ec, resolver::results_type::endpoint_type endpoint) {
        if (!ec) {
          BOOST_LOG_TRIVIAL(info) << "connect succeeded";
        }
        this->on_connect(ec, endpoint);
      });
}

void session_t::on_connect(error_code ec,
                           resolver::results_type::endpoint_type endpoint) {
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
      asio::ssl::stream_base::client, [this](error_code ec) {
        if (!ec) {
          BOOST_LOG_TRIVIAL(info) << "ssl handshake succeeded";
        }
        this->on_ssl_handshake(ec);
      });
}

void session_t::on_ssl_handshake(error_code ec) {
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

  m_ws.async_handshake(m_config.kraken_host(), "/v2", [this](error_code ec) {
    if (!ec) {
      BOOST_LOG_TRIVIAL(info) << "handshake succeeded";
    }
    this->on_handshake(ec);
  });
}

void session_t::on_handshake(error_code ec) {
  if (ec) {
    fail(ec, __FUNCTION__);
  }

  m_keep_processing = true;

  m_handle_connected();

  m_read_buffer.clear();
  m_ws.async_read(m_read_buffer, [this](error_code ec, size_t size) {
    this->on_read(ec, size);
  });
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

  // !@# TODO - see whether we can eliminate this copy by connecting
  // simdjson's iterator directly to buffer sequence
  m_read_msg_str = boost::beast::buffers_to_string(m_read_buffer.data());
  if (m_read_msg_str.size() != size) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << "read expected size: " << size
                             << " but received size: " << m_read_msg_str.size();
    stop_processing();
    return;
  }

  if (size == 0) {
    BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << ": received empty message";
    return;
  }
  m_read_buffer.consume(m_read_msg_str.size());

  if (!m_handle_recv(std::string_view(m_read_msg_str))) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << "handle_recv() returned false -- stop processing";
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
