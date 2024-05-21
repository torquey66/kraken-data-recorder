#pragma once

#include "responses.hpp"

#include <memory>
#include <vector>

namespace krakpot {

struct sink_t {
  virtual ~sink_t() {}

  virtual void accept(const response::book_t &) = 0;
  virtual void accept(const response::trades_t &) = 0;
};

struct multisink_t : public sink_t {

  using sink_ptr_t = std::unique_ptr<sink_t>;
  using sink_vector_t = std::vector<sink_ptr_t>;

  multisink_t(sink_vector_t &sinks) {
    for (auto &sink : sinks) {
      m_sinks.emplace_back(std::move(sink));
    }
  }

  void accept(const response::book_t &response) override {
    for (auto &sink : m_sinks) {
      sink->accept(response);
    }
  }

  void accept(const response::trades_t &response) override {
    for (auto &sink : m_sinks) {
      sink->accept(response);
    }
  }

private:
  sink_vector_t m_sinks;
};

} // namespace krakpot
