#ifndef RUNNER_HPP_
#define RUNNER_HPP_

#include <memory>
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

class Runner {
 private:
  std::shared_ptr<oatpp::network::tcp::server::ConnectionProvider>
      m_tcpConnectionProvider;

 public:
  // Proper constructor for Runner, marked explicit for safety
  explicit Runner(const std::shared_ptr<
      oatpp::network::tcp::server::ConnectionProvider>& provider)
      : m_tcpConnectionProvider(provider) {}

  void run();
};

#endif  // RUNNER_HPP_
