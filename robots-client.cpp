#include <cstdint>
#include <sstream>
#include <unistd.h>

#include "Client.h"

namespace po = boost::program_options;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

static void check_port(int port) {
    if (port < 0 || port > 65535) {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

static void check_address(const std::string &address) {
    size_t colon_idx = address.size() - 1;
    while (colon_idx > 0 && address[colon_idx] != ':')
        colon_idx--;

    if (colon_idx <= 0) {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }

    std::string host = address.substr(0, colon_idx);
    std::string port = address.substr(colon_idx + 1);

    std::stringstream parser(port);
    int port_num = 0;

    if (parser >> port_num) {
        check_port(port_num);
    } else {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }

    if (host.find(':') != std::string::npos) {
        boost::system::error_code ec;
        boost::asio::ip::address::from_string(host, ec);
        if (ec) {
            throw po::validation_error(
                    po::validation_error::invalid_option_value);
        }
    }
}

static std::string get_host_from_address(const std::string &address) {
    size_t colon_idx = address.size() - 1;
    while (colon_idx > 0 && address[colon_idx] != ':')
        colon_idx--;

    return address.substr(0, colon_idx);
}

static uint16_t get_port_from_address(const std::string &address) {
    size_t colon_idx = address.size() - 1;
    while (colon_idx > 0 && address[colon_idx] != ':')
        colon_idx--;

    return (uint16_t) std::strtoul(address.substr(colon_idx + 1).c_str(), NULL,
                                   10);
}

int main(int argc, char *argv[]) {
    ClientParameters clientParams;

    try {
        int port_init;
        std::string nick_init, gui_addr_init, ser_addr_init;

        po::options_description desc("Allowed options");
        desc.add_options()
                ("gui-address,d",
                 po::value<std::string>(&gui_addr_init)->required()->notifier(
                         &check_address),
                 "gui address")
                ("help,h", "produce help message")
                ("player-name,n",
                 po::value<std::string>(&nick_init)->required(),
                 "player name")
                ("port,p",
                 po::value<int>(&port_init)->required()->notifier(&check_port),
                 "port number")
                ("server-address,s",
                 po::value<std::string>(&ser_addr_init)->required()->notifier(
                         &check_address),
                 "server address");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << "Usage: " << argv[0] << " [options]\n" << desc << '\n';
            return 1;
        }
        po::notify(vm);

        clientParams = ClientParameters(vm["player-name"].as<std::string>(),
                                        (uint16_t) vm["port"].as<int>(),
                                        get_host_from_address(
                                                vm["gui-address"].as<std::string>()),
                                        get_port_from_address(
                                                vm["gui-address"].as<std::string>()),
                                        get_host_from_address(
                                                vm["server-address"].as<std::string>()),
                                        get_port_from_address(
                                                vm["server-address"].as<std::string>()));

        boost::asio::io_context io_context;

        tcp::resolver resolver_server(io_context);
        tcp::resolver::results_type endpoint_server = resolver_server.resolve(
                clientParams.get_host_server(),
                std::to_string(clientParams.get_port_server()));

        udp::resolver resolver_gui_send(io_context);
        udp::endpoint endpoint_gui_send = *resolver_gui_send.resolve(
                clientParams.get_host_gui(),
                std::to_string(clientParams.get_port_gui())).begin();

        Client client(io_context, endpoint_server, endpoint_gui_send,
                      clientParams);
    } catch (std::exception &err) {
        std::cerr << "Error: " << err.what() << '\n';
        exit(42);
    }

    return 0;
}
