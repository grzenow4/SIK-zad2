#pragma once

#include "GameSession.h"

using boost::asio::ip::tcp;

class Server {
private:
    tcp::acceptor _acceptor;

    GameRoom _room;

public:
    Server(boost::asio::io_context &io_context,
           const tcp::endpoint &endpoint,
           ServerParameters server_params);

private:
    void do_accept();
};
