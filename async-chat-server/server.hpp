#pragma once

#include <vector>

#include "semantics.hpp"

struct server_t {
public:
  handle_t epollfd;
  handle_t serverfd;

  std::vector<handle_t> clients;
};
