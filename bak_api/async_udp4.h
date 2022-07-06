/*
	Copyright (C) 2022 Brett Kuskie <fullaxx@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __ASYNC_UDP4_H__
#define __ASYNC_UDP4_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SA   struct sockaddr
#define SAI  struct sockaddr_in
#define SAI6 struct sockaddr_in6
#define SAU  struct sockaddr_un

#ifndef UDP_MAX_DATA
#define UDP_MAX_DATA	(65535)
#endif

typedef struct {
	int socket;
	int do_close;
	int closed;
} u4srv_t;

typedef struct {
	int socket;
	SAI addr;
	char ddip[INET_ADDRSTRLEN];		//dotted decimal ip
	unsigned short port;
	int do_close;
	int closed;
	int threaded;
} u4clnt_t;

#define U4_SRV_CALLBACK(CB) void (CB)(u4srv_t *, u4clnt_t *, char *, int, void *)
#define U4_CLNT_CALLBACK(CB) void (CB)(u4clnt_t *, char *, int, void *)

typedef struct {
	u4srv_t *s;
	U4_SRV_CALLBACK(*cb);
	void *user_data;
	unsigned int sleep_val;
} u4sparam_t;

typedef struct {
	u4clnt_t *c;
	U4_CLNT_CALLBACK(*cb);
	void *user_data;
	unsigned int sleep_val;
} u4cparam_t;

int as_udp4_bind_server(u4srv_t *srv, char *address, unsigned short port, void *func, void *user_data, unsigned int sleep_val);
int as_udp4_bind_mcast(u4srv_t *srv, char *address, unsigned short port, void *func, void *user_data, unsigned int sleep_val);
void as_udp4_server_halt(u4srv_t *srv);

int as_udp4_thread_conn(u4clnt_t *c, void *func, void *data, unsigned int sleep_val);
ssize_t as_udp4_client_write(u4clnt_t *c, void *data, int data_len);
int as_udp4_connect(u4clnt_t *c, char *address, unsigned short port);
void as_udp4_conn_halt(u4clnt_t *c);
char* as_udp4_c_getddip(u4clnt_t *c);
unsigned short as_udp4_c_getport(u4clnt_t *c);

#ifdef __cplusplus
}
#endif

#endif	/* __ASYNC_UDP4_H__ */
