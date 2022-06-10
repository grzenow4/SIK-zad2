#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/system.hpp>

#include <cstdint>
#include <sstream>
#include <unistd.h>

#include "Server.h"

namespace po = boost::program_options;
using boost::asio::ip::tcp;

static void check_uint8(int num) {
    if (num < 0 || num > UINT8_MAX) {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

static void check_uint16(int num) {
    if (num < 0 || num > UINT16_MAX) {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

static void check_uint32(long long num) {
    if (num < 0 || num > UINT32_MAX) {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

int main(int argc, char *argv[]) {
    ServerParameters server_params;

    try {
        int bomb_timer, players_count, explosion_radius,
                initial_blocks, game_length, port, seed, size_x, size_y;
        uint64_t turn_duration;
        std::string name;

        po::options_description desc("Allowed options");
        desc.add_options()
                ("bomb-timer,b",
                 po::value<int>(&bomb_timer)->required()->notifier(
                         &check_uint16),
                 "bomb timer")
                ("players-count,c",
                 po::value<int>(&players_count)->required()->notifier(
                         &check_uint8),
                 "players count")
                ("turn-duration,d",
                 po::value<uint64_t>(&turn_duration)->required(),
                 "turn duration")
                ("explosion-radius,e",
                 po::value<int>(&explosion_radius)->required()->notifier(
                         &check_uint16),
                 "explosion radius")
                ("help,h", "produce help message")
                ("initial-blocks,k",
                 po::value<int>(&initial_blocks)->required()->notifier(
                         &check_uint16),
                 "initial blocks")
                ("game-length,l",
                 po::value<int>(&game_length)->required()->notifier(
                         &check_uint16),
                 "game length")
                ("server-name,n",
                 po::value<std::string>(&name)->required(),
                 "server name")
                ("port,p",
                 po::value<int>(&port)->required()->notifier(&check_uint16),
                 "port number")
                ("seed,s",
                 po::value<int>(&seed)->notifier(&check_uint32),
                 "seed")
                ("size-x,x",
                 po::value<int>(&size_x)->required()->notifier(&check_uint16),
                 "map size x")
                ("size-y,y",
                 po::value<int>(&size_y)->required()->notifier(&check_uint16),
                 "map size y");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << "Usage: " << argv[0] << " [options]\n" << desc << '\n';
            return 0;
        }
        if (!vm.count("seed")) {
            seed = (int) std::chrono::system_clock::now().time_since_epoch().count();
        }
        po::notify(vm);

        server_params = ServerParameters(
                (uint16_t) vm["bomb-timer"].as<int>(),
                (uint8_t) vm["players-count"].as<int>(),
                vm["turn-duration"].as<uint64_t>(),
                (uint16_t) vm["explosion-radius"].as<int>(),
                (uint16_t) vm["initial-blocks"].as<int>(),
                (uint16_t) vm["game-length"].as<int>(),
                vm["server-name"].as<std::string>(),
                (uint16_t) vm["port"].as<int>(),
                (uint32_t) vm["seed"].as<int>(),
                (uint16_t) vm["size-x"].as<int>(),
                (uint16_t) vm["size-y"].as<int>());

        boost::asio::io_context io_context;
        tcp::endpoint endpoint(tcp::v6(), server_params.get_port());

        Server server(io_context, endpoint, server_params);
    } catch (std::exception &err) {
        std::cerr << "Error: " << err.what() << '\n';
        exit(1);
    }

    return 0;
}
