#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/system.hpp>

#include <coroutine>
#include <cstring>
#include <list>
#include <map>

#include "GameTypes.h"
#include "ServerParameters.h"

#define BUFF_SIZE 65507

class Message {
private:
    char _buffer_tcp[BUFF_SIZE];
    size_t _read_idx_tcp;
    size_t _buf_len_tcp;
    char _buffer_udp[BUFF_SIZE];
    size_t _read_idx_udp;
    size_t _buf_len_udp;

public:
    Message() : _read_idx_tcp(0), _buf_len_tcp(0),
                _read_idx_udp(0), _buf_len_udp(0) {}

    char* get_buf_tcp() {
        return _buffer_tcp;
    }

    size_t get_read_idx_tcp() {
        return _read_idx_tcp;
    }

    size_t get_buf_len_tcp() {
        return _buf_len_tcp;
    }

    char* get_buf_udp() {
        return _buffer_udp;
    }

    size_t get_read_idx_udp() {
        return _read_idx_udp;
    }

    size_t get_buf_len_udp() {
        return _buf_len_udp;
    }

    void set_buf_len_tcp(size_t len) {
        _buf_len_tcp = len;
    }

private:
    void clear_buffer_tcp();

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
};
