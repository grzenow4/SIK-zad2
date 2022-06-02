#pragma once

#include <iostream>
#include <utility>

class ServerParameters {
private:
    uint16_t _bomb_timer;
    uint8_t _players_count;
    uint64_t _turn_duration;
    uint16_t _explosion_radius;
    uint16_t _initial_blocks;
    uint16_t _game_length;
    std::string _name;
    uint16_t _port;
    uint32_t _seed;
    uint16_t _size_x;
    uint16_t _size_y;

public:
    ServerParameters() = default;

    ServerParameters(uint16_t bomb_timer, uint8_t players_count,
                     uint64_t turn_duration, uint16_t explosion_radius,
                     uint16_t initial_blocks, uint16_t game_length,
                     std::string name, uint16_t port, uint32_t seed,
                     uint16_t size_x, uint16_t size_y) :
            _bomb_timer(bomb_timer),
            _players_count(players_count),
            _turn_duration(turn_duration),
            _explosion_radius(explosion_radius),
            _initial_blocks(initial_blocks),
            _game_length(game_length),
            _name(std::move(name)),
            _port(port),
            _seed(seed),
            _size_x(size_x),
            _size_y(size_y) {}

    uint16_t get_bomb_timer() const {
        return _bomb_timer;
    }

    uint8_t get_players_count() const {
        return _players_count;
    }

    uint64_t get_turn_duration() const {
        return _turn_duration;
    }

    uint16_t get_explosion_radius() const {
        return _explosion_radius;
    }

    uint16_t get_initial_blocks() const {
        return _initial_blocks;
    }

    uint16_t get_game_length() const {
        return _game_length;
    }

    const std::string &get_name() const {
        return _name;
    }

    uint16_t get_port() const {
        return _port;
    }

    uint32_t get_seed() const {
        return _seed;
    }

    uint16_t get_size_x() const {
        return _size_x;
    }

    uint16_t get_size_y() const {
        return _size_y;
    }
};
