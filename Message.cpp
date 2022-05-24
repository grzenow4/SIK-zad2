#include "Message.h"

template<typename T>
void Message::receive(T &num) {
    uint64_t size = sizeof(T);
//    if (_read_idx + size > _buf_len) {
//        read_message();
//    }
    memcpy(&num, &_buffer[_read_idx], size);
    _read_idx += size;
}

void Message::receive(std::string &str) {
//    if (_read_idx > _buf_len || _read_idx + (uint64_t) _buffer[_read_idx] + 1 > _buf_len) {
//        read_message();
//    }
    uint64_t size = (uint64_t) _buffer[_read_idx];
    _read_idx++;
    str = {_buffer + _read_idx, size};
    _read_idx += str.size();
}

void Message::receive(std::pair<std::string, std::string> &player) {
    receive(player.first);
    receive(player.second);
}

void Message::receive(std::pair<uint16_t, uint16_t> &position) {
    receive(position.first);
    receive(position.second);
}

template<typename T>
void Message::receive_list(std::list<T> &list) {
    for (T el : list)
        receive(el);
}

//template<typename K, typename V>
//void Message::receive_map(std::map<K, V> &map) {
//
//}

void Message::receive_hello(ServerParameters &server_params) {
    std::string name;
    uint8_t players_count;
    uint16_t size_x, size_y, game_length, explosion_radius, bomb_timer;

    receive(name);
    receive(players_count);
    receive(size_x);
    receive(size_y);
    receive(game_length);
    receive(explosion_radius);
    receive(bomb_timer);

    server_params = ServerParameters(name,
                                      (int) players_count,
                                      htobe16(size_x),
                                      htobe16(size_y),
                                      htobe16(game_length),
                                      htobe16(explosion_radius),
                                      htobe16(bomb_timer));

    server_params.print();
}

void Message::receive_accepted_player() {
}

void Message::receive_game_started() {
}

void Message::receive_turn() {
}

void Message::receive_game_ended() {
}
