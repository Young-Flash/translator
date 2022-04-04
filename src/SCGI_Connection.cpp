#include "SCGI_Connection.h"
#include "Websocket_Connection.h"

SCGI_Connection::SCGI_Connection(const string& target) {
    const auto &delimPos = target.find_first_of(':');

    int result = 0;
    int sock = 0;
    void *sa = nullptr;
    unsigned long sa_size = 0;
    if (delimPos != std::string::npos) {
        const std::string host(target.substr(0, delimPos));
        const std::string port(target.substr(delimPos + 1));

        sa_size = sizeof(::sockaddr_in);
        sa = ::calloc(1, sa_size);
        struct ::sockaddr_in *sin = (struct ::sockaddr_in *)sa;

        sin->sin_family = AF_INET;
        sin->sin_port = ::htons(std::stoi(port));

        result = ::inet_pton(AF_INET, host.c_str(), &sin->sin_addr);
        if (result <= 0) {
            std::cout << "Invalid address: " << result << std::endl;
            return;
        }

        sock = ::socket(AF_INET, SOCK_STREAM, 0);
    } else {
        sa_size = sizeof(sockaddr_un);
        sa = ::calloc(1, sa_size);
        struct sockaddr_un *sun = (struct sockaddr_un *)sa;

        sun->sun_family = AF_UNIX;

        if (target.size() > sizeof(sun->sun_path)) {
            std::cout << "Invalid path" << std::endl;
            return;
        }

        ::strncpy(sun->sun_path, target.data(), target.size());

        sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
    }

    if (sock < 0) {
        std::cout << "Failed to create socket: " << sock << std::endl;
        return;
    }

    result = ::connect(sock, (struct sockaddr *)sa, sa_size);
    if (result < 0) {
        std::cout << "Failed to connect: " << result << std::endl;
        ::free(sa);
        return;
    }
    else {
        std::cout << "Connected to rTorrent: " << target << std::endl;
    }

    ::free(sa);
    m_sock = sock;
}

bool SCGI_Connection::send_to(json command, int type) {

    // send to rtorrent
    if (type == 1) {
        const std::string payload_str = command.dump();

        std::vector<std::pair<std::string, std::string>> headers = {
                {"CONTENT_LENGTH", std::to_string(command.dump().length())},
                {"CONTENT_TYPE", "application/json"},
                {"SCGI", "1"}
        };
        unsigned long header_length = 0;
        for (const auto &header : headers) {
            // KEY\x00VALUE\x00
            header_length += header.first.length();
            header_length += 1;
            header_length += header.second.length();
            header_length += 1;
        }

        const auto &header_length_str = std::to_string(header_length) + ":";
        write(m_sock, header_length_str.c_str(), header_length_str.length());

        for (const auto &header : headers) {
            write(m_sock, header.first.c_str(), header.first.length());
            write(m_sock, "m_\x00", 1);
            write(m_sock, header.second.c_str(), header.second.length());
            write(m_sock, "\x00", 1);
        }

        write(m_sock, ",", 1);
        write(m_sock, payload_str.c_str(), payload_str.length());

        // receive from rtorrent
        receive_from(1);

        return true;
    }
    // send to peer
    else if (type == 0) {
        dynamic_cast<Websocket_Connection*>(m_peer.get())->data = command;
        m_peer->send_to(data, 1);
        return true;
    }

    return false;
}

json& SCGI_Connection::receive_from(int type) {

    // receive from rtorrent
    if (type == 1) {
        char *buf = static_cast<char *>(calloc(2048, sizeof(char)));
        std::string response_str;
        while (true) {
            int count = read(m_sock, buf, 2000);
            if (count <= 0) {
                break;
            }

            buf[count] = '\x00';
            response_str += buf;
        }
        free(buf);
        data = json::parse(response_str.substr(response_str.find_last_of('\n')));

        // send to peer
        send_to(data, 0);
    }
    // receive from peer
    else if (type == 0) {
        m_peer->send_to(data, 1);
    }
    return data;
}

SCGI_Connection::~SCGI_Connection() {
    if (m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
    }
}