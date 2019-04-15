/*
 * csim.c - implement a cache simulator
 */
#include "cachelab.h"
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ADDR_SIZE 64
typedef struct {
    char *filePath;
    int setNum;
    int lineNum;
    int blockNum;
    bool verbose;
} GlobalArgs;
GlobalArgs globalArgs;
static const char *optString = "hvs:E:b:t:";

/* 
 * setArgs - Read the arguments from user input,
 *           then store them into globalArgs.
 */
void setArgs(int argc, char **argv)
{
    int opt = getopt( argc, argv, optString );
    while( opt != -1 ) {
        switch( opt ) {
            case 'v':
                globalArgs.verbose = true; /* true */
                break;
            case 's':
                globalArgs.setNum = atoi(optarg);
                break;
            case 'E':
                globalArgs.lineNum = atoi(optarg);
                break;
            case 'b':
                globalArgs.blockNum = atoi(optarg);
                break;
            case 't':
                globalArgs.filePath = optarg;
                break;
            default:
                printf("wrong argument\n");
                break;
        }
        
        opt = getopt( argc, argv, optString );
    }
};
typedef struct {
    unsigned long tag;
    long timestamp; // 0 means not valid
} CacheLine;


/* 
 * tryToHitCache - Give a visiting address and check cache miss or hit
 *                 update the hit/miss count into res array
 */
void tryToHitCache(unsigned long address, CacheLine* cache, int res[3]) 
{
    static int timestamp = 0;
    int tagLen = (ADDR_SIZE - globalArgs.blockNum - globalArgs.setNum);
    int set = (int) ((address << tagLen) >> (tagLen + globalArgs.blockNum));
    unsigned long tag = (address >> (ADDR_SIZE - tagLen));
    int min = set * globalArgs.lineNum;
    for (int i = 0; i < globalArgs.lineNum; i++) {
        int idx = set * globalArgs.lineNum + i;
        if (cache[idx].tag == tag && cache[idx].timestamp != 0) {
            cache[idx].timestamp = ++timestamp;
            res[0]++;
            if (globalArgs.verbose) {
		printf(" hit");
            }
	    return;
        } else if (cache[idx].timestamp == 0) {
            cache[idx].timestamp = ++timestamp;
            cache[idx].tag = tag;
            res[1]++;
	    if (globalArgs.verbose) {
		printf(" miss");
            }
            return;
        }
        if (cache[idx].timestamp < cache[min].timestamp) {
            min = idx;
	}
    }
    cache[min].timestamp = ++timestamp;
    cache[min].tag = tag;
    res[1]++;
    res[2]++;
    if (globalArgs.verbose) {
	printf(" miss eviction");
    }
    return;
}
int main(int argc, char **argv)
{
    setArgs(argc,argv);
    if (globalArgs.filePath == NULL) {
        printf("File path not found. Please use -t {filepath}\n");
        exit(0);
    }
    FILE *fp = fopen(globalArgs.filePath,"r");
    if (fp == NULL) {
        printf("File not found %s\n", globalArgs.filePath);
        exit(0);
    }

    int res[3];//0:hit, 1: miss, 2 : evict
    for (int i = 0; i < 3; i++) {
        res[i] = 0;
    }

    int S = (1 << globalArgs.setNum);
    int E = (1 << globalArgs.lineNum);
    CacheLine *cache = 
        calloc(S * E, sizeof(CacheLine));

    char opt;
    unsigned long address;
    int block;
    while (fscanf(fp," %c %lx,%d", &opt, &address, &block) > 0) {
        if (opt == 'I') continue;
        //int hit = res[0], miss = res[1], evict = res[2];
        if (globalArgs.verbose) {
            printf("%c %lx,%d",opt,address,block);
        }
        tryToHitCache(address,cache,res);
        if (opt == 'M') {
            tryToHitCache(address,cache,res);
        }
        if (globalArgs.verbose) {
	    printf("\n");
	}
        
    }
    fclose(fp);
    free(cache);
    
    printSummary(res[0], res[1], res[2]);
    return 0;
}
