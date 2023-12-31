#pragma once

#include <stdbool.h>

//--- NETWORK ---//
bool se_network_is_server();

//--- UDP SERVER ---//
typedef void (*on_network_server_callback) (const char*);
typedef void (*on_network_server_client_connected_callback) ();

bool se_udp_server_initialize(int port, on_network_server_callback user_callback);
void se_udp_server_register_client_connected_callback(on_network_server_client_connected_callback client_connected_callback);
bool se_udp_server_send_message(const char* message);
void se_udp_server_finalize();

//--- UDP CLIENT ---//
typedef void (*on_network_client_callback) (const char*);

bool se_udp_client_initialize(const char* serverAddr, int serverPort, on_network_client_callback user_callback);
bool se_udp_client_send_message(const char* message);
void se_udp_client_finalize();
