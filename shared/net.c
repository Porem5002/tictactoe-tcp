#include <ws2tcpip.h>
#include <assert.h>

#include "net.h"

void net_start()
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
}

void net_finish()
{
    WSACleanup();
}

bool net_get_ip_fom_name(const char* name, IN_ADDR* out_ip)
{
    assert(out_ip != NULL);

    ADDRINFO hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    ADDRINFO* addrs;

    if(getaddrinfo(name, NULL, &hints, &addrs) != 0)
    {
        *out_ip = (IN_ADDR){0};
        return false;
    }

    *out_ip = ((struct sockaddr_in*)addrs->ai_addr)->sin_addr;
    freeaddrinfo(addrs);
    return true;
}

connection_t connection_init(SOCKET sock, bool nonblock)
{
    connection_t conn = {
        .sock = sock,
        .bytes_read = 0
    };

    DWORD imode = nonblock ? 1 : 0;
    ioctlsocket(conn.sock, FIONBIO, &imode);

    return conn;
}

connect_status_t connection_start_connect(connection_t* conn, struct sockaddr_in addr)
{
    if(connect(conn->sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        if(WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSAEINPROGRESS)
            return CONNECT_STATUS_FAILED;

        return CONNECT_STATUS_WAITING; 
    }

    return CONNECT_STATUS_SUCCESS;
}

connect_status_t connection_continue_connect(connection_t* conn)
{
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(conn->sock, &fds);

    struct timeval timeout = {0};
    int n_select = select(conn->sock, NULL, &fds, NULL, &timeout);

    if(n_select < 0) return CONNECT_STATUS_FAILED;

    if(n_select > 0)
    {
        int optval;
        int optlen = sizeof(optval);

        if(getsockopt(conn->sock, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen) == SOCKET_ERROR)
            return CONNECT_STATUS_FAILED;

        if(optval != 0)
            return CONNECT_STATUS_FAILED;

        return CONNECT_STATUS_SUCCESS;
    }

    return CONNECT_STATUS_WAITING;
}

void connection_close(connection_t* connection)
{
    if(connection->sock == INVALID_SOCKET) return;

    closesocket(connection->sock);
    memset(connection, 0, sizeof(*connection));
    connection->sock = INVALID_SOCKET;
}

packet_status_t packet_poll(connection_t* connection, packet_t* out_packet)
{
    int nbytes = recv(connection->sock, connection->data + connection->bytes_read, COMPACT_PACKET_SIZE - connection->bytes_read, 0);

    if(nbytes == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
        return PACKET_STATUS_WAITING;
    
    if(nbytes == SOCKET_ERROR || nbytes == 0)
        return PACKET_STATUS_NO_MORE;

    connection->bytes_read += nbytes;

    if(connection->bytes_read == COMPACT_PACKET_SIZE)
    {
        connection->bytes_read = 0;

        char* data = connection->data;
        
        packet_t p = {0};
        p.kind = data[0]; 

        switch(data[0])
        {
            case PACKET_KIND_RESPONSE_CONNECT:
                p.response_connect.assigned_player = data[1];
                break;
            case PACKET_KIND_RESPONSE_MOVE:
                p.response_move.player = data[1];
                p.response_move.x = data[2];
                p.response_move.y = data[3];
                break;
            case PACKET_KIND_RESPONSE_WINNER:
                p.response_winner.player = data[1];
                break;
            case PACKET_KIND_RESPONSE_RESET:
                break;
            case PACKET_KIND_REQUEST_MOVE:
                p.resquest_move.x = data[1];
                p.resquest_move.y = data[2];
                break;
            case PACKET_KIND_REQUEST_RESET:
                break;
            default:
                return PACKET_STATUS_WAITING;
        }

        *out_packet = p;
        return PACKET_STATUS_RECEIVED;
    }

    return PACKET_STATUS_WAITING;
}

bool packet_send(connection_t* connection, packet_t packet)
{
    char data [COMPACT_PACKET_SIZE] = {0};
    data[0] = packet.kind;

    switch(packet.kind)
    {
        case PACKET_KIND_RESPONSE_CONNECT:
            data[1] = packet.response_connect.assigned_player;
            break;
        case PACKET_KIND_RESPONSE_MOVE:
            data[1] = packet.response_move.player;
            data[2] = packet.response_move.x;
            data[3] = packet.response_move.y;
            break;
        case PACKET_KIND_RESPONSE_WINNER:
            data[1] = packet.response_winner.player;
            break;
        case PACKET_KIND_RESPONSE_RESET:
            break;
        case PACKET_KIND_REQUEST_MOVE:
            data[1] = packet.resquest_move.x;
            data[2] = packet.resquest_move.y;
            break;
        case PACKET_KIND_REQUEST_RESET:
            break;
        default:
            return false;
    }

    return send(connection->sock, data, sizeof(data), 0) != SOCKET_ERROR;
}
