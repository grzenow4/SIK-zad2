#pragma once

#include "GameRoom.h"

#define BUFF_SIZE 65507

using boost::asio::ip::tcp;

class GameSession : public std::enable_shared_from_this<GameSession> {
private:
    tcp::socket _socket;
    GameRoom &_room;

    player_id_t _id = -1;

    char _buffer[BUFF_SIZE];
    size_t _read_idx = 0;
    size_t _buf_len = 0;

public:
    GameSession(tcp::socket socket, GameRoom &room);

    player_id_t get_id();

    void set_id(player_id_t id);

    void start();

    void send_hello(ServerParameters server_params);

    void send_accepted_player(player_id_t player_id, Player player);

    void send_game_started(std::map<player_id_t, Player> map);

    void send_turn(uint16_t turn, std::list<Event> list);

    void send_game_ended(std::map<player_id_t, score_t> map);

private:
    void clear_buffer();

    void read_message();

    void send_message();

    void receive(uint8_t &num);

    void receive(std::string &str);

    void receive_join();

    void receive_place_bomb();

    void receive_place_block();

    void receive_move();

    void receive_message();

    void send(uint8_t num);

    void send(uint16_t num);

    void send(uint32_t num);

    void send(const std::string &str);

    void send(Player player);

    void send(Position position);

    void send(BombPlaced event);

    void send(BombExploded event);

    void send(PlayerMoved event);

    void send(BlockPlaced event);

    void send(std::list<Event> list);

    void send(std::list<player_id_t> list);

    void send(std::list<Position> list);

    void send(std::map<player_id_t, Player> map);

    void send(std::map<player_id_t, score_t> map);
};
