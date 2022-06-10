#pragma once

#include <algorithm>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <string>
#include <variant>

using bomb_id_t = uint32_t;
using player_id_t = uint8_t;
using score_t = uint32_t;

enum Direction {
    Up,
    Right,
    Down,
    Left
};

enum EventType {
    BombPlacedT,
    BombExplodedT,
    PlayerMovedT,
    BlockPlacedT
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
    bomb_id_t bomb_id;
    Position position;
    uint16_t timer;

    bool operator==(const Bomb &other) const {
        return bomb_id == other.bomb_id;
    }
};

struct BombPlaced {
    bomb_id_t bomb_id;
    Position position;
};

struct BombExploded {
    bomb_id_t bomb_id;
    std::list<player_id_t> robots_destroyed;
    std::list<Position> blocks_destroyed;
};

struct PlayerMoved {
    player_id_t player_id;
    Position position;
};

struct BlockPlaced {
    Position position;
};

class Event {
private:
    EventType _type;
public:
    EventType get_type() {
        return _type;
    }

    std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced> item;

    Event(EventType type, struct BombPlaced &bomb_placed) : _type(type) {
        item.emplace<BombPlaced>(bomb_placed);
    }

    Event(EventType type, struct BombExploded &bomb_exploded) : _type(type) {
        item.emplace<BombExploded>(bomb_exploded);
    }

    Event(EventType type, struct PlayerMoved &player_moved) : _type(type) {
        item.emplace<PlayerMoved>(player_moved);
    }

    Event(EventType type, struct BlockPlaced &block_placed) : _type(type) {
        item.emplace<BlockPlaced>(block_placed);
    }
};

struct GameStatus {
    uint16_t turn;
    std::map<uint16_t, std::list<Event>> turns;
    std::list<Position> blocks;
    std::set<Position> explosions;
    std::map<bomb_id_t, Bomb> bombs;
    std::map<player_id_t, Player> players;
    std::map<player_id_t, Position> player_positions;
    std::set<player_id_t> players_scored;
    std::map<player_id_t, score_t> scores;
};
