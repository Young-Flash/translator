#include <nlohmann/json.hpp>

#include "SCGI_Connection.h"
#include "Websocket_Connection.h"

using nlohmann::json;

int main(int argc, char **argv) {

    string tcp = "127.0.0.1:1256";
    string unix_domain_socket = "/home/flash/.local/share/rtorrent/rtorrent.sock";

    Websocket_Connection connection_w_j("127.0.0.1:1258");
    SCGI_Connection connection_j_r(tcp);

    connection_w_j.set_peer(connection_j_r);
    connection_j_r.set_peer(connection_w_j);

    connection_w_j.run();

    return 0;
}