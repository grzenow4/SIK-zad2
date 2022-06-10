#include "Server.h"

Server::Server(boost::asio::io_context &io_context,
               const tcp::endpoint &endpoint,
               ServerParameters server_params) :
        _acceptor(io_context, endpoint),
        _room(io_context, server_params) {
    do_accept();
    io_context.run();
}

void Server::do_accept() {
    _acceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<GameSession>(std::move(socket), _room)->start();
                }
                do_accept();
            }
    );
}
