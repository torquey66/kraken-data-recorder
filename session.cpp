#include "session.hpp"

#include "constants.hpp"
#include "requests.hpp"

#include <boost/log/trivial.hpp>

#include <chrono>
#include <format>

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

session_t::session_t(ioc_t &ioc, ssl_context_t &ssl_context)
    : m_ioc{ioc}, m_ssl_context{ssl_context}, m_ws{ioc, ssl_context} {
  asio::spawn(
      m_ioc,
      [this](yield_context_t yield) {
        handshake(c_kraken_host, c_kraken_port, yield);
        ping(yield);
      },
      [](std::exception_ptr ex) {
        if (ex)
          std::rethrow_exception(ex);
      });
}

void session_t::handshake(std::string host, std::string port,
                          yield_context_t yield) {
  bst::error_code ec;

  asio::ip::tcp::resolver resolver(m_ioc);
  auto const results = resolver.async_resolve(host, port, yield[ec]);
  if (ec) {
    return fail(ec, "resolve");
  }

  bst::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
  auto endpoint = bst::get_lowest_layer(m_ws).async_connect(results, yield[ec]);
  if (ec) {
    return fail(ec, "connect");
  }

  if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(),
                                host.c_str())) {
    ec = bst::error_code(static_cast<int>(::ERR_get_error()),
                         asio::error::get_ssl_category());
    return fail(ec, "connect");
  }

  host += ':' + std::to_string(endpoint.port());
  bst::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
  m_ws.set_option(ws::stream_base::decorator([](ws::request_type &req) {
    req.set(bst::http::field::user_agent,
            std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-coro");
  }));
  m_ws.next_layer().async_handshake(asio::ssl::stream_base::client, yield[ec]);
  if (ec) {
    return fail(ec, "ssl_handshake");
  }

  bst::get_lowest_layer(m_ws).expires_never();
  m_ws.set_option(ws::stream_base::timeout::suggested(bst::role_type::client));
  m_ws.async_handshake(host, "/v2", yield[ec]);
  if (ec) {
    return fail(ec, "handshake");
  }
}

void session_t::ping(yield_context_t yield) {

  bst::error_code ec;
  bst::flat_buffer buffer;
  asio::deadline_timer timer(m_ioc);

  while (m_keep_processing) {
    try {
      const auto ping = request::ping_t{++m_req_id};
      send(ping.str(), yield);
      timer.expires_from_now(boost::posix_time::seconds(c_ping_interval_secs));
      timer.async_wait(yield);
    } catch (const std::exception &ex) {
      BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " " << ex.what();
      m_keep_processing = false;
    }
  }

  m_ws.async_close(ws::close_code::normal, yield[ec]);
  if (ec)
    return fail(ec, "close");
}

void session_t::start_processing(const recv_cb_t &handle_recv) {
  asio::spawn(
      m_ioc,
      [this, handle_recv](yield_context_t yield) {
        process(handle_recv, yield);
      },
      [](std::exception_ptr ex) {
        if (ex)
          std::rethrow_exception(ex);
      });
}

void session_t::send(msg_t msg, yield_context_t yield) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << ": " << msg;
  bst::error_code ec;
  m_ws.async_write(asio::buffer(std::string(msg.data(), msg.size())),
                   yield[ec]);
  if (ec)
    return fail(ec, "write");
}

void session_t::process(const recv_cb_t &handle_recv, yield_context_t yield) {
  bst::error_code ec;
  bst::flat_buffer buffer;
  asio::deadline_timer timer(m_ioc);
  timer.expires_from_now(boost::posix_time::seconds(3));
  timer.async_wait(yield);

  while (m_keep_processing) {
    buffer.clear();
    m_ws.async_read(buffer, yield[ec]);
    if (ec) {
      fail(ec, "read");
    } else {
      auto msg_str = bst::buffers_to_string(buffer.data());
      try {
        m_keep_processing = handle_recv(std::string_view(msg_str), yield);
      } catch (const std::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << ex.what();
        m_keep_processing = false;
      }
    }
  }
}

} // namespace krakpot
