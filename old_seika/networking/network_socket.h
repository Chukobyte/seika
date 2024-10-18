#pragma once

#include <stdbool.h>

#include "seika/defines.h"

#ifdef _WIN32
#include <WinSock2.h>

typedef int32 se_socket_size;
typedef SOCKET ska_socket;
#else
#include<arpa/inet.h>
#include<sys/socket.h>

#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (-1)

typedef socklen_t se_socket_size;
typedef int ska_socket;
#endif

//--- RBE Socket---//
typedef void* (*ska_on_socket_receive_data) (void*);

typedef struct SkaSocket {
    ska_socket sock;
    se_socket_size size;
    struct sockaddr_in si;
    struct sockaddr_in si_other;
} SkaSocket;

bool ska_socket_system_initialize();
void ska_socket_system_finalize();
bool ska_socket_send_message(SkaSocket* sock, const char* message);
bool ska_socket_receive_data(SkaSocket* sock, char* buffer, int32 buffer_size);
void ska_socket_close(SkaSocket* sock);

//--- Server Socket ---//
SkaSocket ska_socket_create_server(int32 port, ska_on_socket_receive_data callback_func);

//--- Client Socket ---//
SkaSocket ska_socket_create_client(const char* serverAddr, int32 serverPort, ska_on_socket_receive_data callback_func);
