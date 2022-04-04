#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

using std::string;
using nlohmann::json;
using std::unique_ptr;

class Connection {
public:

    Connection() = default;
    virtual ~Connection() = default;

    virtual bool send_to(json, int) {
        return false;
    }

    virtual json& receive_from(int) {
        return data;
    }

    void set_peer(Connection& peer) {
        m_peer = unique_ptr<Connection>(&peer);
    }
protected:
    unique_ptr<Connection> m_peer;
    json data;
};