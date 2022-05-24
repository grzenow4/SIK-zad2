#pragma once

#include <cstring>
#include <iostream>
#include <list>
#include <map>

#include "ServerParameters.h"

#define BUFF_SIZE 65535

class Message {

private:
    char _buffer[BUFF_SIZE];
    size_t _read_idx;
    size_t _buf_len;

public:
    Message() : _read_idx(0), _buf_len(0) {}

    const char* get_message() const {
        return _buffer;
    }

    char* get_message() {
        return _buffer;
    }

    size_t get_read_idx() const {
        return _read_idx;
    }

    size_t get_buf_len() const {
        _buf_len;
    }

    void set_read_idx(size_t val) {
        _read_idx = val;
    }

    void set_buf_len(size_t val) {
        _buf_len = val;
    }

private:
    template<typename T>
    void receive(T &num);

    void receive(std::string &str);

    void receive(std::pair<std::string, std::string> &player);

    void receive(std::pair<uint16_t, uint16_t> &position);

    template<typename T>
    void receive_list(std::list<T> &list);

//    template<typename K, typename V>
//    void receive_map(std::map<K, V> &map);

    void receive_hello(ServerParameters &server_params);

    void receive_accepted_player();

    void receive_game_started();

    void receive_turn();

    void receive_game_ended();
};
