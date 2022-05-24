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
};

struct Position {
    uint16_t x;
    uint16_t y;
};

struct Bomb {
    Position position;
    uint16_t timer;
};

class Event {
};

class BombPlaced : public Event {
public:
    bomb_id_t bomb_id;
    Position position;
};

class BombExploded : public Event {
public:
    bomb_id_t bomb_id;
    std::list<player_id_t> robots_destroyed;
    std::list<Position> blocks_destroyed;
};

class PlayerMoved : public Event {
public:
    uint8_t player_id;
    Position position;
};

class BlockPlaced : public Event {
public:
    Position position;
};
