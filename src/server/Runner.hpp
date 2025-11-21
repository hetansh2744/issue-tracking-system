#ifndef RUNNER_HPP_
#define RUNNER_HPP_

#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

class Runner {
 private:
  std::shared_ptr<oatpp::network::tcp::server::ConnectionProvider>
      m_tcpConnectionProvider;

 public:
  Runner(const std::shared_ptr<
         oatpp::network::tcp::server::ConnectionProvider>& provider)
      : m_tcpConnectionProvider(provider) {}

  void run();
};

#endif
