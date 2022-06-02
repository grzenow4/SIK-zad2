#include "Message.h"

void Message::clear_buffer_tcp() {
    _buf_len_tcp = 0;
    _read_idx_tcp = 0;
}

void Message::clear_buffer_udp() {
    _buf_len_udp = 0;
    _read_idx_udp = 0;
}

boost::asio::awaitable<void> Message::read_message(size_t n) {
    size_t already_read = 0;
    while (already_read < n) {
        already_read += co_await tcp_socket.async_read_some(
                boost::asio::buffer(_buffer_tcp + _buf_len_tcp + already_read,
                                    n - already_read),
                boost::asio::use_awaitable);
    }
    _buf_len_tcp += n;
    co_return;
}

boost::asio::awaitable<void> Message::read_message_udp() {
    _buf_len_udp = co_await udp_socket.async_receive(
            boost::asio::buffer(_buffer_udp),
            boost::asio::use_awaitable);
    co_return;
}

boost::asio::awaitable<void> Message::receive(uint8_t &num) {
    uint64_t size = sizeof(uint8_t);
    co_await read_message(size);
    memcpy(&num, _buffer_tcp + _read_idx_tcp, size);
    _read_idx_tcp += size;
    co_return;
}

boost::asio::awaitable<void> Message::receive(uint16_t &num) {
    uint64_t size = sizeof(uint16_t);
    co_await read_message(size);
    memcpy(&num, _buffer_tcp + _read_idx_tcp, size);
    _read_idx_tcp += size;
    num = ntohs(num);
    co_return;
}

boost::asio::awaitable<void> Message::receive(uint32_t &num) {
    uint64_t size = sizeof(uint32_t);
    co_await read_message(size);
    memcpy(&num, _buffer_tcp + _read_idx_tcp, size);
    _read_idx_tcp += size;
    num = ntohl(num);
    co_return;
}

boost::asio::awaitable<void> Message::receive(std::string &str) {
    co_await read_message(1);
    uint64_t size = (uint64_t) _buffer_tcp[_read_idx_tcp];
    _read_idx_tcp++;
    co_await read_message(size);
    str = {_buffer_tcp + _read_idx_tcp, size};
    _read_idx_tcp += size;
    co_return;
}

boost::asio::awaitable<void> Message::receive(Player &player) {
    co_await receive(player.name);
    co_await receive(player.address);
    co_return;
}

boost::asio::awaitable<void> Message::receive(Position &position) {
    co_await receive(position.x);
    co_await receive(position.y);
    co_return;
}

boost::asio::awaitable<void> Message::receive(BombPlaced &event) {
    co_await receive(event.bomb_id);
    co_await receive(event.position);
    //event.timer = _server_params.get_bomb_timer(); TODO wyjebać to jakoś
    co_return;
}

boost::asio::awaitable<void> Message::receive(BombExploded &event) {
    co_await receive(event.bomb_id);
    co_await receive(event.robots_destroyed);
    co_await receive(event.blocks_destroyed);
    co_return;
}

boost::asio::awaitable<void> Message::receive(PlayerMoved &event) {
    co_await receive(event.player_id);
    co_await receive(event.position);
    co_return;
}

boost::asio::awaitable<void> Message::receive(BlockPlaced &event) {
    co_await receive(event.position);
    co_return;
}

boost::asio::awaitable<void> Message::receive(std::list<std::shared_ptr<Event>> &list) {
    uint32_t size;
    co_await receive(size);

    for (uint32_t i = 0; i < size; i++) {
        uint8_t event_id;
        co_await receive(event_id);

        BombPlaced bomb_placed;
        BombExploded bomb_exploded;
        PlayerMoved player_moved;
        BlockPlaced block_placed;

        switch (event_id) {
            case 0:
                co_await receive(bomb_placed);
                list.emplace_back(std::make_shared<BombPlaced>(bomb_placed));
                break;
            case 1:
                co_await receive(bomb_exploded);
                list.emplace_back(std::make_shared<BombExploded>(bomb_exploded));
                break;
            case 2:
                co_await receive(player_moved);
                list.emplace_back(std::make_shared<PlayerMoved>(player_moved));
                break;
            case 3:
                co_await receive(block_placed);
                list.emplace_back(std::make_shared<BlockPlaced>(block_placed));
                break;
        }
    }
    co_return;
}

