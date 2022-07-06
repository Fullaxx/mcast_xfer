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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <netdb.h>			// gethostbyname()
#include <sched.h>

#include "async_udp4.h"

static inline void idlehands(useconds_t usec)
{
#ifdef YIELDFORSPEED
	sched_yield();
#else
	usleep(usec);
#endif
}

static void* udp4_listen(void *param)
{
	u4sparam_t *p = (u4sparam_t *)param;
	u4srv_t *s = p->s;
	u4clnt_t c;
	socklen_t c_len;
	ssize_t r, flags;
	char *buf = calloc(1, UDP_MAX_DATA);

	prctl(PR_SET_NAME, "a_udp4_srv", 0, 0, 0);

	//set non-blocking
	if ((flags = fcntl(s->socket, F_GETFL, 0)) >= 0) {
		fcntl(s->socket, F_SETFL, flags | O_NONBLOCK);
	}

	c.socket = s->socket;
	c_len = sizeof(c.addr);
	// we repeat until we are asked to close
	while(!s->do_close) {
		r = recvfrom(s->socket, buf, UDP_MAX_DATA, 0, (SA *)&c.addr, &c_len);
		if(r <= 0) {
			if((r == -1) && (errno == EWOULDBLOCK)) { idlehands(p->sleep_val); }
			else { s->do_close = 1; }
		} else {
			inet_ntop(AF_INET, &c.addr.sin_addr, c.ddip, INET_ADDRSTRLEN);
			c.port = ntohs(c.addr.sin_port);
			p->cb(s, &c, buf, r, p->user_data);
			memset(buf, 0, UDP_MAX_DATA);
		}
	}

#ifdef DEBUG
	if((r < 0) && (errno != EWOULDBLOCK)) { fprintf(stderr, "error: recvfrom() [udp4_listen()] (errno: %i) %s\n", errno, strerror(errno)); }
#endif
	close(s->socket);
	if(buf) { free(buf); }
	p->cb(s, NULL, NULL, 0, p->user_data);
	s->closed = 1;
	free(p);
#ifdef DEBUG
	printf("udp4_listen() EXIT\n");
#endif
	return NULL;
}

int as_udp4_bind_server(u4srv_t *srv, char *address, unsigned short port, void *func, void *user_data, unsigned int sleep_val)
{
	u4sparam_t *p;
	pthread_t thr_id;
	SAI server;
	int err;

	srv->socket = socket(PF_INET, SOCK_DGRAM, 0);
	if(srv->socket < 0) {
		fprintf(stderr, "error: socket()\n");
		perror("socket");
		return -1;
	}

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if(!address) {
		server.sin_addr.s_addr = INADDR_ANY;
	} else {
		inet_pton(AF_INET, address, &server.sin_addr);
	}

	err = bind(srv->socket, (SA *)&server, sizeof(server));
	if (err < 0) {
		fprintf(stderr, "error: bind()\n");
		perror("bind");
		return -2;
	}

	p = (u4sparam_t *)calloc(1, sizeof(u4sparam_t));
	p->s = srv;
	p->cb = func;
	p->user_data = user_data;
	p->sleep_val = sleep_val;

	err = pthread_create(&thr_id, NULL, &udp4_listen, p);
	if(err) { free(p); close(srv->socket); return -3; }
	err = pthread_detach(thr_id);
	if(err) { free(p); close(srv->socket); return -4; }

	return 0;
}

int as_udp4_bind_mcast(u4srv_t *srv, char *address, unsigned short port, void *func, void *user_data, unsigned int sleep_val)
{
	u4sparam_t *p;
	pthread_t thr_id;
	SAI server;
	struct ip_mreq mreq;
	int err, on;

	if(!srv || !address || !func) return -1;
	memset(srv, 0, sizeof(u4srv_t));
	memset(&mreq, 0, sizeof(mreq));

	srv->socket = socket(PF_INET, SOCK_DGRAM, 0);
	if(srv->socket < 0) {
		fprintf(stderr, "error: socket()\n");
		perror("socket");
		return -2;
	}

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	on = 1;
	err = setsockopt(srv->socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (err < 0) {
		fprintf(stderr, "error: setsockopt(SO_REUSEADDR)\n");
		perror("setsockopt");
		return -3;
	}

	err = bind(srv->socket, (SA *)&server, sizeof(server));
	if (err < 0) {
		fprintf(stderr, "error: bind()\n");
		perror("bind");
		return -4;
	}

	inet_pton(AF_INET, address, &mreq.imr_multiaddr.s_addr);
	mreq.imr_interface.s_addr = INADDR_ANY;
	err = setsockopt(srv->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq));
	if (err < 0) {
		fprintf(stderr, "error: setsockopt(IP_ADD_MEMBERSHIP)\n");
		perror("setsockopt");
		return -5;
	}

	p = (u4sparam_t *)calloc(1, sizeof(u4sparam_t));
	p->s = srv;
	p->cb = func;
	p->user_data = user_data;
	p->sleep_val = sleep_val;

	err = pthread_create(&thr_id, NULL, &udp4_listen, p);
	if(err) { free(p); close(srv->socket); return -6; }
	err = pthread_detach(thr_id);
	if(err) { free(p); close(srv->socket); return -7; }

	return 0;
}

