#include "GameRoom.h"
#include "GameSession.h"

GameRoom::GameRoom(boost::asio::io_context &io_context,
                   ServerParameters server_params) :
        _server_params(server_params),
        _timer(io_context, boost::asio::chrono::seconds(_server_params.get_turn_duration())) {
    _next_random_num = _server_params.get_seed();
}

void GameRoom::join(std::shared_ptr<GameSession> participant) {
    _players.insert(participant);
    participant->send_hello(_server_params);

    if (_game_in_progress) {
        participant->send_game_started(_game_status.players);
        for (auto turn: _game_status.turns) {
            participant->send_turn(turn.first, turn.second);
        }
    } else {
        for (auto player: _game_status.players) {
            participant->send_accepted_player(player.first, player.second);
        }
    }
}

void GameRoom::disconnect(std::shared_ptr<GameSession> participant) {
    _players.erase(participant);
}

void GameRoom::add_message(std::shared_ptr<GameSession> participant, ClientMessage msg) {
    _moves.insert({participant, msg});
}

void GameRoom::resolve_join(std::shared_ptr<GameSession> participant, Player player) {
    if (!_game_in_progress) {
        participant->set_id(_next_id);
        _game_status.players.insert({_next_id, player});
        sendall_accepted_player(_next_id, player);
        _next_id++;

        if (_game_status.players.size() == _server_params.get_players_count()) {
            _game_in_progress = true;
            init_game();
        }
    }
}

uint32_t GameRoom::random() {
    _next_random_num = (uint32_t) ((uint64_t) _next_random_num * Q % MOD);
    return _next_random_num;
}

void GameRoom::resolve_place_bomb(std::shared_ptr<GameSession> participant) {
    player_id_t id = participant->get_id();
    Position position = _game_status.player_positions[id];

    Bomb bomb{.bomb_id = _next_bomb,
            .position = position,
            .timer = _server_params.get_bomb_timer()};

    _game_status.bombs.insert({_next_bomb, bomb});
    BombPlaced event{.bomb_id = _next_bomb, .position = position};
    _game_status.turns[_game_status.turn].emplace_back(BombPlacedT, event);
}

void GameRoom::resolve_place_block(std::shared_ptr<GameSession> participant) {
    player_id_t id = participant->get_id();
    Position position = _game_status.player_positions[id];

    if (std::find(_game_status.blocks.begin(), _game_status.blocks.end(),
                  position) != _game_status.blocks.end()) {
        BlockPlaced event{.position = position};
        _game_status.blocks.push_back(position);
        _game_status.turns[_game_status.turn].emplace_back(BlockPlacedT, event);
    }
}

void GameRoom::resolve_move(std::shared_ptr<GameSession> participant, Direction direction) {
    player_id_t id = participant->get_id();
    Position position = _game_status.player_positions[id];

    switch (direction) {
        case Up:
            if (position.y == _server_params.get_size_y() - 1)
                return;
            position.y++;
            break;
        case Right:
            if (position.x == _server_params.get_size_x() - 1)
                return;
            position.x++;
            break;
        case Down:
            if (position.y == 0)
                return;
            position.y--;
            break;
        case Left:
            if (position.x == 0)
                return;
            position.x--;
            break;
    }

    if (std::find(_game_status.blocks.begin(), _game_status.blocks.end(),
                  position) != _game_status.blocks.end()) {
        return;
    }

    _game_status.player_positions[id] = position;
    PlayerMoved event{.player_id = id, .position = position};
    _game_status.turns[_game_status.turn].emplace_back(PlayerMovedT, event);
}

void GameRoom::init_game() {
    _game_status.turn = 0;
    std::list<Event> events;

    for (auto player: _game_status.players) {
        uint16_t x = (uint16_t) (random() % _server_params.get_size_x());
        uint16_t y = (uint16_t) (random() % _server_params.get_size_y());
        player_id_t p_id = player.first;
        Position position{.x = x, .y = y};
        PlayerMoved event{.player_id = p_id, .position = position};
        _game_status.player_positions.insert({p_id, position});
        events.emplace_back(PlayerMovedT, event);
    }

    for (size_t i = 0; i < _server_params.get_initial_blocks(); i++) {
        uint16_t x = (uint16_t) (random() % _server_params.get_size_x());
        uint16_t y = (uint16_t) (random() % _server_params.get_size_y());
        Position position{.x = x, .y = y};
        if (std::find(_game_status.blocks.begin(), _game_status.blocks.end(),
                      position) == _game_status.blocks.end()) {
            BlockPlaced event{.position = position};
            _game_status.blocks.push_back(position);
            events.emplace_back(BlockPlacedT, event);
        }
    }

    sendall_game_started();
    sendall_turn(events);

    _game_status.turns[_game_status.turn] = events;
    _game_status.turn++;
    handle_turn();
}

