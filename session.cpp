#include "session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

#include <chrono>

namespace {
void fail(boost::beast::error_code ec, char const *what) {
  BOOST_LOG_TRIVIAL(error) << what << ": " << ec.message();
}
} // namespace

namespace krakpot {

session_t::session_t(ioc_t &ioc, ssl_context_t &ssl_context)
    : m_ioc(ioc), m_ssl_context(ssl_context) {}

void session_t::start_heartbeat(std::string host, std::string port) {
  boost::asio::spawn(
      m_ioc,
      [this, host, port](yield_context_t yc) {
        this->run_heartbeat(host, port, yc);
      },
      [](std::exception_ptr ex) {
        BOOST_LOG_TRIVIAL(info) << "Mooooooooooooooooooooooo!";
        if (ex)
          std::rethrow_exception(ex);
      });
}

void session_t::run_heartbeat(std::string host, std::string port,
                              yield_context_t yield) {

  boost::beast::error_code ec;

  boost::asio::ip::tcp::resolver resolver(m_ioc);
  boost::beast::websocket::stream<
      boost::beast::ssl_stream<boost::beast::tcp_stream>>
      ws(m_ioc, m_ssl_context);
  auto const results = resolver.async_resolve(host, port, yield[ec]);
  if (ec)
    return fail(ec, "resolve");
  boost::beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));
  auto ep =
      boost::beast::get_lowest_layer(ws).async_connect(results, yield[ec]);
  if (ec)
    return fail(ec, "connect");
  if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(),
                                host.c_str())) {
    ec = boost::beast::error_code(static_cast<int>(::ERR_get_error()),
                                  boost::asio::error::get_ssl_category());
    return fail(ec, "connect");
  }
  host += ':' + std::to_string(ep.port());
  boost::beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));
  ws.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::request_type &req) {
        req.set(boost::beast::http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
      }));
  ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client,
                                  yield[ec]);
  if (ec)
    return fail(ec, "ssl_handshake");
  boost::beast::get_lowest_layer(ws).expires_never();
  ws.set_option(boost::beast::websocket::stream_base::timeout::suggested(
      boost::beast::role_type::client));
  ws.async_handshake(host, "/", yield[ec]);
  if (ec)
    return fail(ec, "handshake");

  // TODO: https://stackoverflow.com/a/30560228
  // use timer scheme described for ping/pong loop
  boost::beast::flat_buffer buffer;
  int id = 0;
  while (id < 42) {
    auto ping = nlohmann::json{};
    ping["event"] = "ping";
    ping["reqid"] = ++id;

    ws.async_write(boost::asio::buffer(ping.dump()), yield[ec]);
    if (ec)
      return fail(ec, "write");

    buffer.clear();
    ws.async_read(buffer, yield[ec]);

    if (ec)
      return fail(ec, "read");
    BOOST_LOG_TRIVIAL(info) << boost::beast::make_printable(buffer.data());
  }

  ws.async_close(boost::beast::websocket::close_code::normal, yield[ec]);
  if (ec)
    return fail(ec, "close");
}

} // namespace krakpot
