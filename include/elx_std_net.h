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

#ifndef ELX_STD_NET_H
#define ELX_STD_NET_H

#include "elx_std.h"
#include "elx_std_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ELX_NET_FAMILY_UNSPEC = 0,
    ELX_NET_FAMILY_IPV4 = 4,
    ELX_NET_FAMILY_IPV6 = 6,
} elx_net_family_t;

typedef enum {
    ELX_NET_SOCK_STREAM = 1,
    ELX_NET_SOCK_DGRAM = 2,
    ELX_NET_SOCK_RAW = 3,
} elx_net_sock_type_t;

typedef enum {
    ELX_NET_OK = 0,
    ELX_NET_ERR_FAILED,
    ELX_NET_ERR_TIMEOUT,
    ELX_NET_ERR_CONNECT_REFUSED,
    ELX_NET_ERR_DNS,
    ELX_NET_ERR_BIND,
    ELX_NET_ERR_LISTEN,
    ELX_NET_ERR_ACCEPT,
    ELX_NET_ERR_SEND,
    ELX_NET_ERR_RECV,
    ELX_NET_ERR_CLOSED,
} elx_net_result_t;

typedef struct elx_net_socket elx_net_socket_t;

elx_net_socket_t *elx_net_create_socket(elx_net_family_t family, elx_net_sock_type_t type);
void elx_net_socket_free(elx_net_socket_t *sock);
elx_net_result_t elx_net_connect(elx_net_socket_t *sock, const char *host, uint16_t port);
elx_net_result_t elx_net_bind(elx_net_socket_t *sock, const char *host, uint16_t port);
elx_net_result_t elx_net_listen(elx_net_socket_t *sock, int backlog);
elx_net_socket_t *elx_net_accept(elx_net_socket_t *listen_sock);
elx_net_result_t elx_net_send(elx_net_socket_t *sock, const void *data, size_t len, size_t *sent);
elx_net_result_t elx_net_recv(elx_net_socket_t *sock, void *buffer, size_t max_len, size_t *received);
elx_net_result_t elx_net_send_to(elx_net_socket_t *sock, const void *data, size_t len, const char *host, uint16_t port, size_t *sent);
elx_net_result_t elx_net_recv_from(elx_net_socket_t *sock, void *buffer, size_t max_len, char *src_host, size_t host_len, uint16_t *src_port, size_t *received);
elx_net_result_t elx_net_close(elx_net_socket_t *sock);
void elx_net_set_timeout(elx_net_socket_t *sock, uint32_t timeout_ms);
void elx_net_set_nodelay(elx_net_socket_t *sock, int enable);
char *elx_net_resolve_host(const char *hostname, elx_net_family_t family);
void elx_net_free_resolved(char *result);
const char *elx_net_version(void);

#ifdef __cplusplus
}
#endif

#endif
