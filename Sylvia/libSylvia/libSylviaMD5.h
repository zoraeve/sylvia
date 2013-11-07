#pragma once

#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

typedef struct _LIBSYLVIA_MD5_CTX_
{
	UINT4 state[4]; /* state (ABCD) */
	UINT4 count[2]; /* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64]; /* input buffer */
} LIBSYLVIA_MD5_CTX;

void libSylvia_MD5Init(LIBSYLVIA_MD5_CTX*);
void libSylvia_MD5Update(LIBSYLVIA_MD5_CTX*, unsigned char*, unsigned int);
void libSylvia_MD5Final(unsigned char[16], LIBSYLVIA_MD5_CTX*);
