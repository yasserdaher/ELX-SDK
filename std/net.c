/*
 * Copyright (c) 2026 ELECTRON VISION.
 *
 * ELX SDK — ELECTRON VISION
 *
 * All rights reserved.
 *
 * Licensed under the VOLT 1.26 License Agreement.
 * See the LICENSE file for the applicable terms.
 */

#include "elx_std_net.h"
#include "elx_std_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *elx_net_version(void) { return "0.1.0"; }

struct elx_net_socket {
    int fd;
    elx_net_family_t family;
    elx_net_sock_type_t type;
    bool connected;
    uint32_t timeout_ms;
};

elx_net_socket_t *elx_net_create_socket(elx_net_family_t family, elx_net_sock_type_t type) {
    (void)family; (void)type;
    fprintf(stderr, "[elx_net] sockets not available on this platform\n");
    return NULL;
}

void elx_net_socket_free(elx_net_socket_t *sock) {
    if (!sock) return;
    elx_net_close(sock);
    free(sock);
}

elx_net_result_t elx_net_connect(elx_net_socket_t *sock, const char *host, uint16_t port) {
    (void)sock; (void)host; (void)port;
    return ELX_NET_ERR_FAILED;
}

elx_net_result_t elx_net_bind(elx_net_socket_t *sock, const char *host, uint16_t port) {
    (void)sock; (void)host; (void)port;
    return ELX_NET_ERR_FAILED;
}

elx_net_result_t elx_net_listen(elx_net_socket_t *sock, int backlog) {
    (void)sock; (void)backlog;
    return ELX_NET_ERR_FAILED;
}

elx_net_socket_t *elx_net_accept(elx_net_socket_t *listen_sock) {
    (void)listen_sock;
    return NULL;
}

elx_net_result_t elx_net_send(elx_net_socket_t *sock, const void *data, size_t len, size_t *sent) {
    (void)sock; (void)data; (void)len;
    if (sent) *sent = 0;
    return ELX_NET_ERR_FAILED;
}

elx_net_result_t elx_net_recv(elx_net_socket_t *sock, void *buffer, size_t max_len, size_t *received) {
    (void)sock; (void)buffer; (void)max_len;
    if (received) *received = 0;
    return ELX_NET_ERR_FAILED;
}

elx_net_result_t elx_net_send_to(elx_net_socket_t *sock, const void *data, size_t len, const char *host, uint16_t port, size_t *sent) {
    (void)sock; (void)data; (void)len; (void)host; (void)port;
    if (sent) *sent = 0;
    return ELX_NET_ERR_FAILED;
}

elx_net_result_t elx_net_recv_from(elx_net_socket_t *sock, void *buffer, size_t max_len, char *src_host, size_t host_len, uint16_t *src_port, size_t *received) {
    (void)sock; (void)buffer; (void)max_len; (void)src_host; (void)host_len; (void)src_port;
    if (received) *received = 0;
    return ELX_NET_ERR_FAILED;
}

elx_net_result_t elx_net_close(elx_net_socket_t *sock) {
    if (!sock) return ELX_NET_ERR_FAILED;
    sock->connected = false;
    sock->fd = -1;
    return ELX_NET_OK;
}

void elx_net_set_timeout(elx_net_socket_t *sock, uint32_t timeout_ms) {
    if (sock) sock->timeout_ms = timeout_ms;
}

void elx_net_set_nodelay(elx_net_socket_t *sock, int enable) { (void)sock; (void)enable; }

char *elx_net_resolve_host(const char *hostname, elx_net_family_t family) {
    (void)hostname; (void)family;
    return NULL;
}

void elx_net_free_resolved(char *result) { free(result); }
