#include "network.h"

#include <stdio.h>
#include <string.h>

#include "network_socket.h"
#include "seika/logger.h"
#include "seika/assert.h"

//--- NETWORK ---//
#define SKA_NETWORK_HANDSHAKE_MESSAGE "init"

static bool ska_is_server = false;

bool ska_network_is_server() {
    return ska_is_server;
}

//--- UDP SERVER ---//
#define SKA_SERVER_BUFFER_SIZE 512

static void* ska_udp_server_poll(void* arg);

static ska_on_network_server_callback server_user_callback = NULL;
static ska_on_network_server_client_connected_callback server_client_connected_callback = NULL;

static SkaSocket server_socket;

bool ska_udp_server_initialize(int32 port, ska_on_network_server_callback user_callback) {
    server_user_callback = user_callback;
    ska_socket_system_initialize();
    server_socket = ska_socket_create_server(port, ska_udp_server_poll);
    ska_is_server = true;

    return true;
}

void ska_udp_server_register_client_connected_callback(ska_on_network_server_client_connected_callback client_connected_callback) {
    server_client_connected_callback = client_connected_callback;
}

void* ska_udp_server_poll(void* arg) {
    static char server_input_buffer[SKA_SERVER_BUFFER_SIZE];
    //keep listening for data
    while (true) {
        fflush(stdout);

        //clear the buffer by filling null, it might have previously received data
        memset(server_input_buffer, '\0', SKA_SERVER_BUFFER_SIZE);
        //try to receive some data, this is a blocking call
        if (!ska_socket_receive_data(&server_socket, server_input_buffer, SKA_SERVER_BUFFER_SIZE)) {
            ska_logger_error("Failed to receive socket data with server!");
//            return NULL;
            continue;
        }

        // Process data from client
        if (strcmp(server_input_buffer, SKA_NETWORK_HANDSHAKE_MESSAGE) == 0) {
            if (server_client_connected_callback != NULL) {
                server_client_connected_callback();
            }
            ska_udp_server_send_message(SKA_NETWORK_HANDSHAKE_MESSAGE);
        } else {
            // Call user callback
            server_user_callback(server_input_buffer);
        }
    }
    return NULL;
}

bool ska_udp_server_send_message(const char* message) {
    if (!ska_socket_send_message(&server_socket, message)) {
        ska_logger_error("Failed to send message '%s'", message);
        return false;
    }
    return true;
}

void ska_udp_server_finalize() {
    ska_socket_close(&server_socket);
    ska_socket_system_finalize();
}

//--- UDP CLIENT ---//
#define SKA_CLIENT_BUFFER_SIZE 512

static void* ska_udp_client_poll(void*);

static SkaSocket client_socket;
static ska_on_network_client_callback client_user_callback = NULL;

bool ska_udp_client_initialize(const char* serverAddr, int32 serverPort, ska_on_network_client_callback user_callback) {
    client_user_callback = user_callback;
    ska_socket_system_initialize();
    client_socket = ska_socket_create_client(serverAddr, serverPort, ska_udp_client_poll);
    ska_logger_debug("Client: Initialized thread.");

    return true;
}

void* ska_udp_client_poll(void *unused) {
    static char client_input_buffer[SKA_CLIENT_BUFFER_SIZE];
    // TODO: Do handshake
    // TODO: Figure out why there is failure if no message is sent at first
    ska_socket_send_message(&client_socket, SKA_NETWORK_HANDSHAKE_MESSAGE);

    while (true) {
        fflush(stdout);
        //clear the buffer by filling null, it might have previously received data
        memset(client_input_buffer, '\0', SKA_CLIENT_BUFFER_SIZE);
        //try to receive some data, this is a blocking call
        if (!ska_socket_receive_data(&client_socket, client_input_buffer, SKA_CLIENT_BUFFER_SIZE)) {
            ska_logger_error("Failed to receive socket data from client!");
//            RBE_ASSERT(false);
            continue;
        }

        if (strcmp(client_input_buffer, SKA_NETWORK_HANDSHAKE_MESSAGE) == 0) {
            if (server_client_connected_callback != NULL) {
                server_client_connected_callback();
            }
        } else {
            // Call user callback
            client_user_callback(client_input_buffer);
        }
    }

    return NULL;
}

bool ska_udp_client_send_message(const char* message) {
    if (!ska_socket_send_message(&client_socket, message)) {
        ska_logger_error("Failed to send message '%s'", message);
        return false;
    }

    return true;
}

void ska_udp_client_finalize() {
    ska_socket_close(&client_socket);
    ska_socket_system_finalize();
}
