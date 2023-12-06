#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "semantics.hpp"
#include "server.hpp"

#define PROTO_AUTO 0

#define DIE_IF(condition, ...) if (condition) { \
  printf(__VA_ARGS__); \
  exit(-1); \
}

using std::vector;
using std::printf;

constexpr int maxEvents = 10;
constexpr unsigned short desiredPort = 42069;

static server_t server {
  .clients = vector<handle_t>()
};

void accept_loop () {
  handle_t connfd = accept4(
    server.serverfd,
    nullptr,
    nullptr,
    SOCK_NONBLOCK
  );

  DIE_IF(connfd < 0, "Couldn't accept connection\n");

  // Add the client to the event loop
  epoll_event event {};
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = connfd;
  epoll_ctl(server.epollfd, EPOLL_CTL_ADD, connfd, &event);

  server.clients.push_back(connfd);
  printf("[i] client %d connected\n", connfd);
}

void client_disconnected (handle_t connfd) {
  // Remove connection from active client list
  // and close the socket upon disconnection

  auto &clients = server.clients;
  for (auto it = clients.begin(); it != clients.end(); it ++) {
    if (*it == connfd) {
      printf("[i] client %d just disconnected\n", *it);
      clients.erase(it);
      break;
    }
  }

  // Remove disconnected client from event loop
  epoll_ctl(server.epollfd, EPOLL_CTL_DEL, connfd, nullptr);
  close(connfd);
}

void read_loop (handle_t connfd) {
  char buffer[1024];
  int recvCount = recv(connfd, buffer, 1024, 0);

  DIE_IF(recvCount < 0, "recv() failed\n");

  // End of file canary, socket disconnected
  if (recvCount == 0) {
    client_disconnected(connfd);
    return;
  }

  printf(
    "[i] received message from client %d: %.*s",
    connfd,
    recvCount,
    buffer
  );

  for (auto &client : server.clients) {
    if (client == connfd) {
      // The message might otherwise appear twice on
      // the client because this isn't really the
      // most sophisticated program out there
      continue;
    }

    send(client, buffer, recvCount, MSG_DONTWAIT);
  }
}

void network_loop () {
  epoll_event receiver[maxEvents] {};
  int eventCount = epoll_wait(
    server.epollfd,
    receiver,
    maxEvents,
    -1
  );

  DIE_IF(eventCount < 0, "epoll_wait() failed\n");

  for (int i = 0; i < eventCount; i ++) {
    auto event = receiver[i];

    // @todo: separate server and clients
    // to different epoll instances?
    if (event.data.fd == server.serverfd) {
      accept_loop();
      continue;
    }

    read_loop(event.data.fd);
  }
}

int main () {
  printf("[i] hi!\n");

  server.epollfd = epoll_create1(0);
  DIE_IF(server.epollfd < 0, "Couldn't create epoll instance\n");

  server.serverfd = socket(
    AF_INET,
    SOCK_STREAM | SOCK_NONBLOCK,
    PROTO_AUTO
  );

  DIE_IF(server.serverfd < 0, "Couldn't create a socket\n");

  {
    sockaddr_in address {
      .sin_family = AF_INET,
      .sin_port = htons(desiredPort),
      .sin_addr = {
        .s_addr = INADDR_ANY
      }
    };

    auto pAddress = (sockaddr *) &address;
    auto addr_size = sizeof(address);
    int ret = bind(server.serverfd, pAddress, addr_size);
    DIE_IF(ret != 0, "Couldn't bind socket\n");
  }

  DIE_IF(listen(server.serverfd, 3) != 0, "Couldn't listen\n");

  // Add the server to the event loop
  // to poll for new connection requests
  epoll_event event {};
  event.events = EPOLLIN;
  event.data.fd = server.serverfd;
  epoll_ctl(
    server.epollfd,
    EPOLL_CTL_ADD,
    server.serverfd,
    &event
  );

  printf("[i] now ready for clients!\n");
  forever {
    network_loop();
  }
}
