#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/system.hpp>

#include <coroutine>

#include "GameTypes.h"
#include "ServerParameters.h"

using boost::asio::ip::tcp;

class Server {
private:
    boost::asio::io_context &_io_context;
    tcp::socket _tcp_socket;
    tcp::resolver::results_type _endpoint;

    ServerParameters _server_params;

public:
    Server(boost::asio::io_context &io_context,
           const tcp::resolver::results_type &endpoint,
           ServerParameters server_params);

private:
    boost::asio::awaitable<void> server_listener();
};
