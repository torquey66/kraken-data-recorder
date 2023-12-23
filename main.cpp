#include "cert.hpp"
#include "session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <string>

int main(int argc, char **argv) {
  auto const host = "ws.kraken.com";
  auto const port = "443";

  boost::asio::io_context ioc;
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
  krakpot::load_root_certificates(ctx);

  auto session = krakpot::session_t(ioc, ctx);
  session.start_heartbeat(host, port);
  ioc.run();

  return EXIT_SUCCESS;
}
