#include "Websocket_Connection.h"
#include "SCGI_Connection.h"

using std::cout;

Websocket_Connection::Websocket_Connection(const string& target) {
    m_target = target;
    const auto &delimPos = m_target.find_first_of(':');
    if (delimPos != std::string::npos) {
        m_ip = m_target.substr(0, delimPos);
        m_port = std::stoi(m_target.substr(delimPos + 1));
    }
    else {
        throw "unix domain socket for websocket have not implemented, try ip:port instead";
    }


    SSLApp sslApp = SSLApp(m_socketContextOptions).ws("/*", SSLApp::WebSocketBehavior<ConnectionData> {
        .idleTimeout = 300,
        .upgrade = [](HttpResponse<true>* res, HttpRequest* req, auto* context) {
            res->template upgrade<ConnectionData>(
                ConnectionData{
                    // store client address in ConnectionData (one ConnectionData per client)
                    .address = res->getRemoteAddressAsText()
                },
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context
            );
        },
        .open = [&](WebSocket<true, true, ConnectionData>* ws) {
            std::cout << "Websocket client " << ws->getUserData()->address << " connected!" << std::endl;
        },
        .message = [&](WebSocket<true, true, ConnectionData>* ws, std::string_view message, OpCode opCode) {
            m_websocket = ws;
            data = json::parse(message);
            std::cout << "<<<<<<< REQUEST" << std::endl;
            std::cout << data.dump(2) << std::endl;
            std::cout << "=======" << std::endl;

            dynamic_cast<SCGI_Connection*>(m_peer.get())->init();
            receive_from(1);
            dynamic_cast<SCGI_Connection*>(m_peer.get())->close();
        }
    }).listen(m_ip, m_port, [&](us_listen_socket_t* listen_socket) {
        if (listen_socket) {
            std::cout << "Listening websocket on " << m_ip << ":" << m_port << std::endl;
        } else std::cout << "Listen failed on " << m_ip << ":" << m_port << std::endl;
    });

    m_sslApp = std::make_unique<SSLApp>(std::move(sslApp));
}

Websocket_Connection::~Websocket_Connection() = default;

bool Websocket_Connection::send_to(json command, int type) {
    // send to websocket client
    if (type == 1) {
        m_websocket->send(data.dump(), OpCode::TEXT);
        std::cout << data.dump(2) << std::endl;
        std::cout << ">>>>>>> RESPONSE" << std::endl;
        return true;
    }
    // send to peer
    else if (type == 0) {
        dynamic_cast<SCGI_Connection*>(m_peer.get())->data = command;
        m_peer->send_to(command, 1);
        return true;
    }
    return false;
}

json& Websocket_Connection::receive_from(int type) {

    // receive from websocket client
    if (type == 1) {
        send_to(data, 0);
    }
    // receive from peer
    else if (type == 0) {
        send_to(data, 1);
    }
    return data;
}

void Websocket_Connection::run() {
    m_sslApp->run();
}