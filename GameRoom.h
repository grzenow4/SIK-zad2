#pragma once

#include "GameTypes.h"
#include "ServerParameters.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/system.hpp>

#include <memory>

#define Q 48271
#define MOD 2147483647

using boost::asio::ip::tcp;

// Forward declaration.
class GameSession;

class GameRoom {
private:
    std::set<std::shared_ptr<GameSession>> _players;
    std::map<std::shared_ptr<GameSession>, ClientMessage> _moves;

    GameStatus _game_status;
    ServerParameters _server_params;

    boost::asio::steady_timer _timer;

    bool _game_in_progress = false;
    player_id_t _next_id = 0;
    bomb_id_t _next_bomb = 0;
    uint32_t _next_random_num;

public:
    GameRoom(boost::asio::io_context &io_context, ServerParameters server_params);

    void join(std::shared_ptr<GameSession> participant);

    void disconnect(std::shared_ptr<GameSession> participant);

    void add_message(std::shared_ptr<GameSession> participant, ClientMessage msg);

    void resolve_join(std::shared_ptr<GameSession> participant, Player player);

private:
    uint32_t random();

    void resolve_place_bomb(std::shared_ptr<GameSession> participant);

    void resolve_place_block(std::shared_ptr<GameSession> participant);

    void resolve_move(std::shared_ptr<GameSession> participant, Direction direction);

    void init_game();

    void explode_bombs();

    void handle_turn();

    void reset_state();

    void sendall_accepted_player(player_id_t player_id, Player player);

    void sendall_game_started();

    void sendall_turn(std::list<Event> events);

    void sendall_game_ended();
};
