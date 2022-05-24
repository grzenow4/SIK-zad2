#include "Client.h"

#include <iostream>

Client::Client(boost::asio::io_context& io_context,
               const tcp::resolver::results_type& endpoints,
               udp::endpoint receiver_endpoint,
               ClientParameters client_params) :
               _io_context(io_context),
               _tcp_socket(io_context),
               _udp_socket(io_context),
               _receiver_endpoint(receiver_endpoint),
               _client_params(client_params)
{
    do_connect(endpoints);
}

void Client::do_connect(const tcp::resolver::results_type& endpoints) {
    boost::asio::connect(_tcp_socket, endpoints);
    _udp_socket.open(udp::v4());
}

void Client::close() {
    boost::asio::post(_io_context, [this]() { _tcp_socket.close(); });
}

void Client::clear_buffer() {
    buf_length = 0;
    read_idx = 0;
}

void Client::read_message() {
    boost::system::error_code error;

    buf_length += _tcp_socket.read_some(boost::asio::buffer(buf), error);

    if (error == boost::asio::error::eof) {
        close(); // Connection closed cleanly by peer.
    }
    else if (error) {
        throw boost::system::system_error(error); // Some other error.
    }
}

void Client::receive(uint8_t &num) {
    uint64_t size = sizeof(uint8_t);
    if (read_idx + size > buf_length) {
        read_message();
    }
    memcpy(&num, &buf[read_idx], size);
    read_idx += size;
}

void Client::receive(uint16_t &num) {
    uint64_t size = sizeof(uint16_t);
    if (read_idx + size > buf_length) {
        read_message();
    }
    memcpy(&num, &buf[read_idx], size);
    read_idx += size;
    num = ntohs(num);
}

void Client::receive(uint32_t &num) {
    uint64_t size = sizeof(uint32_t);
    if (read_idx + size > buf_length) {
        read_message();
    }
    memcpy(&num, &buf[read_idx], size);
    read_idx += size;
    num = ntohl(num);
}

void Client::receive(std::string &str) {
    if (read_idx > buf_length || read_idx + (uint64_t) buf[read_idx] + 1 > buf_length) {
        read_message();
    }

    uint64_t size = (uint64_t) buf[read_idx];
    read_idx++;
    str = {buf + read_idx, size};
    read_idx += str.size();
}

void Client::receive(Player &player) {
    receive(player.name);
    receive(player.address);
}

void Client::receive(Position &position) {
    receive(position.x);
    receive(position.y);
}

void Client::receive(BombPlaced &event) {
    receive(event.bomb_id);
    receive(event.position);
}

void Client::receive(BombExploded &event) {
    receive(event.bomb_id);
    receive(event.robots_destroyed);
    receive(event.blocks_destroyed);
}

void Client::receive(PlayerMoved &event) {
    receive(event.player_id);
    receive(event.position);
}

void Client::receive(BlockPlaced &event) {
    receive(event.position);
}

void Client::receive(std::list<Event> &list) {
    uint32_t size;
    receive(size);

    for (uint32_t i = 0; i < size; i++) {
        if (read_idx == buf_length) {
            read_message();
        }

        BombPlaced bomb_placed;
        BombExploded bomb_exploded;
        PlayerMoved player_moved;
        BlockPlaced block_placed;

        switch ((int) buf[read_idx]) {
            case 0:
                receive(bomb_placed);
                list.push_back(bomb_placed);
                break;
            case 1:
                receive(bomb_exploded);
                list.push_back(bomb_exploded);
                break;
            case 2:
                receive(player_moved);
                list.push_back(player_moved);
                break;
            case 3:
                receive(block_placed);
                list.push_back(block_placed);
                break;
        }
    }
}

void Client::receive(std::list<player_id_t> &list) {
    uint32_t size;
    receive(size);

    for (uint32_t i = 0; i < size; i++) {
        player_id_t player_id;
        receive(player_id);
        list.push_back(player_id);
    }
}

void Client::receive(std::list<Position> &list) {
    uint32_t size;
    receive(size);

    for (uint32_t i = 0; i < size; i++) {
        Position position;
        receive(position);
        list.push_back(position);
    }
}

void Client::receive_map_start(std::map<player_id_t, Player> &map) {
    uint32_t size;
    receive(size);

    for (uint32_t i = 0; i < size; i++) {
        player_id_t key;
        Player value;
        receive(key);
        receive(value);
        map.insert({key, value});
    }
}

void Client::receive_map_end(std::map<player_id_t, score_t> &map) {
    uint32_t size;
    receive(size);

    for (uint32_t i = 0; i < size; i++) {
        player_id_t key;
        score_t value;
        receive(key);
        receive(value);
        map.insert({key, value});
    }
}

void Client::receive_message() {
    read_message();

    switch ((int) buf[read_idx]) {
        case 0: // Hello
            read_idx++;
            receive_hello();
            break;
        case 1: // AcceptedPlayer
            read_idx++;
            receive_accepted_player();
            break;
        case 2: // GameStarted
            read_idx++;
            receive_game_started();
            break;
        case 3: // Turn
            read_idx++;
            receive_turn();
            break;
        case 4: // GameEnded
            read_idx++;
            receive_game_ended();
            break;
    }

    clear_buffer();
}

void Client::receive_hello() {
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

    _server_params = ServerParameters(name,
                                      (int) players_count,
                                      size_x,
                                      size_y,
                                      game_length,
                                      explosion_radius,
                                      bomb_timer);

    _server_params.print();
}

void Client::receive_accepted_player() {
    player_id_t player_id;
    Player player;
    receive(player_id);
    receive(player);
    // TODO: coś dalej
}

void Client::receive_game_started() {
    std::map<player_id_t, Player> players;
    receive_map_start(players);
    // TODO: coś dalej
}

void Client::receive_turn() {
    uint16_t turn;
    std::list<Event> events;
    receive(turn);
    receive(events);
    // TODO: coś dalej
}

void Client::receive_game_ended() {
    std::map<player_id_t, score_t> scores;
    receive_map_end(scores);
    // TODO: coś dalej
}

void Client::send(uint8_t num) {
    uint64_t size = sizeof(uint8_t);
    memcpy(&buf[read_idx], &num, size);
    read_idx += size;
}

void Client::send(uint16_t num) {
    uint64_t size = sizeof(uint16_t);
    num = htons(num);
    memcpy(&buf[read_idx], &num, size);
    read_idx += size;
}

void Client::send(uint32_t num) {
    uint64_t size = sizeof(uint32_t);
    num = htonl(num);
    memcpy(&buf[read_idx], &num, size);
    read_idx += size;
}

void Client::send(const std::string& str) {
    uint8_t size = (uint8_t) str.size();
    send(size);
    memcpy(&buf[read_idx], str.c_str(), size);
    read_idx += str.size();
}

void Client::send_join() {
    uint8_t message_no = 0;
    send(message_no);
    send(_client_params.get_nickname());
}

void Client::send_place_bomb() {
    uint8_t message_no = 1;
    send(message_no);
}

void Client::send_place_block() {
    uint8_t message_no = 2;
    send(message_no);
}

void Client::send_move(Direction direction) {
    uint8_t message_no = 3;
    send(message_no);
    send((uint8_t) direction);
}

void Client::send_lobby() {
    // TODO
}

void Client::send_game() {
    // TODO
}
