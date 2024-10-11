#include "shmem_sink.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>

#include <csignal>
#include <iostream>
#include <memory>
#include <string>

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

int main(int argc, char *argv[]) {
  std::signal(SIGSEGV, signal_handler);

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <pair name>" << std::endl;
    return 1;
  }

  const std::string pair{argv[1]};

  using kdr::shmem::shmem_names_t;
  const kdr::shmem::shmem_names_t names{
      pair, std::string{shmem_names_t::c_book_kind}};

  try {
    bip::managed_shared_memory segment{bip::open_only, names.segment().c_str()};
    kdr::shmem::book_content_t content;
    mutex_ptr = std::make_unique<bip::named_mutex>(bip::open_only,
                                                   names.mutex().c_str());
    std::pair<kdr::shmem::book_content_t *,
              bip::managed_shared_memory::size_type>
        result{
            segment.find<kdr::shmem::book_content_t>(names.content().c_str())};
    kdr::shmem::book_content_t *content_ptr =
        reinterpret_cast<kdr::shmem::book_content_t *>(result.first);

    {
      bip::scoped_lock<bip::named_mutex> lock(*mutex_ptr);
      content = *content_ptr;
    }
    std::cout << content.str();

  } catch (const std::exception &ex) {
    std::cerr << "ex: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
