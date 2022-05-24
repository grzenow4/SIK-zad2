#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <coroutine>
#include <list>
#include <map>

#include "ClientParameters.h"
#include "GameTypes.h"
#include "ServerParameters.h"

#define BUFF_SIZE 65535

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Client {
private:
    boost::asio::io_context& _io_context;
    tcp::socket _tcp_socket;
    udp::socket _udp_socket;
    udp::endpoint _receiver_endpoint;
    ClientParameters _client_params;
    ServerParameters _server_params;
    char buf[BUFF_SIZE];
    uint64_t read_idx = 0;
    uint64_t buf_length = 0;

public:
    Client(boost::asio::io_context& io_context,
           const tcp::resolver::results_type& endpoints,
           udp::endpoint receiver_endpoint,
           ClientParameters client_params);

    void receive_message();

    void close();

private:
    void do_connect(const tcp::resolver::results_type& endpoints);

    void clear_buffer();

    void read_message();

    void receive(uint8_t &num);

    void receive(uint16_t &num);

    void receive(uint32_t &num);

    void receive(std::string &str);

    void receive(Player &player);

    void receive(Position &position);

    void receive(BombPlaced &event);

    void receive(BombExploded &event);

    void receive(PlayerMoved &event);

    void receive(BlockPlaced &event);

    void receive(std::list<Event> &list);

    void receive(std::list<player_id_t> &list);

    void receive(std::list<Position> &list);

    void receive_map_start(std::map<player_id_t, Player> &map);

    void receive_map_end(std::map<player_id_t, score_t> &map);

    void receive_hello();

    void receive_accepted_player();

    void receive_game_started();

    void receive_turn();

    void receive_game_ended();

    void send(uint8_t num);

    void send(uint16_t num);

    void send(uint32_t num);

    void send(const std::string& str);

    void send_join();

    void send_place_bomb();

    void send_place_block();

    void send_move(Direction direction);

    void send_lobby();

    void send_game();
};
