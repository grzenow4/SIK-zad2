#include "Client.h"

Client::Client(boost::asio::io_context &io_context,
               const tcp::resolver::results_type &endpoint_server,
               udp::endpoint endpoint_gui,
               ClientParameters client_params) :
        _tcp_socket(io_context),
        _endpoint_server(endpoint_server),
        _endpoint_gui(endpoint_gui),
        _udp_socket_send(io_context),
        _udp_socket_rec(io_context),
        _client_params(client_params) {
    boost::asio::connect(_tcp_socket, endpoint_server);
    _tcp_socket.set_option(tcp::no_delay(true));

    _udp_socket_send.open(udp::v6());

    _udp_socket_rec.open(udp::v6());
    _udp_socket_rec.bind({udp::v6(), _client_params.get_port()});

    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    co_spawn(io_context, server_listener(), boost::asio::detached);
    co_spawn(io_context, gui_listener(), boost::asio::detached);

    io_context.run();
}

boost::asio::awaitable<void> Client::server_listener() {
    for (;;) {
        co_await receive_message();
    }
    co_return;
}

boost::asio::awaitable<void> Client::gui_listener() {
    for (;;) {
        co_await receive_input_message();
        if (_in_lobby && _valid_msg) {
            co_await send_join();
        }
    }
    co_return;
}

void Client::clear_buffer() {
    _buf_len_tcp = 0;
    _read_idx_tcp = 0;
}

void Client::clear_buffer_udp() {
    _buf_len_udp = 0;
    _read_idx_udp = 0;
}

boost::asio::awaitable<void> Client::read_message(size_t n) {
    size_t already_read = 0;
    while (already_read < n) {
        already_read += co_await _tcp_socket.async_read_some(
                boost::asio::buffer(_buf_tcp + _buf_len_tcp + already_read,
                                    n - already_read),
                boost::asio::use_awaitable);
    }
    _buf_len_tcp += n;
    co_return;
}

boost::asio::awaitable<void> Client::read_message_udp() {
    _buf_len_udp = co_await _udp_socket_rec.async_receive(
            boost::asio::buffer(_buf_udp),
            boost::asio::use_awaitable);
    co_return;
}

boost::asio::awaitable<void> Client::receive(uint8_t &num) {
    uint64_t size = sizeof(uint8_t);
    co_await read_message(size);
    memcpy(&num, _buf_tcp + _read_idx_tcp, size);
    _read_idx_tcp += size;
    co_return;
}

boost::asio::awaitable<void> Client::receive(uint16_t &num) {
    uint64_t size = sizeof(uint16_t);
    co_await read_message(size);
    memcpy(&num, _buf_tcp + _read_idx_tcp, size);
    _read_idx_tcp += size;
    num = ntohs(num);
    co_return;
}

boost::asio::awaitable<void> Client::receive(uint32_t &num) {
    uint64_t size = sizeof(uint32_t);
    co_await read_message(size);
    memcpy(&num, _buf_tcp + _read_idx_tcp, size);
    _read_idx_tcp += size;
    num = ntohl(num);
    co_return;
}

boost::asio::awaitable<void> Client::receive(std::string &str) {
    co_await read_message(1);
    uint64_t size = (uint64_t) _buf_tcp[_read_idx_tcp];
    _read_idx_tcp++;
    co_await read_message(size);
    str = {_buf_tcp + _read_idx_tcp, size};
    _read_idx_tcp += size;
    co_return;
}

boost::asio::awaitable<void> Client::receive(Player &player) {
    co_await receive(player.name);
    co_await receive(player.address);
    co_return;
}

boost::asio::awaitable<void> Client::receive(Position &position) {
    co_await receive(position.x);
    co_await receive(position.y);
    co_return;
}

boost::asio::awaitable<void> Client::receive(BombPlaced &event) {
    co_await receive(event.bomb_id);
    co_await receive(event.position);
    co_return;
}

boost::asio::awaitable<void> Client::receive(BombExploded &event) {
    co_await receive(event.bomb_id);
    co_await receive(event.robots_destroyed);
    co_await receive(event.blocks_destroyed);
    co_return;
}

boost::asio::awaitable<void> Client::receive(PlayerMoved &event) {
    co_await receive(event.player_id);
    co_await receive(event.position);
    co_return;
}

boost::asio::awaitable<void> Client::receive(BlockPlaced &event) {
    co_await receive(event.position);
    co_return;
}

