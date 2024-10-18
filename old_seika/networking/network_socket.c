#include "network_socket.h"

#include <string.h>

#include "seika/logger.h"
#include "seika/assert.h"
#include "seika/string.h"
#include "seika/thread/pthread.h"

//--- RBE Socket ---//
static int ska_socket_get_last_error() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return -1;
#endif
}

bool ska_socket_system_initialize() {
#ifdef _WIN32
    // Initialise winsock
    WSADATA wsa;
    ska_logger_debug("Initialising client Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        ska_logger_error("Client: Failed. Error Code : %d", ska_socket_get_last_error());
        return false;
    }
    ska_logger_debug("Client: Initialized Winsock.");
#endif
    return true;
}

void ska_socket_system_finalize() {
#ifdef _WIN32
    WSACleanup();
#endif
}

bool ska_socket_send_message(SkaSocket* sock, const char* message) {
#define CRE_SOCKET_SEND_BUFFER_SIZE 512
    static char socket_output_buffer[CRE_SOCKET_SEND_BUFFER_SIZE];
    ska_strcpy(socket_output_buffer, message);
    if (sendto(sock->sock, socket_output_buffer, (int) strlen(socket_output_buffer), 0, (struct sockaddr*) &sock->si_other, sock->size) == SOCKET_ERROR) {
        ska_logger_error("sendto() failed with error code : %d", ska_socket_get_last_error());
        return false;
    }
    return true;
#undef CRE_SOCKET_SEND_BUFFER_SIZE
}

bool ska_socket_receive_data(SkaSocket* sock, char* buffer, int32 buffer_size) {
    if ((recvfrom(sock->sock, buffer, buffer_size, 0, (struct sockaddr*) &sock->si_other, &sock->size)) == SOCKET_ERROR) {
        ska_logger_error("recvfrom() failed with error code : %d", ska_socket_get_last_error());
        return false;
    }
    return true;
}

void ska_socket_close(SkaSocket* sock) {}

//--- Server Socker ---//
static pthread_t server_thread;

SkaSocket ska_socket_create_server(int32 port, ska_on_socket_receive_data callback_func) {
    SKA_ASSERT_FMT(callback_func, "Callback func is NULL");
    SkaSocket sock;
    sock.size = sizeof(sock.si_other);

    // Create a socket
    if ((sock.sock = socket(AF_INET, SOCK_DGRAM, 0 )) == INVALID_SOCKET) {
        SKA_ASSERT_FMT(false, "Server: could not create socket : %d", ska_socket_get_last_error());
    }
    ska_logger_debug("Server: socket created!");

    // Prepare the sockaddr_in structure
    sock.si.sin_family = AF_INET;
    sock.si.sin_addr.s_addr = INADDR_ANY;
    sock.si.sin_port = htons(port);

    // Bind
    if (bind(sock.sock, (struct sockaddr *) &sock.si, sizeof(sock.si)) == SOCKET_ERROR) {
        SKA_ASSERT_FMT(false, "Server: Bind failed with error code : %d", ska_socket_get_last_error());
    }
    ska_logger_debug("Server: Bind done!");

    // Start Networking Thread
    if (pthread_create(&server_thread, NULL, callback_func, NULL) != 0) {
        SKA_ASSERT_FMT(false, "Failed to create server thread!");
    }
    ska_logger_debug("Server: Initialized thread.");
    return sock;
}

//--- Client Socket ---//
static pthread_t client_thread;

SkaSocket ska_socket_create_client(const char* serverAddr, int32 serverPort, ska_on_socket_receive_data callback_func) {
    //create socket
    SkaSocket sock;
    sock.size = sizeof(sock.si_other);
    ska_logger_debug("Client: Creating socket....");
    if ((int) (sock.sock = (int) socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
        SKA_ASSERT_FMT(false, "Client: socket() failed with error code : %d", ska_socket_get_last_error());
    }
    ska_logger_debug("Client: socket created.");

    // setup address structure
    memset((char*) &sock.si_other, 0, sizeof(sock.si_other));
    sock.si_other.sin_family = AF_INET;
    sock.si_other.sin_port = htons(serverPort);
#ifdef _WIN32
    sock.si_other.sin_addr.S_un.S_addr = inet_addr(serverAddr);
#else
    if (inet_aton(serverAddr, &sock.si_other.sin_addr) == 0) {
        SKA_ASSERT_FMT(false, "inet_aton() failed!");
    }
#endif

    // Start Networking Thread
    if (pthread_create(&client_thread, NULL, callback_func, NULL) != 0) {
        SKA_ASSERT_FMT(false, "Failed to create server thread!");
    }

    ska_logger_debug("Client: Initialized thread.");
    return sock;
}
