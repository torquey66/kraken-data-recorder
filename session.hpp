#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/ssl.hpp>

#include <string>

namespace krakpot {

struct session_t final {

  using ioc_t = boost::asio::io_context;
  using ssl_context_t = boost::asio::ssl::context;
  using yield_context_t = boost::asio::yield_context;

  session_t(ioc_t &, ssl_context_t &);

  void start_heartbeat(std::string host, std::string port);

private:
  void run_heartbeat(std::string host, std::string port, yield_context_t);

  ioc_t &m_ioc;
  ssl_context_t &m_ssl_context;
};

} // namespace krakpot