boost::asio::awaitable<void> Client::receive(std::list<Event> &list) {
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
                list.emplace_back(BombPlacedT, bomb_placed);
                break;
            case 1:
                co_await receive(bomb_exploded);
                list.emplace_back(BombExplodedT, bomb_exploded);
                break;
            case 2:
                co_await receive(player_moved);
                list.emplace_back(PlayerMovedT, player_moved);
                break;
            case 3:
                co_await receive(block_placed);
                list.emplace_back(BlockPlacedT, block_placed);
                break;
        }
    }
    co_return;
}

boost::asio::awaitable<void> Client::receive(std::list<player_id_t> &list) {
    uint32_t size;
    co_await receive(size);

    for (uint32_t i = 0; i < size; i++) {
        player_id_t player_id;
        co_await receive(player_id);
        list.push_back(player_id);
    }

    co_return;
}

boost::asio::awaitable<void> Client::receive(std::list<Position> &list) {
    uint32_t size;
    co_await receive(size);

    for (uint32_t i = 0; i < size; i++) {
        Position position;
        co_await receive(position);
        list.push_back(position);
    }

    co_return;
}

boost::asio::awaitable<void> Client::receive(std::map<player_id_t, Player> &map) {
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

boost::asio::awaitable<void> Client::receive(std::map<player_id_t, score_t> &map) {
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

boost::asio::awaitable<void> Client::receive_hello() {
    std::string name;
    uint8_t players_count;
    uint16_t size_x, size_y, game_length, explosion_radius, bomb_timer;

    co_await receive(name);
    co_await receive(players_count);
    co_await receive(size_x);
    co_await receive(size_y);
    co_await receive(game_length);
    co_await receive(explosion_radius);
    co_await receive(bomb_timer);

    _server_params = ServerParameters(bomb_timer,
                                      players_count,
                                      0,
                                      explosion_radius,
                                      0,
                                      game_length,
                                      name,
                                      0,
                                      0,
                                      size_x,
                                      size_y);
    co_return;
}

boost::asio::awaitable<void> Client::receive_accepted_player() {
    player_id_t player_id;
    Player player;
    co_await receive(player_id);
    co_await receive(player);
    _game_status.players.insert({player_id, player});
    co_return;
}

boost::asio::awaitable<void> Client::receive_game_started() {
    std::map<player_id_t, Player> players;
    co_await receive(players);
    _game_status.players = players;
    for (auto player: _game_status.players) {
        _game_status.scores.insert({player.first, 0});
    }
    co_return;
}

void Client::resolve(BombPlaced event) {
    Bomb bomb{.bomb_id = event.bomb_id,
              .position = event.position,
              .timer = _server_params.get_bomb_timer()};
    _game_status.bombs.insert({event.bomb_id, bomb});
}

void Client::resolve(BombExploded event) {
    Position exp_pos = _game_status.bombs.at(event.bomb_id).position;
    for (int i = exp_pos.x;
            i >= 0 && i >= exp_pos.x - _server_params.get_explosion_radius(); i--) {
        Position p{(uint16_t) i, exp_pos.y};
        _game_status.explosions.insert(p);
        if (std::find(event.blocks_destroyed.begin(), event.blocks_destroyed.end(),
                        p) != event.blocks_destroyed.end()) {
            break;
        }
    }
    for (int i = exp_pos.x;
            i < _server_params.get_size_x() &&
            i <= exp_pos.x + _server_params.get_explosion_radius(); i++) {
        Position p{(uint16_t) i, exp_pos.y};
        _game_status.explosions.insert(p);
        if (std::find(event.blocks_destroyed.begin(), event.blocks_destroyed.end(),
                        p) != event.blocks_destroyed.end()) {
            break;
        }
    }
    for (int i = exp_pos.y;
            i >= 0 && i >= exp_pos.y - _server_params.get_explosion_radius(); i--) {
        Position p{exp_pos.x, (uint16_t) i};
        _game_status.explosions.insert(p);
        if (std::find(event.blocks_destroyed.begin(), event.blocks_destroyed.end(),
                        p) != event.blocks_destroyed.end()) {
            break;
        }
    }
    for (int i = exp_pos.y;
            i < _server_params.get_size_y() &&
            i <= exp_pos.y + _server_params.get_explosion_radius(); i++) {
        Position p{exp_pos.x, (uint16_t) i};
        _game_status.explosions.insert(p);
        if (std::find(event.blocks_destroyed.begin(), event.blocks_destroyed.end(),
                        p) != event.blocks_destroyed.end()) {
            break;
        }
    }

    for (auto block: event.blocks_destroyed) {
        _game_status.blocks.remove(block);
    }

    for (auto p_id: event.robots_destroyed) {
        if (!_game_status.players_scored.contains(p_id)) {
            _game_status.players_scored.insert(p_id);
            auto score = _game_status.scores.at(p_id);
            score++;
            _game_status.scores.erase(p_id);
            _game_status.scores.insert({p_id, score});
        }
    }

    _game_status.bombs.erase(event.bomb_id);
}

void Client::resolve(PlayerMoved event) {
    _game_status.player_positions.erase(event.player_id);
    _game_status.player_positions.insert({event.player_id, event.position});
}

void Client::resolve(BlockPlaced event) {
    _game_status.blocks.push_back(event.position);
}

boost::asio::awaitable<void> Client::receive_turn() {
    uint16_t turn;
    std::list<Event> events;
    co_await receive(turn);
    co_await receive(events);
    _game_status.turn = turn;
    for (auto event: events) {
        switch (event.get_type()) {
            case BombPlacedT:
                resolve(std::get<BombPlaced>(event.item));
                break;
            case BombExplodedT:
                resolve(std::get<BombExploded>(event.item));
                break;
            case PlayerMovedT:
                resolve(std::get<PlayerMoved>(event.item));
                break;
            case BlockPlacedT:
                resolve(std::get<BlockPlaced>(event.item));
                break;
        }
    }
    co_return;
}

boost::asio::awaitable<void> Client::receive_game_ended() {
    std::map<player_id_t, score_t> scores;
    co_await receive(scores);
    _in_lobby = true;
    _game_status.turn = 0;
    _game_status.blocks.clear();
    co_return;
}

boost::asio::awaitable<void> Client::receive_message() {
    clear_buffer();
    uint8_t msg_id;
    co_await receive(msg_id);

    switch (msg_id) {
        case 0: // Hello
            co_await receive_hello();
            co_await send_lobby();
            break;
        case 1: // AcceptedPlayer
            co_await receive_accepted_player();
            co_await send_lobby();
            break;
        case 2: // GameStarted
            co_await receive_game_started();
            _in_lobby = false;
            break;
        case 3: // Turn
            co_await receive_turn();
            co_await send_game();
            break;
        case 4: // GameEnded
            co_await receive_game_ended();
            co_await send_lobby();
            break;
        default:
            exit(1);
    }

    clear_buffer();
    co_return;
}

boost::asio::awaitable<void> Client::receive_input_message() {
    clear_buffer_udp();
    co_await read_message_udp();

    switch ((int) _buf_udp[0]) {
        case 0: // PlaceBomb
            _valid_msg = (_buf_len_udp == 1);
            if (_valid_msg && !_in_lobby)
                co_await send_place_bomb();
            break;
        case 1: // PlaceBlock
            _valid_msg = (_buf_len_udp == 1);
            if (_valid_msg && !_in_lobby)
                co_await send_place_block();
            break;
        case 2: // Move
            _valid_msg = (_buf_len_udp == 2 && _buf_udp[1] >= 0 && _buf_udp[1] <= 3);
            if (_valid_msg && !_in_lobby)
                co_await send_move((Direction) _buf_udp[1]);
            break;
        default:
            _valid_msg = false;
            break;
    }

    clear_buffer_udp();
}

void Client::send(uint8_t num) {
    uint64_t size = sizeof(uint8_t);
    memcpy(_buf_tcp + _buf_len_tcp, &num, size);
    _buf_len_tcp += size;
}

void Client::send(const std::string &str) {
    uint8_t size = (uint8_t) str.size();
    send(size);
    memcpy(_buf_tcp + _buf_len_tcp, str.c_str(), size);
    _buf_len_tcp += str.size();
}

boost::asio::awaitable<void> Client::send_join() {
    clear_buffer();
    uint8_t message_no = 0;
    send(message_no);
    send(_client_params.get_nickname());
    co_await _tcp_socket.async_send(boost::asio::buffer(_buf_tcp, _buf_len_tcp),
                                    boost::asio::use_awaitable);
    clear_buffer();
    co_return;
}

boost::asio::awaitable<void> Client::send_place_bomb() {
    clear_buffer();
    uint8_t message_no = 1;
    send(message_no);
    co_await _tcp_socket.async_send(boost::asio::buffer(_buf_tcp, _buf_len_tcp),
                                    boost::asio::use_awaitable);
    clear_buffer();
    co_return;
}

boost::asio::awaitable<void> Client::send_place_block() {
    clear_buffer();
    uint8_t message_no = 2;
    send(message_no);
    co_await _tcp_socket.async_send(boost::asio::buffer(_buf_tcp, _buf_len_tcp),
                                    boost::asio::use_awaitable);
    clear_buffer();
    co_return;
}

boost::asio::awaitable<void> Client::send_move(Direction direction) {
    clear_buffer();
    uint8_t message_no = 3;
    send(message_no);
    send((uint8_t) direction);
    co_await _tcp_socket.async_send(boost::asio::buffer(_buf_tcp, _buf_len_tcp),
                                    boost::asio::use_awaitable);
    clear_buffer();
    co_return;
}

void Client::send_gui(uint8_t num) {
    uint64_t size = sizeof(uint8_t);
    memcpy(_buf_udp + _buf_len_udp, &num, size);
    _buf_len_udp += size;
}

void Client::send_gui(uint16_t num) {
    uint64_t size = sizeof(uint16_t);
    num = htons(num);
    memcpy(_buf_udp + _buf_len_udp, &num, size);
    _buf_len_udp += size;
}

void Client::send_gui(uint32_t num) {
    uint64_t size = sizeof(uint32_t);
    num = htonl(num);
    memcpy(_buf_udp + _buf_len_udp, &num, size);
    _buf_len_udp += size;
}

void Client::send_gui(const std::string &str) {
    uint8_t size = (uint8_t) str.size();
    send_gui(size);
    memcpy(_buf_udp + _buf_len_udp, str.c_str(), size);
    _buf_len_udp += size;
}

void Client::send_gui(Player player) {
    send_gui(player.name);
    send_gui(player.address);
}

void Client::send_gui(Position position) {
    send_gui(position.x);
    send_gui(position.y);
}

void Client::send_gui(Bomb bomb) {
    send_gui(bomb.position);
    send_gui(bomb.timer);
}

void Client::send_gui(std::list<Position> list) {
    uint32_t size = (uint32_t) list.size();
    send_gui(size);
    for (auto el: list) {
        send_gui(el);
    }
}

void Client::send_gui(std::set<Position> set) {
    uint32_t size = (uint32_t) set.size();
    send_gui(size);
    for (auto el: set) {
        send_gui(el);
    }
}

void Client::send_gui(std::map<bomb_id_t, Bomb> map) {
    uint32_t size = (uint32_t) map.size();
    send_gui(size);
    for (auto el: map) {
        send_gui(el.second);
    }
}

void Client::send_gui(std::map<player_id_t, Player> map) {
    uint32_t size = (uint32_t) map.size();
    send_gui(size);
    for (auto el: map) {
        send_gui(el.first);
        send_gui(el.second);
    }
}

void Client::send_gui(std::map<player_id_t, Position> map) {
    uint32_t size = (uint32_t) map.size();
    send_gui(size);
    for (auto el: map) {
        send_gui(el.first);
        send_gui(el.second);
    }
}

void Client::send_gui(std::map<player_id_t, score_t> map) {
    uint32_t size = (uint32_t) map.size();
    send_gui(size);
    for (auto el: map) {
        send_gui(el.first);
        send_gui(el.second);
    }
}

boost::asio::awaitable<void> Client::send_lobby() {
    clear_buffer_udp();
    uint8_t message_no = 0;
    send_gui(message_no);
    send_gui(_server_params.get_name());
    send_gui(_server_params.get_players_count());
    send_gui(_server_params.get_size_x());
    send_gui(_server_params.get_size_y());
    send_gui(_server_params.get_game_length());
    send_gui(_server_params.get_explosion_radius());
    send_gui(_server_params.get_bomb_timer());
    send_gui(_game_status.players);
    co_await _udp_socket_send.async_send_to(
            boost::asio::buffer(_buf_udp, _buf_len_udp),
            _endpoint_gui,
            boost::asio::use_awaitable);
    clear_buffer_udp();
    co_return;
}

boost::asio::awaitable<void> Client::send_game() {
    clear_buffer_udp();
    uint8_t message_no = 1;
    send_gui(message_no);
    send_gui(_server_params.get_name());
    send_gui(_server_params.get_size_x());
    send_gui(_server_params.get_size_y());
    send_gui(_server_params.get_game_length());
    send_gui(_game_status.turn);
    send_gui(_game_status.players);
    send_gui(_game_status.player_positions);
    send_gui(_game_status.blocks);
    send_gui(_game_status.bombs);
    send_gui(_game_status.explosions);
    send_gui(_game_status.scores);
    co_await _udp_socket_send.async_send_to(
            boost::asio::buffer(_buf_udp, _buf_len_udp),
            _endpoint_gui,
            boost::asio::use_awaitable);
    for (auto &pair : _game_status.bombs) {
        pair.second.timer--;
    }
    _game_status.explosions.clear();
    _game_status.players_scored.clear();
    clear_buffer_udp();
    co_return;
}
