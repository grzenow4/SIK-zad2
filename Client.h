#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/system.hpp>

#include <coroutine>

#include "ClientParameters.h"
#include "GameTypes.h"
#include "ServerParameters.h"

#define BUFF_SIZE 65507

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Client {
private:
    boost::asio::io_context &_io_context;
    tcp::socket _tcp_socket;
    tcp::resolver::results_type _endpoint_server;
    udp::endpoint _endpoint_gui;
    udp::socket _udp_socket_send;
    udp::socket _udp_socket_rec;

    ClientParameters _client_params;
    ServerParameters _server_params;
    GameStatus _game_status;

    bool _in_lobby = true;
    bool _valid_msg = false;

    char buf[BUFF_SIZE];
    uint64_t read_idx = 0;
    uint64_t buf_length = 0;

    char buf_udp[BUFF_SIZE];
    uint64_t read_idx_udp = 0;
    uint64_t buf_len_udp = 0;

public:
    Client(boost::asio::io_context &io_context,
           const tcp::resolver::results_type &endpoints,
           udp::endpoint receiver_endpoint,
           ClientParameters client_params);

private:
    boost::asio::awaitable<void> server_listener();

    boost::asio::awaitable<void> gui_listener();

    void clear_buffer();

    void clear_buffer_udp();

    boost::asio::awaitable<void> read_message(size_t n);

    boost::asio::awaitable<void> read_message_udp();

    boost::asio::awaitable<void> receive(uint8_t &num);

    boost::asio::awaitable<void> receive(uint16_t &num);

    boost::asio::awaitable<void> receive(uint32_t &num);

    boost::asio::awaitable<void> receive(std::string &str);

    boost::asio::awaitable<void> receive(Player &player);

    boost::asio::awaitable<void> receive(Position &position);

    boost::asio::awaitable<void> receive(BombPlaced &event);

    boost::asio::awaitable<void> receive(BombExploded &event);

    boost::asio::awaitable<void> receive(PlayerMoved &event);

    boost::asio::awaitable<void> receive(BlockPlaced &event);

    boost::asio::awaitable<void> receive(std::list<std::shared_ptr<Event>> &list);

    boost::asio::awaitable<void> receive(std::list<player_id_t> &list);

    boost::asio::awaitable<void> receive(std::list<Position> &list);

    boost::asio::awaitable<void> receive(std::map<player_id_t, Player> &map);

    boost::asio::awaitable<void> receive(std::map<player_id_t, score_t> &map);

    boost::asio::awaitable<void> receive_hello();

    boost::asio::awaitable<void> receive_accepted_player();

    boost::asio::awaitable<void> receive_game_started();

    boost::asio::awaitable<void> receive_turn();

    boost::asio::awaitable<void> receive_game_ended();

    boost::asio::awaitable<void> receive_message();

    boost::asio::awaitable<void> receive_input_message();

    void send(uint8_t num);

    void send(const std::string &str);

    boost::asio::awaitable<void> send_join();

    boost::asio::awaitable<void> send_place_bomb();

    boost::asio::awaitable<void> send_place_block();

    boost::asio::awaitable<void> send_move(Direction direction);

    void send_gui(uint8_t num);

    void send_gui(uint16_t num);

    void send_gui(uint32_t num);

    void send_gui(const std::string &str);

    void send_gui(Player player);

    void send_gui(Position position);

    void send_gui(Bomb bomb);

    void send_gui(std::list<Position> list);

    void send_gui(std::set<Position> set);

    void send_gui(std::map<bomb_id_t, Bomb> map);

    void send_gui(std::map<player_id_t, Player> map);

    void send_gui(std::map<player_id_t, Position> map);

    void send_gui(std::map<player_id_t, score_t> map);

    boost::asio::awaitable<void> send_lobby();

    boost::asio::awaitable<void> send_game();
};
