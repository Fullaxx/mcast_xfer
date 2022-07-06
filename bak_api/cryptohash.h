/*
	cryptohash is an easy to use interface to OpenSSL hashing functions
	Copyright (C) 2018 Brett Kuskie <fullaxx@gmail.com>

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

#ifndef __CRYPTOHASH_H__
#define __CRYPTOHASH_H__

#ifdef CHASH_USES_MDC2
#include <openssl/mdc2.h>
#endif

#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/whrlpool.h>

#define DGSTLENMAX	(512/8)

#define ALG_MDFOUR	( 1)
#define ALG_MDFIVE	( 2)
#define ALG_RIPEMD	( 3)
#define ALG_SHAONE	( 4)
#define ALG_SHA224	( 5)
#define ALG_SHA256	( 6)
#define ALG_SHA384	( 7)
#define ALG_SHA512	( 8)
#define ALG_WHRLPL	( 9)

#ifdef CHASH_USES_MDC2
#define ALG_MDCTWO	(10)
#endif

#define CHASH_ALG_MIN ( 1)
#ifdef CHASH_USES_MDC2
#define CHASH_ALG_MAX (10)
#else
#define CHASH_ALG_MAX ( 9)
#endif

/*
Sooo... if the ALG LIST changed, it will adjust the output of CHASH_GETALG
(i.e.) if we can't compile in MDCTWO for example, CHASH_ALG_MAX goes to 9
*/
#define CHASH_GETALG(x)	(((x)%(CHASH_ALG_MAX-CHASH_ALG_MIN+1))+CHASH_ALG_MIN)

typedef struct {
	int algorithm;
	int digestlen;
	unsigned long updates;
	unsigned long bytes;
	unsigned int complete;
	void *context;
	unsigned char digest[DGSTLENMAX];
	char hash[(DGSTLENMAX*2)+1];
} chash_t;

chash_t* chash_new(int);
int chash_dgstlen(chash_t *);
void chash_upd(chash_t *, void *, size_t);
void chash_fini(chash_t *);
void chash_del(chash_t *);
int chash_cpy_dgst(chash_t *, unsigned char *);
char* chash_get_hash(chash_t *);
int chash_get_sum(chash_t *);

#endif	/* __CRYPTOHASH_H__ */
