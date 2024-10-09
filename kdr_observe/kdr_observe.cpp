#include "shmem_sink.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>

#include <csignal>
#include <iostream>
#include <string>

namespace bip = boost::interprocess;

void signal_handler(int signal) {
  if (signal == SIGSEGV) {
    std::cerr << "Segmentation fault caught!" << std::endl;
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  std::signal(SIGSEGV, signal_handler);

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <pair name>" << std::endl;
    return 1;
  }

  const std::string pair{argv[1]};
  const std::string suffix{kdr::shmem::normalized_symbol(pair)};
  const std::string segment_name{kdr::shmem::segment_name(suffix)};
  const std::string content_name{kdr::shmem::content_name(suffix)};
  const std::string mutex_name{kdr::shmem::mutex_name(suffix)};

  try {
    bip::managed_shared_memory segment{bip::open_only, segment_name.c_str()};

    kdr::shmem::book_content_t content;
    bip::named_mutex mutex{bip::open_only, mutex_name.c_str()};
    {
      bip::scoped_lock<bip::named_mutex> lock(mutex);
      std::pair<kdr::shmem::book_content_t *,
                bip::managed_shared_memory::size_type>
          result{
              segment.find<kdr::shmem::book_content_t>(content_name.c_str())};
      kdr::shmem::book_content_t *content_ptr =
          reinterpret_cast<kdr::shmem::book_content_t *>(result.first);
      content = *content_ptr;
    }

    std::cout << "num_asks: " << content.num_asks() << std::endl;
    for (size_t idx = 0; idx < content.num_asks(); ++idx) {
      const auto &quote = content.ask(idx);
      std::cout << quote.second.str() << " @ " << quote.first.str()
                << std::endl;
    }
    std::cout << std::endl;
    std::cout << "num_bids: " << content.num_bids() << std::endl;
    for (size_t idx = 0; idx < content.num_bids(); ++idx) {
      const auto &quote = content.bid(idx);
      std::cout << quote.second.str() << " @ " << quote.first.str()
                << std::endl;
    }
  } catch (const std::exception &ex) {
    std::cerr << "ex: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