void GameRoom::explode_bombs() {
    for (auto el: _game_status.bombs) {
        Bomb bomb = el.second;
        if (bomb.timer == 0) {
            std::list<player_id_t> robots_destroyed;
            std::list<Position> blocks_destroyed;

            for (int i = bomb.position.x; i >= 0 && i >= bomb.position.x -
                                                         _server_params.get_explosion_radius(); i--) {
                Position p{(uint16_t) i, bomb.position.y};
                for (auto it = _game_status.player_positions.begin();
                     it != _game_status.player_positions.end(); it++) {
                    if (it->second == p) {
                        robots_destroyed.emplace_back(it->first);
                    }
                }
                if (std::find(_game_status.blocks.begin(), _game_status.blocks.end(),
                              p) != _game_status.blocks.end()) {
                    blocks_destroyed.emplace_back(p);
                    break;
                }
            }
            for (int i = bomb.position.x; i < _server_params.get_size_x() &&
                                          i <= bomb.position.x +
                                               _server_params.get_explosion_radius(); i++) {
                Position p{(uint16_t) i, bomb.position.y};
                for (auto it = _game_status.player_positions.begin();
                     it != _game_status.player_positions.end(); it++) {
                    if (it->second == p) {
                        robots_destroyed.emplace_back(it->first);
                    }
                }
                if (std::find(_game_status.blocks.begin(), _game_status.blocks.end(),
                              p) != _game_status.blocks.end()) {
                    blocks_destroyed.emplace_back(p);
                    break;
                }
            }
            for (int i = bomb.position.y; i >= 0 && i >= bomb.position.y -
                                                         _server_params.get_explosion_radius(); i--) {
                Position p{bomb.position.x, (uint16_t) i};
                for (auto it = _game_status.player_positions.begin();
                     it != _game_status.player_positions.end(); it++) {
                    if (it->second == p) {
                        robots_destroyed.emplace_back(it->first);
                    }
                }
                if (std::find(_game_status.blocks.begin(), _game_status.blocks.end(),
                              p) != _game_status.blocks.end()) {
                    blocks_destroyed.emplace_back(p);
                    break;
                }
            }
            for (int i = bomb.position.y; i < _server_params.get_size_y() &&
                                          i <= bomb.position.x +
                                               _server_params.get_explosion_radius(); i++) {
                Position p{bomb.position.x, (uint16_t) i};
                for (auto it = _game_status.player_positions.begin();
                     it != _game_status.player_positions.end(); it++) {
                    if (it->second == p) {
                        robots_destroyed.emplace_back(it->first);
                    }
                }
                if (std::find(_game_status.blocks.begin(), _game_status.blocks.end(),
                              p) != _game_status.blocks.end()) {
                    blocks_destroyed.emplace_back(p);
                    break;
                }
            }
            BombExploded event{.bomb_id = bomb.bomb_id,
                    .robots_destroyed = robots_destroyed,
                    .blocks_destroyed = blocks_destroyed};
            _game_status.turns[_game_status.turn].emplace_back(BombExplodedT, event);
        }
    }
}

void GameRoom::handle_turn() {
    _timer.expires_from_now(boost::asio::chrono::milliseconds(
            _server_params.get_turn_duration()));

    _timer.async_wait([this](boost::system::error_code ec) {
        if (ec) {
            std::cerr << "Round timer error.\n";
            exit(1);
        } else {
            for (auto pair: _moves) {
                switch (pair.second.message_id) {
                    case 1:
                        resolve_place_bomb(pair.first);
                        break;
                    case 2:
                        resolve_place_block(pair.first);
                        break;
                    case 3:
                        resolve_move(pair.first, pair.second.direction);
                        break;
                }
            }
            _moves.clear();

            for (auto &pair: _game_status.bombs) {
                pair.second.timer--;
            }
            explode_bombs();

            sendall_turn(_game_status.turns[_game_status.turn]);

            if (_game_status.turn == _server_params.get_game_length()) {
                sendall_game_ended();
                reset_state();
                return;
            } else {
                _game_status.turn++;
                handle_turn();
            }
        }
    });
}

void GameRoom::reset_state() {
    _game_in_progress = false;
    _next_id = 0;
    _next_bomb = 0;
    _next_random_num = _server_params.get_seed();
    _game_status.turns.clear();
    _game_status.blocks.clear();
    _game_status.bombs.clear();
    _game_status.players.clear();
    _game_status.player_positions.clear();
    _game_status.scores.clear();
}

void GameRoom::sendall_accepted_player(player_id_t player_id, Player player) {
    for (auto participant: _players) {
        participant->send_accepted_player(player_id, player);
    }
}

void GameRoom::sendall_game_started() {
    for (auto participant: _players) {
        participant->send_game_started(_game_status.players);
    }
}

void GameRoom::sendall_turn(std::list<Event> events) {
    for (auto participant: _players) {
        participant->send_turn(_game_status.turn, events);
    }
}

void GameRoom::sendall_game_ended() {
    for (auto participant: _players) {
        participant->send_game_ended(_game_status.scores);
    }
}
