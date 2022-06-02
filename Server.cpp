#include "Server.h"

Server::Server(boost::asio::io_context &io_context,
               const tcp::resolver::results_type &endpoint,
               ServerParameters server_params) :
        _io_context(io_context),
        _tcp_socket(io_context),
        _endpoint(endpoint),
        _server_params(server_params) {
    boost::asio::connect(_tcp_socket, _endpoint);
    _tcp_socket.set_option(tcp::no_delay(true));

    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    co_spawn(io_context, server_listener(), boost::asio::detached);

    _io_context.run();
}

boost::asio::awaitable<void> Server::server_listener() {
    co_return;
}
