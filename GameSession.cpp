#include "GameSession.h"

GameSession::GameSession(tcp::socket socket, GameRoom &room) :
            _socket(std::move(socket)), _room(room) {
    _socket.set_option(tcp::no_delay(true));
}

player_id_t GameSession::get_id() {
    return _id;
}

void GameSession::set_id(player_id_t id) {
    _id = id;
}

void GameSession::start() {
    _room.join(shared_from_this());
    read_message();
}

void GameSession::clear_buffer() {
    _read_idx = 0;
    _buf_len = 0;
}

void GameSession::read_message() {
    _socket.async_receive(
        boost::asio::buffer(_buffer + _buf_len, BUFF_SIZE),
        [this](boost::system::error_code ec, std::size_t len) {
            if (ec) {
                _room.disconnect(shared_from_this());
            } else {
                _buf_len += len;

                uint8_t msg_id;
                receive(msg_id);

                switch (msg_id) {
                    case 0:
                        receive_join();
                        break;
                    case 1:
                        receive_place_bomb();
                        break;
                    case 2:
                        receive_place_block();
                        break;
                    case 3:
                        receive_move();
                        break;
                    default:
                        break;
                }
            }
        }
    );
}

void GameSession::send_message() {
    boost::asio::async_write(_socket,
        boost::asio::buffer(_buffer, _buf_len),
        [this](boost::system::error_code ec, std::size_t) {
            if (ec) {
                exit(1);
            }
        });
}

void GameSession::receive(uint8_t &num) {
    size_t size = sizeof(uint8_t);
    memcpy(&num, _buffer + _read_idx, size);
    _read_idx += size;
}

void GameSession::receive(std::string &str) {
    uint64_t size = (uint64_t) _buffer[_read_idx];
    _read_idx++;
    str = {_buffer + _read_idx, size};
    _read_idx += size;
}

void GameSession::receive_join() {
    Player player;
    receive(player.name);
    player.address = _socket.remote_endpoint().address().to_string();
    _room.resolve_join(shared_from_this(), player);
}

void GameSession::receive_place_bomb() {
    _room._moves.insert({shared_from_this(), {1, -1}});
}

void GameSession::receive_place_block() {
    _room._moves.insert({shared_from_this(), {2, -1}});
}

void GameSession::receive_move() {
    uint8_t dir;
    receive(dir);
    _room._moves.insert({shared_from_this(), {3, dir}});
}

void GameSession::send(uint8_t num) {
    uint64_t size = sizeof(num);
    memcpy(_buffer + _buf_len, &num, size);
    _buf_len += size;
}

void GameSession::send(uint16_t num) {
    uint64_t size = sizeof(num);
    num = htons(num);
    memcpy(_buffer + _buf_len, &num, size);
    _buf_len += size;
}

void GameSession::send(uint32_t num) {
    uint64_t size = sizeof(num);
    num = htonl(num);
    memcpy(_buffer + _buf_len, &num, size);
    _buf_len += size;
}

void GameSession::send(const std::string &str) {
    uint8_t size = (uint8_t) str.size();
    send(size);
    memcpy(_buffer + _buf_len, str.c_str(), size);
    _buf_len += size;
}

void GameSession::send(Player player) {
    send(player.name);
    send(player.address);
}

void GameSession::send(Position position) {
    send(position.x);
    send(position.y);
}

void GameSession::send(BombPlaced event) {
    uint8_t event_no = 0;
    send(event_no);
    send(event.bomb_id);
    send(event.position);
}

void GameSession::send(BombExploded event) {
    uint8_t event_no = 1;
    send(event_no);
    send(event.bomb_id);
    send(event.robots_destroyed);
    send(event.blocks_destroyed);
}

void GameSession::send(PlayerMoved event) {
    uint8_t event_no = 2;
    send(event_no);
    send(event.player_id);
    send(event.position);
}

void GameSession::send(BlockPlaced event) {
    uint8_t event_no = 3;
    send(event_no);
    send(event.position);
}

void GameSession::send(std::list<Event> list) {
    uint32_t size = (uint32_t) list.size();
    send(size);
    for (auto el: list) {
        switch (el.get_type()) {
        case BombPlacedT:
            send(std::get<BombPlaced>(el.item));
            break;
        case BombExplodedT:
            send(std::get<BombExploded>(el.item));
            break;
        case PlayerMovedT:
            send(std::get<PlayerMoved>(el.item));
            break;
        case BlockPlacedT:
            send(std::get<BlockPlaced>(el.item));
            break;
        }
    }
}

void GameSession::send(std::list<player_id_t> list) {
    uint32_t size = (uint32_t) list.size();
    send(size);
    for (auto el: list) {
        send(el);
    }
}

void GameSession::send(std::list<Position> list) {
    uint32_t size = (uint32_t) list.size();
    send(size);
    for (auto el: list) {
        send(el);
    }
}

void GameSession::send(std::map<player_id_t, Player> map) {
    uint32_t size = (uint32_t) map.size();
    send(size);
    for (auto el: map) {
        send(el.first);
        send(el.second);
    }
}

void GameSession::send(std::map<player_id_t, score_t> map) {
    uint32_t size = (uint32_t) map.size();
    send(size);
    for (auto el: map) {
        send(el.first);
        send(el.second);
    }
}

void GameSession::send_hello(ServerParameters server_params) {
    clear_buffer();
    uint8_t message_no = 0;
    send(message_no);
    send(server_params.get_name());
    send(server_params.get_players_count());
    send(server_params.get_size_x());
    send(server_params.get_size_y());
    send(server_params.get_game_length());
    send(server_params.get_explosion_radius());
    send(server_params.get_bomb_timer());
    send_message();
    clear_buffer();
}

void GameSession::send_accepted_player(player_id_t player_id, Player player) {
    clear_buffer();
    uint8_t message_no = 1;
    send(message_no);
    send(player_id);
    send(player);
    send_message();
    clear_buffer();
}

void GameSession::send_game_started(std::map<player_id_t, Player> map) {
    clear_buffer();
    uint8_t message_no = 2;
    send(message_no);
    send(map);
    send_message();
    clear_buffer();
}

void GameSession::send_turn(uint16_t turn, std::list<Event> list) {
    clear_buffer();
    uint8_t message_no = 3;
    send(message_no);
    send(turn);
    send(list);
    send_message();
    clear_buffer();
}

void GameSession::send_game_ended(std::map<player_id_t, score_t> map) {
    clear_buffer();
    uint8_t message_no = 4;
    send(message_no);
    send(map);
    send_message();
    clear_buffer();
}
