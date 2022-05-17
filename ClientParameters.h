#pragma once

#include <iostream>

class ClientParameters {
private:
    std::string _nickname;
    uint16_t _port;

    std::string _host_gui;
    uint16_t _port_gui;

    std::string _host_server;
    uint16_t _port_server;

public:
    ClientParameters() = default;
    ClientParameters(const std::string & nickname, uint16_t port,
                     const std::string & host_gui, uint16_t port_gui,
                     const std::string & host_server, uint16_t port_server) {
        _nickname = nickname;
        _port = port;
        _host_gui = host_gui;
        _port_gui = port_gui;
        _host_server = host_server;
        _port_server = port_server;
    }

    const std::string & get_nickname() const {
        return _nickname;
    }

    uint16_t get_port() const {
        return _port;
    }

    const std::string & get_host_gui() const {
        return _host_gui;
    }

    uint16_t get_port_gui() const {
        return _port_gui;
    }

    const std::string & get_host_server() const {
        return _host_server;
    }

    uint16_t get_port_server() const {
        return _port_server;
    }
};
