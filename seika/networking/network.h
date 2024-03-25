#pragma once

#include <stdbool.h>

#include "seika/defines.h"

//--- NETWORK ---//
bool ska_network_is_server();

//--- UDP SERVER ---//
typedef void (*ska_on_network_server_callback) (const char*);
typedef void (*ska_on_network_server_client_connected_callback) ();

bool ska_udp_server_initialize(int32 port, ska_on_network_server_callback user_callback);
void ska_udp_server_register_client_connected_callback(ska_on_network_server_client_connected_callback client_connected_callback);
bool ska_udp_server_send_message(const char* message);
void ska_udp_server_finalize();

//--- UDP CLIENT ---//
typedef void (*ska_on_network_client_callback) (const char*);

bool ska_udp_client_initialize(const char* serverAddr, int32 serverPort, ska_on_network_client_callback user_callback);
bool ska_udp_client_send_message(const char* message);
void ska_udp_client_finalize();
