#define _GNU_SOURCE	// basename()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
//#include <libgen.h> //basename()

#include "async_udp4.h"
#include "futils.h"
#include "cryptohash.h"

u4clnt_t g_uc;
char *g_ip = "224.111.111.111";
unsigned short g_port = 11111;
int g_mtu = 1500;
int g_sleepdelay = 0;
chash_t *g_ch = NULL;

static void send_file(char *filename, long filesize)
{
	int n, bytes;
	long offset, noffset;
	char *filehash;
	unsigned char payload[65536];

	int chunksize = g_mtu-60;
	FILE *f = fopen(filename, "r");
	if(!f) { fprintf(stderr, "fopen(%s, r) failed: %s\n", filename, strerror(errno)); exit(1); }

	// Start a hash object
	g_ch = chash_new(ALG_SHA512);

	// Write File Header
	payload[0] = 0;
	n = sprintf((char *)&payload[1], "%s %ld %d", basename(filename), filesize, chunksize);
	as_udp4_client_write(&g_uc, &payload[0], n+2);

	// Write File Chunks
	offset = 0;
	payload[0] = 1;
	while(filesize > 0) {
		if(feof(f)) { fprintf(stderr, "feof(%s) is TRUE!\n", filename); exit(1); }
		if(filesize < chunksize) { chunksize = filesize; }
		noffset = htonl(offset);
		memcpy(&payload[1], &noffset, 8);
		bytes = fread(&payload[9], 1, chunksize, f);
		as_udp4_client_write(&g_uc, &payload[0], bytes+9);
		chash_upd(g_ch, &payload[9], bytes);
		//putchar('.');
		if(g_sleepdelay > 0) { usleep(g_sleepdelay); }
		offset += bytes;
		filesize -= bytes;
	}

	// Finish the hash object
	chash_fini(g_ch);
	filehash = chash_get_hash(g_ch);

	// Write File Trailer
	payload[0] = 2;
	n = sprintf((char *)&payload[1], "%s", filehash);
	as_udp4_client_write(&g_uc, &payload[0], n+2);
	usleep(25);
	as_udp4_client_write(&g_uc, &payload[0], n+2);
	usleep(25);
	as_udp4_client_write(&g_uc, &payload[0], n+2);
	//putchar('\n');

	fclose(f);
}

static void process_env(void)
{
	char *mtu_arg;
	char *sleep_arg;
	char *ip_arg;
	char *port_arg;

	mtu_arg = getenv("MTU");
	if(mtu_arg) { g_mtu = atoi(mtu_arg); }

	sleep_arg = getenv("SLEEPDELAY");
	if(sleep_arg) { g_sleepdelay = atoi(sleep_arg); }

	ip_arg = getenv("MCIP");
	if(ip_arg) { g_ip = ip_arg; }

	port_arg = getenv("PORT");
	if(port_arg) { g_port = atoi(port_arg); }
}

int main(int argc, char *argv[])
{
	struct timespec start;
	struct timespec stop;

	process_env();

	if(argc != 2) {
		fprintf(stderr, "%s: <file>\n", argv[0]);
		exit(1);
	}
	char *filename = argv[1];

	int z = as_udp4_connect(&g_uc, g_ip, g_port);
	if(z < 0) {
		fprintf(stderr, "as_udp4_connect(%s, %u) failed!\n", g_ip, g_port);
		exit(1);
	}

	long filesize = get_filesize(filename, 1);
	printf("Started SEND of %s (%ld MB)", filename, filesize/(long)1e6);
	printf(" [mtu:%d]", g_mtu);
	if(g_sleepdelay > 0) { printf(" {delay:%dus}", g_sleepdelay); }
	printf(" ...\n");

	clock_gettime(CLOCK_MONOTONIC, &start);
	send_file(filename, filesize);
	clock_gettime(CLOCK_MONOTONIC, &stop);

	long ns = ((stop.tv_sec - start.tv_sec)*1e9) + (stop.tv_nsec - start.tv_nsec);
	double MB = (double)filesize/(double)1e6;
	double sec = (double)ns/(double)1e9;
	printf("Sent %.3fMB in %.3fs: %.3f Mbps\n", MB, sec, (MB*8.0)/sec);

	return 0;
}
