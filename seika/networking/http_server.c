#if SKA_HTTP_SERVER

#include "http_server.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "seika/defines.h"
#include "seika/logger.h"

#ifdef PLATFORM_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>
   typedef SOCKET ska_tcp_sock_t;
#  define SKA_TCP_INVALID INVALID_SOCKET
#  define ska_tcp_close(s) closesocket(s)
#  define ska_tcp_send(s, b, n) send((s), (b), (int)(n), 0)
#  define ska_tcp_recv(s, b, n) recv((s), (b), (int)(n), 0)
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
   typedef int ska_tcp_sock_t;
#  define SKA_TCP_INVALID (-1)
#  define ska_tcp_close(s) close(s)
#  define ska_tcp_send(s, b, n) send((s), (b), (n), 0)
#  define ska_tcp_recv(s, b, n) recv((s), (b), (n), 0)
#endif

#define HTTP_RECV_BUF 8192
#define HTTP_PATH_MAX 2048
#define HTTP_HDR_MAX 512

// Sniff image MIME type from magic bytes so misnamed files (e.g. JPEG saved as .png) work.
static const char* mime_from_magic(const char* filepath) {
    unsigned char buf[12] = {0};
    FILE* f = fopen(filepath, "rb");
    if (!f) return NULL;
    usize n = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (n < 4) return NULL;
    if (buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF) return "image/jpeg";
    if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47) return "image/png";
    if (buf[0] == 0x47 && buf[1] == 0x49 && buf[2] == 0x46) return "image/gif";
    if (n >= 12 && buf[0] == 0x52 && buf[1] == 0x49 && buf[2] == 0x46 && buf[3] == 0x46 &&
        buf[8] == 0x57 && buf[9] == 0x45 && buf[10] == 0x42 && buf[11] == 0x50) return "image/webp";
    return NULL;
}

static const char* mime_from_path(const char* path) {
    const char* dot = strrchr(path, '.');
    if (!dot) return "application/octet-stream";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) return "text/html; charset=utf-8";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".js") == 0) return "application/javascript";
    if (strcmp(dot, ".json") == 0) return "application/json";
    if (strcmp(dot, ".png") == 0 || strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0 ||
        strcmp(dot, ".gif") == 0 || strcmp(dot, ".webp") == 0 || strcmp(dot, ".ico") == 0) {
        // Use magic-byte sniffing so files with wrong extensions serve correctly
        return NULL; // caller will sniff from the file
    }
    if (strcmp(dot, ".svg") == 0) return "image/svg+xml";
    if (strcmp(dot, ".woff") == 0) return "font/woff";
    if (strcmp(dot, ".woff2") == 0) return "font/woff2";
    if (strcmp(dot, ".txt") == 0) return "text/plain";
    return "application/octet-stream";
}

static void url_decode(char* out, const char* in, usize max) {
    usize i = 0, o = 0;
    while (in[i] && o + 1 < max) {
        if (in[i] == '%' && in[i + 1] && in[i + 2]) {
            char hex[3] = { in[i + 1], in[i + 2], 0 };
            out[o++] = (char)strtol(hex, NULL, 16);
            i += 3;
        } else {
            out[o++] = (in[i] == '+') ? ' ' : in[i];
            i++;
        }
    }
    out[o] = '\0';
}

static void send_404(ska_tcp_sock_t conn) {
    const char* r = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\nConnection: close\r\n\r\nNot Found";
    ska_tcp_send(conn, r, strlen(r));
}

static void send_file(ska_tcp_sock_t conn, const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) { send_404(conn); return; }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Prefer extension-based MIME; fall back to magic-byte sniff for images
    const char* mime = mime_from_path(filepath);
    if (!mime) mime = mime_from_magic(filepath);
    if (!mime) mime = "application/octet-stream";

    char hdr[HTTP_HDR_MAX];
    int hlen = snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n",
        mime, fsize);
    ska_tcp_send(conn, hdr, (usize)hlen);

    char buf[4096];
    usize n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        ska_tcp_send(conn, buf, n);
    }
    fclose(f);
}

static void handle_connection(ska_tcp_sock_t conn, const char* root_dir) {
    char req[HTTP_RECV_BUF];
    int received = (int)ska_tcp_recv(conn, req, sizeof(req) - 1);
    if (received <= 0) { ska_tcp_close(conn); return; }
    req[received] = '\0';

    if (strncmp(req, "GET ", 4) != 0) {
        const char* r = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        ska_tcp_send(conn, r, strlen(r));
        ska_tcp_close(conn);
        return;
    }

    char* path_start = req + 4;
    char* path_end = strchr(path_start, ' ');
    if (!path_end) { ska_tcp_close(conn); return; }
    *path_end = '\0';

    char* qs = strchr(path_start, '?');
    if (qs) *qs = '\0';

    char decoded[HTTP_PATH_MAX];
    url_decode(decoded, path_start, sizeof(decoded));

    if (strstr(decoded, "..")) {
        const char* r = "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        ska_tcp_send(conn, r, strlen(r));
        ska_tcp_close(conn);
        return;
    }

    char filepath[HTTP_PATH_MAX * 2];
    if (decoded[0] == '/') {
        snprintf(filepath, sizeof(filepath), "%s%s", root_dir, decoded);
    } else {
        snprintf(filepath, sizeof(filepath), "%s/%s", root_dir, decoded);
    }

    usize fplen = strlen(filepath);
    if (filepath[fplen - 1] == '/') {
        char idx[HTTP_PATH_MAX * 2 + 12];
        snprintf(idx, sizeof(idx), "%sindex.html", filepath);
        send_file(conn, idx);
    } else {
        // Try exact path; if not a file try appending /index.html
        FILE* f = fopen(filepath, "rb");
        if (f) {
            fclose(f);
            send_file(conn, filepath);
        } else {
            char idx[HTTP_PATH_MAX * 2 + 12];
            snprintf(idx, sizeof(idx), "%s/index.html", filepath);
            send_file(conn, idx);
        }
    }

    ska_tcp_close(conn);
}

void ska_http_server_serve(SkaHttpServerConfig config) {
#ifdef PLATFORM_WINDOWS
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        ska_logger_error("HTTP server: WSAStartup failed");
        return;
    }
#endif

    ska_tcp_sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == SKA_TCP_INVALID) {
        ska_logger_error("HTTP server: socket creation failed");
        goto wsa_cleanup;
    }

    {
        int opt = 1;
        setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    }

    {
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons((unsigned short)config.port);

        if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            ska_logger_error("HTTP server: bind failed on port %d", config.port);
            ska_tcp_close(srv);
            goto wsa_cleanup;
        }
    }

    listen(srv, 10);
    ska_logger_message("Serving '%s' at http://localhost:%d  (Ctrl+C to stop)", config.root_dir, config.port);

    for (;;) {
        struct sockaddr_in client_addr = {0};
        int client_len = (int)sizeof(client_addr);
        ska_tcp_sock_t conn = accept(srv, (struct sockaddr*)&client_addr, &client_len);
        if (conn == SKA_TCP_INVALID) continue;
        handle_connection(conn, config.root_dir);
    }

wsa_cleanup:;
#ifdef PLATFORM_WINDOWS
    WSACleanup();
#endif
}

#endif // SKA_HTTP_SERVER
