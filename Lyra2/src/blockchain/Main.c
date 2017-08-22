#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "../Lyra2.h"
#include "../Sponge.h"

#ifndef BENCH
        #define BENCH 0
#endif

#include "sph_blake.h"
#include "sph_cubehash.h"
#include "sph_bmw.h"
#include "sph_keccak.h"
#include "sph_skein.h"

int work();

int main(int argc, char *argv[]) {
    unsigned char buf[128];
    unsigned char output[128];

    memset(buf, 0, sizeof buf);

    char *input = (char *)&buf[0];
    input[0] = 0;

    sph_blake256_context ctx_blake;
	sph_cubehash256_context ctx_cubehash;
	sph_keccak256_context ctx_keccak;
	sph_skein256_context ctx_skein;
	sph_bmw256_context ctx_bmw;

	uint32_t hashA[8], hashB[8];

	sph_blake256_init(&ctx_blake);
    sph_blake256(&ctx_blake, input, 80);
    sph_blake256_close (&ctx_blake, hashA);

    sph_keccak256_init(&ctx_keccak);
    sph_keccak256(&ctx_keccak, hashA, 32);
    sph_keccak256_close(&ctx_keccak, hashB);

    sph_cubehash256_init(&ctx_cubehash);
    sph_cubehash256(&ctx_cubehash, hashB, 32);
    sph_cubehash256_close(&ctx_cubehash, hashA);

    work(hashA, hashA, hashB);

    sph_skein256_init(&ctx_skein);
    sph_skein256(&ctx_skein, hashB, 32);
    sph_skein256_close(&ctx_skein, hashA);

    sph_cubehash256_init(&ctx_cubehash);
    sph_cubehash256(&ctx_cubehash, hashA, 32);
    sph_cubehash256_close(&ctx_cubehash, hashB);

    sph_bmw256_init(&ctx_bmw);
    sph_bmw256(&ctx_bmw, hashB, 32);
    sph_bmw256_close(&ctx_bmw, hashA);

    //output = malloc(32);
   	memcpy(output, hashA, 32);

    printf("Blockchain output: \n");

    printf("\n\tK: ");
    int i;
    for (i = 0; i < 32; i++) {
        printf("%02x|", output[i]);
    }
    printf("\n");
    printf("------------------------------------------------------------------------------------------------------------------------------------------\n");

    //free(output);
}

int work(char *pwd, char *salt, void *output) {
    //=================== Basic variables, with default values =======================//
    unsigned int kLen = 32;         // kLen
    unsigned int t_cost = 1;        // tCost
    unsigned int m_cost = 4;        // nRows
    char *spongeName ="Blake2";

    //char *pwd = "Lyra2 PHS";
    //unsigned int pwdLen = strlen(pwd);
    //char *salt = "saltsaltsaltsalt";
    //unsigned int saltLen = strlen(salt);

    unsigned int pwdLen = 32;
    unsigned int saltLen = 32;

    unsigned char *K = malloc(kLen);

    printf("Inputs: \n");
    //printf("\tPassword: %s\n", pwd);
    printf("\tPassword Length: %u\n", pwdLen);
    //printf("\tSalt: %s\n", salt);
    printf("\tSalt Length: %u\n", saltLen);
    printf("\tOutput Length: %u\n", kLen);
    printf("------------------------------------------------------------------------------------------------------------------------------------------\n");

    printf("Parameters: \n");
    printf("\tT: %u\n", t_cost);
    printf("\tR: %u\n", m_cost);
    printf("\tC: %u\n", N_COLS);
    printf("\tParallelism: %u\n", nPARALLEL);

    printf("\tSponge: %s\n", spongeName);
    printf("\tSponge Blocks (bitrate): %u = %u bits\n", BLOCK_LEN_INT64, BLOCK_LEN_INT64*64);

    size_t sizeMemMatrix = (size_t) ((size_t)m_cost * (size_t)ROW_LEN_BYTES);

    if(sizeMemMatrix > (1610612736)){
    printf("\tMemory: %ld bytes (IMPORTANT: This implementation is known to have "
            "issues for such a large memory usage)\n", sizeMemMatrix);
    }else{
        printf("\tMemory: %ld bytes\n", sizeMemMatrix);
    }


    printf("------------------------------------------------------------------------------------------------------------------------------------------\n");

    #if (BENCH == 1)
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
#endif
    int result;

    result = PHS(K, kLen, pwd, pwdLen, salt, saltLen, t_cost, m_cost);

#if (BENCH == 1)
    gettimeofday(&end, NULL);
    unsigned long elapsed = (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec;
    printf("Execution Time: %lu us\n", elapsed);
    printf("------------------------------------------------------------------------------------------------------------------------------------------\n");
#endif

    switch (result) {
        case 0:
            memcpy(output, K, kLen);

            printf("Output: \n");

            printf("\n\tK: ");
            int i;
            for (i = 0; i < kLen; i++) {
                printf("%02x|", K[i]);
            }
            break;
        case -1:
            printf("Error: unable to allocate memory (nRows too large?)\n");
            break;
        default:
            printf("Unexpected error\n");
            break;
    }

    printf("\n");
    printf("------------------------------------------------------------------------------------------------------------------------------------------\n");
    free(K);

    return 0;
}
