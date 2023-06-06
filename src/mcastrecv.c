#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "async_udp4.h"
#include "futils.h"
#include "cryptohash.h"

u4srv_t g_srv;
int g_sigcaught = 0;
int g_shutdown = 0;
char *g_ip = "224.111.111.111";
unsigned short g_srv_port = 11111;
unsigned short g_conn_port = 0;
char g_fname_final[2048];
char g_fname_temp[2048];
FILE *g_fh = NULL;
chash_t *g_ch = NULL;

int g_chunksize = 0;
long g_last = 0;

int g_bail_on_miss = 0;
int g_miss = 0;

static void sig_handler(int signum)
{
	switch(signum) {
		case SIGHUP:
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			g_sigcaught = 1;
			g_shutdown = 1;
			break;
	}
}

static void file_beg(char *buf, ssize_t len)
{
	char *space1, *space2;
	int exists;
	long filesize;

	if((g_fh) || (g_ch)) { return; }

	space1 = strchr(buf, ' ');
	if(!space1) {
		fprintf(stderr, "ERROR FORMATTING ERROR!\n");
		exit(1);
	}

	space2 = strchr(space1+1, ' ');
	if(!space2) {
		fprintf(stderr, "ERROR FORMATTING ERROR!\n");
		exit(1);
	}

	*space1 = 0;
	*space2 = 0;
	snprintf(g_fname_final, sizeof(g_fname_final), "%s", buf);
	snprintf(g_fname_temp, sizeof(g_fname_temp), "%s.mpart", buf);
	filesize = atol(space1+1);
	g_chunksize = atoi(space2+1);

	// If file already exists, bail
	exists = is_regfile(g_fname_final, 0);
	if(exists == 1) {
		printf("Cowardly refusing to overwrite %s\n", g_fname_final);
		exit(1);
	}

	g_fh = fopen(g_fname_temp, "w");
	if(!g_fh) {
		fprintf(stderr, "fopen(%s, w)\n", g_fname_temp);
		exit(1);
	}

	// Start a hash object
	g_ch = chash_new(ALG_SHA512);
	printf("Started RECV of %s (%ld MB) ...\n", g_fname_final, filesize/(long)1e6);
}

static void bail_early(void)
{
	fprintf(stderr, "Bailing early due to missed chunks!\n");
	fclose(g_fh);
	g_fh = NULL;
	remove(g_fname_temp);
	chash_del(g_ch);
	g_ch = NULL;
	g_shutdown = 1;
	g_miss = 1;
}

static void file_mid(char *buf, ssize_t len)
{
	long *noffset;
	long offset;
	size_t written;

	if((!g_fh) || (!g_ch)) { return; }

	noffset = (long *)buf;
	offset = ntohl(*noffset);
	buf += 8; len -= 8;

	if(offset > 0) {
		//if(offset - g_last != g_chunksize) { printf("%ld - %ld != %d\n", offset, g_last, g_chunksize); fflush(stdout); }
		//if(offset - g_last != g_chunksize) { printf("Chunk Diff %ld > %d\n", offset - g_last, g_chunksize); fflush(stdout); }
		if(offset - g_last != g_chunksize) {
			printf("Missed %ld chunks!\n", (offset-g_last)/g_chunksize);
			fflush(stdout);
			if(g_bail_on_miss) { bail_early(); return; }
		}
	}
	g_last = offset;

	fseek(g_fh, offset, SEEK_SET);
	written = fwrite(buf, 1, len, g_fh);
	if(written != len) {
		fprintf(stderr, "fwrite(%ld) only wrote %lu!\n", len, written);
		exit(1);
	}

	// Update the hash object with data
	// This will only work if all the file pieces are in order!
	chash_upd(g_ch, buf, len);
}

static void file_end(char *buf, ssize_t len)
{
	if((!g_fh) || (!g_ch)) { return; }

	fclose(g_fh);
	g_fh = NULL;

	// Finish the hash object
	chash_fini(g_ch);
	char *filehash = chash_get_hash(g_ch);

	int compare = strncmp(buf, filehash, strlen(filehash));
	if(compare == 0) {
		printf("Ending %s with matching hash: %s\n", g_fname_final, buf);
		int z = rename(g_fname_temp, g_fname_final);
		if(z) { fprintf(stderr, "rename(%s, %s) failed: %s\n", g_fname_temp, g_fname_final, strerror(errno)); }
	} else {
		fprintf(stderr, "Ending %s with FAILED hash check!\n", g_fname_final);
	}
	remove(g_fname_temp);
	chash_del(g_ch);
	g_ch = NULL;
	g_shutdown = 1;
}

static void udb_cb(u4srv_t *s, u4clnt_t *c, char *buf, ssize_t len, void *user_data)
{
	if(g_shutdown) { return; }
	if((!buf) || (len <= 0)) { return; }

	//printf("UDP: %s %u %s\n", c->ddip, c->port, buf);

	int type = *buf;
	if(type == 0) { g_conn_port = c->port; file_beg(buf+1, len-1); }
	if(c->port != g_conn_port) { return; }	//Wrong Connection
	if(type == 1) { file_mid(buf+1, len-1); }
	if(type == 2) { file_end(buf+1, len-1); }
}

static void process_env(void)
{
	char *ip_arg = getenv("MCIP");
	if(ip_arg) { g_ip = ip_arg; }

	char *port_arg = getenv("PORT");
	if(port_arg) { g_srv_port = atoi(port_arg); }

	if(atoi(getenv("BAILONMISS")) != 0) {
		g_bail_on_miss = 1;
	}
}

int main(int argc, char *argv[])
{
	process_env();

	int z = as_udp4_bind_mcast(&g_srv, g_ip, g_srv_port, &udb_cb, NULL, 25);
	if(z < 0) {
		fprintf(stderr, "as_udp4_bind_mcast(%s, %u) failed!\n", g_ip, g_srv_port);
		exit(1);
	}

	signal(SIGINT,	sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGQUIT,	sig_handler);
	signal(SIGHUP,	sig_handler);

	while(!g_shutdown) {
		usleep(1000);
	}

	as_udp4_server_halt(&g_srv);

	if(g_sigcaught) { exit(99); }
	if(g_miss) { exit(98); }

	return 0;
}
