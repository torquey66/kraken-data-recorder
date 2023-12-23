#pragma once

#include <boost/asio/ssl.hpp>

namespace krakpot {

    void load_root_certificates(boost::asio::ssl::context & ctx);

}