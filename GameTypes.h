#pragma once

#include <iostream>
#include <list>
#include <map>

using bomb_id_t = uint32_t;
using player_id_t = uint8_t;
using score_t = uint32_t;

enum Direction {
    Up,
    Right,
    Down,
    Left
};

struct Player {
    std::string name;
    std::string address;
    bool scored = false;
};

struct Position {
    uint16_t x;
    uint16_t y;

    bool operator==(const Position &other) const {
        return x == other.x && y == other.y;
    }

    bool operator<(const Position &other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
};

struct Bomb {
    uint32_t bomb_id;
    Position position;
    uint16_t timer;

    bool operator==(const Bomb &other) const {
        return bomb_id == other.bomb_id;
    }
};

struct GameStatus {
    uint16_t explosion_radius;
    uint16_t size_x;
    uint16_t size_y;
    std::set<player_id_t> players_scored;

    std::map<player_id_t, Player> players;
    uint16_t turn;
    std::map<player_id_t, Position> player_positions;
    std::list<Position> blocks;
    std::map<bomb_id_t, Bomb> bombs;
    std::set<Position> explosions;
    std::map<player_id_t, score_t> scores;
};

class Event {
public:
    virtual ~Event() {};

    virtual void resolve(GameStatus &) = 0;
};

class BombPlaced : public Event {
public:
    bomb_id_t bomb_id;
    Position position;
    uint16_t timer;

    void resolve(GameStatus &game_status) override {
        Bomb bomb{.bomb_id = bomb_id, .position = position, .timer = timer};
        game_status.bombs.insert({bomb_id, bomb});
    }
};

class BombExploded : public Event {
public:
    bomb_id_t bomb_id;
    std::list<player_id_t> robots_destroyed;
    std::list<Position> blocks_destroyed;

    void resolve(GameStatus &game_status) override {
        Position exp_pos = game_status.bombs.at(bomb_id).position;
        for (int i = exp_pos.x;
             i >= 0 && i >= exp_pos.x - game_status.explosion_radius; i--) {
            Position p{(uint16_t) i, exp_pos.y};
            game_status.explosions.insert(p);
            if (std::find(blocks_destroyed.begin(), blocks_destroyed.end(),
                          p) != blocks_destroyed.end()) {
                break;
            }
        }
        for (int i = exp_pos.x; i <= game_status.size_x && i <= exp_pos.x +
                                                                game_status.explosion_radius; i++) {
            Position p{(uint16_t) i, exp_pos.y};
            game_status.explosions.insert(p);
            if (std::find(blocks_destroyed.begin(), blocks_destroyed.end(),
                          p) != blocks_destroyed.end()) {
                break;
            }
        }
        for (int i = exp_pos.y;
             i >= 0 && i >= exp_pos.y - game_status.explosion_radius; i--) {
            Position p{exp_pos.x, (uint16_t) i};
            game_status.explosions.insert(p);
            if (std::find(blocks_destroyed.begin(), blocks_destroyed.end(),
                          p) != blocks_destroyed.end()) {
                break;
            }
        }
        for (int i = exp_pos.y; i <= game_status.size_y && i <= exp_pos.y +
                                                                game_status.explosion_radius; i++) {
            Position p{exp_pos.x, (uint16_t) i};
            game_status.explosions.insert(p);
            if (std::find(blocks_destroyed.begin(), blocks_destroyed.end(),
                          p) != blocks_destroyed.end()) {
                break;
            }
        }

        for (auto block: blocks_destroyed) {
            game_status.blocks.remove(block);
        }

        for (auto p_id: robots_destroyed) {
            if (!game_status.players_scored.contains(p_id)) {
                game_status.players_scored.insert(p_id);
                auto score = game_status.scores.at(p_id);
                score++;
                game_status.scores.erase(p_id);
                game_status.scores.insert({p_id, score});
            }
        }

        game_status.bombs.erase(bomb_id);
    }
};

class PlayerMoved : public Event {
public:
    uint8_t player_id;
    Position position;

    void resolve(GameStatus &game_status) override {
        game_status.player_positions.erase(player_id);
        game_status.player_positions.insert({player_id, position});
    }
};

class BlockPlaced : public Event {
public:
    Position position;

    void resolve(GameStatus &game_status) override {
        game_status.blocks.emplace_back(position);
    }
};
