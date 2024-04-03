#define BOOST_ASIO_DISABLE_BOOST_COROUTINE

#include "cert.hpp"
#include "session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <string>
#include <string_view>

int main(int argc, char **argv) {
  auto const host = "ws.kraken.com";
  auto const port = "443";

  boost::asio::io_context ioc;
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv13_client};
  krakpot::load_root_certificates(ctx);

  krakpot::processor_t processor;
  const auto handle_recv = [&processor](std::string_view msg) {
    try {
      processor.process(msg);
    } catch (const std::exception &ex) {
      BOOST_LOG_TRIVIAL(error) << ex.what();
      return false;
    }
    return true; // !@# TODO  - have processor return status...
  };

  auto session = krakpot::session_t(ioc, ctx);
  session.start_processing(handle_recv);
  ioc.run();

  return EXIT_SUCCESS;
}