boost::asio::awaitable<void> Message::receive(std::list<player_id_t> &list) {
    uint32_t size;
    co_await receive(size);
    for (uint32_t i = 0; i < size; i++) {
        player_id_t player_id;
        co_await receive(player_id);
        list.push_back(player_id);
    }
    co_return;
}

boost::asio::awaitable<void> Message::receive(std::list<Position> &list) {
    uint32_t size;
    co_await receive(size);
    for (uint32_t i = 0; i < size; i++) {
        Position position;
        co_await receive(position);
        list.push_back(position);
    }
    co_return;
}

boost::asio::awaitable<void> Message::receive(std::map<player_id_t, Player> &map) {
    uint32_t size;
    co_await receive(size);
    for (uint32_t i = 0; i < size; i++) {
        player_id_t key;
        Player value;
        co_await receive(key);
        co_await receive(value);
        map.insert({key, value});
    }
    co_return;
}

boost::asio::awaitable<void> Message::receive(std::map<player_id_t, score_t> &map) {
    uint32_t size;
    co_await receive(size);
    for (uint32_t i = 0; i < size; i++) {
        player_id_t key;
        score_t value;
        co_await receive(key);
        co_await receive(value);
        map.insert({key, value});
    }
    co_return;
}

void Message::send(uint8_t num) {
    uint64_t size = sizeof(uint8_t);
    memcpy(_buffer_tcp + _buf_len_tcp, &num, size);
    _buf_len_tcp += size;
}

void Message::send(const std::string &str) {
    uint8_t size = (uint8_t) str.size();
    send(size);
    memcpy(_buffer_tcp + _buf_len_tcp, str.c_str(), size);
    _buf_len_tcp += str.size();
}

void Message::send_gui(uint8_t num) {
    uint64_t size = sizeof(uint8_t);
    memcpy(_buffer_udp + _buf_len_udp, &num, size);
    _buf_len_udp += size;
}

void Message::send_gui(uint16_t num) {
    uint64_t size = sizeof(uint16_t);
    num = htons(num);
    memcpy(_buffer_udp + _buf_len_udp, &num, size);
    _buf_len_udp += size;
}

void Message::send_gui(uint32_t num) {
    uint64_t size = sizeof(uint32_t);
    num = htonl(num);
    memcpy(_buffer_udp + _buf_len_udp, &num, size);
    _buf_len_udp += size;
}

void Message::send_gui(const std::string &str) {
    uint8_t size = (uint8_t) str.size();
    send_gui(size);
    memcpy(_buffer_udp + _buf_len_udp, str.c_str(), size);
    _buf_len_udp += size;
}

void Message::send_gui(Player player) {
    send_gui(player.name);
    send_gui(player.address);
}

void Message::send_gui(Position position) {
    send_gui(position.x);
    send_gui(position.y);
}

void Message::send_gui(Bomb bomb) {
    send_gui(bomb.position);
    send_gui(bomb.timer);
}

void Message::send_gui(std::list<Position> list) {
    uint32_t size = (uint32_t) list.size();
    send_gui(size);
    for (auto el: list) {
        send_gui(el);
    }
}

void Message::send_gui(std::set<Position> set) {
    uint32_t size = (uint32_t) set.size();
    send_gui(size);
    for (auto el: set) {
        send_gui(el);
    }
}

void Message::send_gui(std::map<bomb_id_t, Bomb> map) {
    uint32_t size = (uint32_t) map.size();
    send_gui(size);
    for (auto el: map) {
        send_gui(el.second);
    }
}

void Message::send_gui(std::map<player_id_t, Player> map) {
    uint32_t size = (uint32_t) map.size();
    send_gui(size);
    for (auto el: map) {
        send_gui(el.first);
        send_gui(el.second);
    }
}

void Message::send_gui(std::map<player_id_t, Position> map) {
    uint32_t size = (uint32_t) map.size();
    send_gui(size);
    for (auto el: map) {
        send_gui(el.first);
        send_gui(el.second);
    }
}

void Message::send_gui(std::map<player_id_t, score_t> map) {
    uint32_t size = (uint32_t) map.size();
    send_gui(size);
    for (auto el: map) {
        send_gui(el.first);
        send_gui(el.second);
    }
}
