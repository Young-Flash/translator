#pragma once

#include <uWebSockets/App.h>

#include "Connection.h"

using namespace uWS;

/** user defined struct to store connection info for websocket */
struct ConnectionData {
    std::string_view address;
    ConnectionData() = default;
};

class Websocket_Connection : public Connection{
public:

    /** set up the websocket server */
    Websocket_Connection(const string&);

    virtual ~Websocket_Connection();

    /** send json data to websocket client when type == 1, to scgi connection (peer) when type == 0 */
    bool send_to(json, int type) override;

    /** receive json data from websocket client when type == 1, from scgi connection (peer) when type == 0 */
    json& receive_from(int type) override;

    /** start websocket connection */
    void run();

private:

    /** listening address, ip:port or unix domain socket (not implement now) */
    string m_target;

    string m_ip;

    int m_port;

    unique_ptr<SSLApp> m_sslApp;

    WebSocket<true, true, ConnectionData>* m_websocket;

    /** ssl config */
    SocketContextOptions m_socketContextOptions = {
        .key_file_name = "../misc/key.pem",
        .cert_file_name = "../misc/cert.pem",
        .passphrase = "1234"
    };

    friend class SCGI_Connection;
};