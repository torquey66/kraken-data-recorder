#include "shmem_sink.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>

#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace bip = boost::interprocess;

std::unique_ptr<bip::named_mutex> mutex_ptr;

void signal_handler(int signal) {
  if (signal == SIGSEGV) {
    std::cerr << "Segmentation fault caught!" << std::endl;
    if (mutex_ptr) {
      mutex_ptr->unlock();
    }
    exit(-1);
  }
}

template <typename T>
boost::json::object observe(std::string pair, std::string kind) {

  const kdr::shmem::shmem_names_t names{pair, kind};

  bip::managed_shared_memory segment{bip::open_only, names.segment().c_str()};
  T content;
  mutex_ptr =
      std::make_unique<bip::named_mutex>(bip::open_only, names.mutex().c_str());
  std::pair<T *, bip::managed_shared_memory::size_type> result{
      segment.find<T>(names.content().c_str())};
  T *content_ptr = reinterpret_cast<T *>(result.first);

  {
    bip::scoped_lock<bip::named_mutex> lock(*mutex_ptr);
    content = *content_ptr;
  }
  return content.to_json_obj();
}

int main(int argc, char *argv[]) {
  std::signal(SIGSEGV, signal_handler);

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <pair name>" << std::endl;
    return 1;
  }

  const std::string pair{argv[1]};

  try {
    const auto book = observe<kdr::shmem::book_content_t>(
        pair, std::string{kdr::shmem::shmem_names_t::c_book_kind});
    const auto trades = observe<kdr::shmem::trade_content_t>(
        pair, std::string{kdr::shmem::shmem_names_t::c_trade_kind});

    boost::json::object result = {{"book", book}, {"trades", trades}};
    std::cout << boost::json::serialize(result);

  } catch (const std::exception &ex) {
    std::cerr << "ex: " << ex.what() << std::endl;
    return -11;
  }
  return 0;
}
