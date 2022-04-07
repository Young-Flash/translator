#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "Connection.h"



class SCGI_Connection : public Connection{
public:

    /** set up connection to rTorrent */
    SCGI_Connection(const string&);

    virtual ~SCGI_Connection();

    /** send json data to rTorrent when type == 1, to websocket connection (peer) when type == 0*/
    bool send_to(json, int type) override;

    /** receive json data from rTorrent when type == 1, from websocket connection (peer) when type == 0 */
    json& receive_from(int type) override;

    void init();

    void close();

private:

    /** keep the file descriptor which indicates the connection to rTorrent */
    int m_sock;

    string m_target;

    friend class Websocket_Connection;
};



