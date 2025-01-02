#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <winsock2.h>

#include "server.h"

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "ERROR: Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    char* endp;
    long port = strtol(argv[1], &endp, 10);
    if(port < 0 || *endp != '\0')
    {
        fprintf(stderr, "ERROR: Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    net_start();

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET)
    {
        fprintf(stderr, "ERROR: Could not create socket!\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons((u_short)port);

    if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: Could not bind socket!\n");
        exit(EXIT_FAILURE);
    }

    if(listen(sock, 10) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: Could not start listenning!\n");
        exit(EXIT_FAILURE);
    }

    server_t server = server_make(sock);

    while(true)
    {
        server_wait_for_action(&server);
        server_accept_client(&server);
        server_update_rooms(&server);
    }

    server_free(&server);
    net_finish();
    return 0;
}