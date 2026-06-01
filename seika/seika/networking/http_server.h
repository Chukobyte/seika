#pragma once

#if SKA_HTTP_SERVER

#include <stdbool.h>
#include "seika/defines.h"

typedef struct SkaHttpServerConfig {
    const char* root_dir;
    i32 port;
} SkaHttpServerConfig;

// Blocks indefinitely, serving static files from root_dir over HTTP.
void ska_http_server_serve(SkaHttpServerConfig config);

#endif // SKA_HTTP_SERVER
