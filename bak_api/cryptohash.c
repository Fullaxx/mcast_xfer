/*
	cryptohash is an easy to use interface to OpenSSL hashing functions
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

/*
Ubuntu Install: sudo apt install libssl-dev
Compile with: cryptohash.c -o binary -lcrypto
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cryptohash.h"

chash_t* chash_new(int alg)
{
	chash_t *s = calloc(1, sizeof(chash_t));
	s->algorithm = alg;
	switch(alg) {
		case ALG_MDFOUR:
			s->digestlen = MD4_DIGEST_LENGTH;
			s->context = malloc(sizeof(MD4_CTX));
			MD4_Init(s->context);
			break;
		case ALG_MDFIVE:
			s->digestlen = MD5_DIGEST_LENGTH;
			s->context = malloc(sizeof(MD5_CTX));
			MD5_Init(s->context);
			break;
		case ALG_RIPEMD:
			s->digestlen = RIPEMD160_DIGEST_LENGTH;
			s->context = malloc(sizeof(RIPEMD160_CTX));
			RIPEMD160_Init(s->context);
			break;
		case ALG_SHAONE:
			s->digestlen = SHA_DIGEST_LENGTH;
			s->context = malloc(sizeof(SHA_CTX));
			SHA1_Init(s->context);
			break;
		case ALG_SHA224:
			s->digestlen = SHA224_DIGEST_LENGTH;
			s->context = malloc(sizeof(SHA256_CTX));
			SHA224_Init(s->context);
			break;
		case ALG_SHA256:
			s->digestlen = SHA256_DIGEST_LENGTH;
			s->context = malloc(sizeof(SHA256_CTX));
			SHA256_Init(s->context);
			break;
		case ALG_SHA384:
			s->digestlen = SHA384_DIGEST_LENGTH;
			s->context = malloc(sizeof(SHA512_CTX));
			SHA384_Init(s->context);
			break;
		case ALG_SHA512:
			s->digestlen = SHA512_DIGEST_LENGTH;
			s->context = malloc(sizeof(SHA512_CTX));
			SHA512_Init(s->context);
			break;
		case ALG_WHRLPL:
			s->digestlen = WHIRLPOOL_DIGEST_LENGTH;
			s->context = malloc(sizeof(WHIRLPOOL_CTX));
			WHIRLPOOL_Init(s->context);
			break;
#ifdef CHASH_USES_MDC2
		case ALG_MDCTWO:
			s->digestlen = MDC2_DIGEST_LENGTH;
			s->context = malloc(sizeof(MDC2_CTX));
			MDC2_Init(s->context);
			break;
#endif
		default:
			free(s); s=NULL;
	}
	return s;
}

int chash_dgstlen(chash_t *s)
{
	return s->digestlen;
}

void chash_upd(chash_t *s, void *string, size_t len)
{
	s->updates++;
	s->bytes += len;
	switch(s->algorithm) {
		case ALG_MDFOUR: MD4_Update(s->context, string, len); break;
		case ALG_MDFIVE: MD5_Update(s->context, string, len); break;
		case ALG_RIPEMD: RIPEMD160_Update(s->context, string, len); break;
		case ALG_SHAONE: SHA1_Update(s->context, string, len); break;
		case ALG_SHA224: SHA224_Update(s->context, string, len); break;
		case ALG_SHA256: SHA256_Update(s->context, string, len); break;
		case ALG_SHA384: SHA384_Update(s->context, string, len); break;
		case ALG_SHA512: SHA512_Update(s->context, string, len); break;
		case ALG_WHRLPL: WHIRLPOOL_Update(s->context, string, len); break;
#ifdef CHASH_USES_MDC2
		case ALG_MDCTWO: MDC2_Update(s->context, string, len); break;
#endif
	}
}

void chash_fini(chash_t *s)
{
	int i;
	switch(s->algorithm) {
		case ALG_MDFOUR: MD4_Final(s->digest, s->context); break;
		case ALG_MDFIVE: MD5_Final(s->digest, s->context); break;
		case ALG_RIPEMD: RIPEMD160_Final(s->digest, s->context); break;
		case ALG_SHAONE: SHA1_Final(s->digest, s->context); break;
		case ALG_SHA224: SHA224_Final(s->digest, s->context); break;
		case ALG_SHA256: SHA256_Final(s->digest, s->context); break;
		case ALG_SHA384: SHA384_Final(s->digest, s->context); break;
		case ALG_SHA512: SHA512_Final(s->digest, s->context); break;
		case ALG_WHRLPL: WHIRLPOOL_Final(s->digest, s->context); break;
#ifdef CHASH_USES_MDC2
		case ALG_MDCTWO: MDC2_Final(s->digest, s->context); break;
#endif
	}
	for(i=0; i<(s->digestlen); i++) { sprintf(&s->hash[i*2], "%02x", s->digest[i]); }
	s->complete = 1;
}

void chash_del(chash_t *s)
{
	size_t size = 0;
	switch(s->algorithm) {
		case ALG_MDFOUR: size = sizeof(MD4_CTX); break;
		case ALG_MDFIVE: size = sizeof(MD5_CTX); break;
		case ALG_RIPEMD: size = sizeof(RIPEMD160_CTX); break;
		case ALG_SHAONE: size = sizeof(SHA_CTX); break;
		case ALG_SHA224: size = sizeof(SHA256_CTX); break;
		case ALG_SHA256: size = sizeof(SHA256_CTX); break;
		case ALG_SHA384: size = sizeof(SHA512_CTX); break;
		case ALG_SHA512: size = sizeof(SHA512_CTX); break;
		case ALG_WHRLPL: size = sizeof(WHIRLPOOL_CTX); break;
#ifdef CHASH_USES_MDC2
		case ALG_MDCTWO: size = sizeof(MDC2_CTX); break;
#endif
	}
	if(size > 0) { memset(s->context, 0, size); }
	free(s->context);
	free(s);
}

/* This will copy the digest bits to your pointer */
int chash_cpy_dgst(chash_t *s, unsigned char *dstdgst)
{
	if(!s->complete) { return 1; }
	memcpy(dstdgst, &s->digest[0], s->digestlen);
	return 0;
}

/* This will return the hash in human readable hex */
char* chash_get_hash(chash_t *s)
{
	if(s->complete) { return &s->hash[0]; }
	return NULL;
}

int chash_get_sum(chash_t *s)
{
	int i, sum=0;
	if(!s->complete) { return -1; }
	for(i=0; i<(s->digestlen); i++) { sum += (int) s->digest[i]; }
	return sum;
}