void as_udp4_server_halt(u4srv_t *srv)
{
	if(srv->socket > 0) {
		srv->do_close = 1;
		while(!srv->closed) { usleep(1000); }
	}
	srv->socket = 0;
}

static void* udp4_conn_thread(void *param)
{
	int flags;
	ssize_t r;
	u4cparam_t *p = (u4cparam_t *)param;
	u4clnt_t *c = p->c;
	char *buf = calloc(1, UDP_MAX_DATA);

	prctl(PR_SET_NAME, "a_udp_conn", 0, 0, 0);

	//set non-blocking
	if((flags = fcntl(c->socket, F_GETFL, 0)) >= 0) {
		fcntl(c->socket, F_SETFL, flags | O_NONBLOCK);
		c->threaded = 1;
	}

	// we repeat until we are asked to close
	while (!c->do_close) {
		r = recvfrom(c->socket, buf, UDP_MAX_DATA, 0, NULL, NULL);
		if(r <= 0) {
			if((r == -1) && (errno == EWOULDBLOCK)) { idlehands(p->sleep_val); }
			else { c->do_close = 1; }
		} else {
			p->cb(c, buf, r, p->user_data);
			memset(buf, 0, UDP_MAX_DATA);
		}
	}

#ifdef DEBUG
	if((r < 0) && (errno != EWOULDBLOCK)) { fprintf(stderr, "error: recvfrom() [udp4_conn_thread()] (errno: %i) %s\n", errno, strerror(errno)); }
#endif
	close(c->socket);
	if(buf) { free(buf); }
	p->cb(c, NULL, 0, p->user_data);
	c->closed = 1;
	free(p);
#ifdef DEBUG
	printf("udp4_conn_thread() EXIT\n");
#endif
	return (NULL);
}

int as_udp4_thread_conn(u4clnt_t *c, void *func, void *data, unsigned int sleep_val)
{
	int z;
	pthread_t thr_id;
	u4cparam_t *p = (u4cparam_t *)calloc(1, sizeof(u4cparam_t));

	p->c = c;
	p->cb = func;
	p->user_data = data;
	p->sleep_val = sleep_val;

	z = pthread_create(&thr_id, NULL, &udp4_conn_thread, (void *)p);
	if(z) { free(p); return -1; }
	z = pthread_detach(thr_id);
	if(z) { free(p); return -2; }
	return 0;
}

ssize_t as_udp4_client_write(u4clnt_t *c, void *data, int data_len)
{
	ssize_t z;
	int retry;

	if(!c) { return 0; }

	do {
		retry = 0;
		z = sendto(c->socket, data, data_len, 0, (SA *)&c->addr, sizeof(c->addr));
		if(z < 0) {
			if(errno == EWOULDBLOCK) { sched_yield(); retry = 1; }
			else { fprintf(stderr, "%s(): (errno: %i) %s\n", __func__, errno, strerror(errno)); }
		}
	} while(retry);

	return z;
}

int as_udp4_connect(u4clnt_t *c, char *address, unsigned short port)
{
	struct hostent *host = NULL;
	
	c->socket = socket(PF_INET, SOCK_DGRAM, 0);
	if(c->socket < 0) {
		fprintf(stderr, "error: socket() [as_udp4_client_write()] (errno: %i)\n", errno);
		perror("socket");
		return -1;
	}
	
	// The gethostbyname*() and gethostbyaddr*() functions are obsolete.
	// Applications should use  getaddrinfo(3) and getnameinfo(3) instead.
	if (!(host = gethostbyname(address))) {
		fprintf(stderr, "gethostbyname(): %s could not be resolved\n", address);
		perror("gethostbyname");
		return -2;
	}
	
	memset(&c->addr, 0, sizeof(c->addr));
	c->addr.sin_family = AF_INET;
	c->addr.sin_port = htons(port);
	//c->addr.sin_addr = host->h_addr;
	memcpy(&c->addr.sin_addr, host->h_addr, sizeof(c->addr.sin_addr));
	
/*
	if (inet_pton(AF_INET, address, &c->addr.sin_addr) < 1) {
		fprintf(stderr, "error: inet_pton()\n");
		perror("inet_pton");
		exit(-1);
	}
*/
	return 0;
}

void as_udp4_conn_halt(u4clnt_t *c)
{
	if(c->socket > 0) {
		if(c->threaded) {
			// Inform the thread that we are all done
			c->do_close = 1;
			while(!c->closed) { usleep(1000); }
		} else {
			close(c->socket);
		}
	}
	c->socket = 0;
}

char* as_udp4_c_getddip(u4clnt_t *c)
{
	return c->ddip;
}

unsigned short as_udp4_c_getport(u4clnt_t *c)
{
	return c->port;
}
