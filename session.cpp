#include "session.hpp"

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
    : m_ioc{ioc}, m_ssl_context{ssl_context}, m_ws{ioc, ssl_context} {

  boost::asio::spawn(
      m_ioc,
      [this](yield_context_t yc) {
        handshake("ws.kraken.com", "443", yc);
        heartbeat(yc);
      },
      [](std::exception_ptr ex) {
        if (ex)
          std::rethrow_exception(ex);
      });
}

void session_t::handshake(std::string host, std::string port,
                          yield_context_t yield) {

  boost::beast::error_code ec;

  boost::asio::ip::tcp::resolver resolver(m_ioc);
  auto const results = resolver.async_resolve(host, port, yield[ec]);
  if (ec)
    return fail(ec, "resolve");
  boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
  auto ep =
      boost::beast::get_lowest_layer(m_ws).async_connect(results, yield[ec]);
  if (ec)
    return fail(ec, "connect");
  if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(),
                                host.c_str())) {
    ec = boost::beast::error_code(static_cast<int>(::ERR_get_error()),
                                  boost::asio::error::get_ssl_category());
    return fail(ec, "connect");
  }
  host += ':' + std::to_string(ep.port());
  boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
  m_ws.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::request_type &req) {
        req.set(boost::beast::http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
      }));
  m_ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client,
                                    yield[ec]);
  if (ec)
    return fail(ec, "ssl_handshake");
  boost::beast::get_lowest_layer(m_ws).expires_never();
  m_ws.set_option(boost::beast::websocket::stream_base::timeout::suggested(
      boost::beast::role_type::client));
  m_ws.async_handshake(host, "/", yield[ec]);
  if (ec)
    return fail(ec, "handshake");
}

void session_t::heartbeat(yield_context_t yield) {

  boost::beast::error_code ec;
  boost::beast::flat_buffer buffer;
  boost::asio::deadline_timer timer(m_ioc);
  size_t id = 0;
  while (true) {
    timer.expires_from_now(boost::posix_time::seconds(3));
    timer.async_wait(yield);

    auto ping = nlohmann::json{};
    ping["event"] = "ping";
    ping["reqid"] = ++id;

    m_ws.async_write(boost::asio::buffer(ping.dump()), yield[ec]);
    if (ec)
      return fail(ec, "write");

    buffer.clear();
    m_ws.async_read(buffer, yield[ec]);

    if (ec)
      return fail(ec, "read");
    BOOST_LOG_TRIVIAL(info) << boost::beast::make_printable(buffer.data());
  }

  /*
  m_ws.async_close(boost::beast::websocket::close_code::normal, yield[ec]);
  if (ec)
    return fail(ec, "close");
  */
}

} // namespace krakpot
