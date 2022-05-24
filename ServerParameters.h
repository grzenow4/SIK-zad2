#pragma once

#include <iostream>
#include <utility>

class ServerParameters {
private:
    std::string _name;
    int _players_count;
    uint16_t _size_x;
    uint16_t _size_y;
    uint16_t _game_length;
    uint16_t _explosion_radius;
    uint16_t _bomb_timer;

public:
    ServerParameters() = default;
    ServerParameters(std::string name, int players_count,
                     uint16_t size_x, uint16_t size_y, uint16_t game_length,
                     uint16_t explosion_radius, uint16_t bomb_timer) :
            _name(std::move(name)), _players_count(players_count),
            _size_x(size_x), _size_y(size_y), _game_length(game_length),
            _explosion_radius(explosion_radius), _bomb_timer(bomb_timer) {}


    void print() {
        std::cout << "name: " << _name << '\n';
        std::cout << "players_count: " << _players_count << '\n';
        std::cout << "size_x: " << _size_x << '\n';
        std::cout << "size_y: " << _size_y << '\n';
        std::cout << "game_length: " << _game_length << '\n';
        std::cout << "explosion_radius: " << _explosion_radius << '\n';
        std::cout << "bomb_timer: " << _bomb_timer << '\n';
    }
};
