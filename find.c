
// gcc find.c cubiomes/libcubiomes.a -Icubiomes/ -lm -Ofast -o find && ./find

#include "generator.h"
#include "finders.h"
#include "util.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <threads.h>
#include <stdatomic.h>

const int version = MC_1_19;
const uint64_t seed = 69420;
const long long startRegion = 0;
const int progressUpdateIntervalSeconds = 15;
const int threadcount = 4;
StructureConfig monumentConfig;
static Generator g;

void makeSnapshot(long long n, int x, int z);
static inline int spiral(long long n, Pos* ret);

mtx_t outputMutex;
atomic_llong globalN;

static inline int performChecks(long long n, int rx, int rz) {

	// locate structures
	Pos p = getLargeStructurePos(monumentConfig, seed, rx, rz);
	if(!isViableStructurePos(Monument, &g, p.x, p.z, 0)) {
		return 0;
	}

	// compare positions of different structures
	// Pos p2 = getLargeStructurePos(monumentConfig, seed, rx, rz-1);
	// if(!isViableStructurePos(Monument, &g, p2.x, p2.z, 0)) {
	// 	return 0;
	// }
	// int dx2 = p.x - p2.x;
	// int dz2 = p.z - p2.z;
	// if(!((dx2 <= 85 && dx2 >= 75) || (dx2 <= -85 && dx2 > -75))) {
	// 	return 0;
	// }
	// if(!((dz2 <= 405 && dz2 >= 395) || (dz2 <= -405 && dz2 > -395))) {
	// 	return 0;
	// }

	// check for structures
	// if(getStructurePos(Outpost, version, seed, rx, rz, &p) == 0) {
	// 	return 0;
	// }
	// if(!isViableStructurePos(Outpost, &g, p.x, p.z, 0)) {
	// 	return 0;
	// }

	// check for biomes
	// if(getBiomeAt(&g, 1, p.x, 256, p.z) != desert) {
	// 	return 0;
	// }

	// check for biomes relative to the located structure
	// if(getBiomeAt(&g, 1, p.x-571, 256, p.z-1680) != mushroomIsland) {
	// 	return 0;
	// }
	mtx_lock(&outputMutex);
	printf("%-20lld %-7d %-7d\n", n, p.x, p.z);
	fprintf(f, "%-20lld %-7d %-7d\n", n, p.x, p.z);
	makeSnapshot(n, p.x, p.z);
	mtx_unlock(&outputMutex);
	return 1;
}

FILE* f;

int th(void* args) {
	Pos p;
	for(;;) {
		long long n = atomic_fetch_add_explicit(&globalN, 1, memory_order_relaxed);
		spiral(n, &p);
		performChecks(n, p.x, p.z);
	}
}

int main() {
	printf("Hello world!\n");
	setvbuf(stdout, NULL, _IONBF, 0);

	setupGenerator(&g, version, 0);
	applySeed(&g, 0, seed);
	getStructureConfig(Monument, version, &monumentConfig);

	f = fopen("./potentialPositions.txt", "a");
	if(f == NULL) {
		printf("Failed to open file: %s\n", strerror(errno));
		return 0;
	}
	fprintf(f, "----------------\n");
	fflush(f);

	mtx_init(&outputMutex, mtx_plain);
	atomic_init(&globalN, startRegion);

	thrd_t threads[threadcount];

	for(int i = 0; i < threadcount; i++) {
		thrd_create(&threads[i], th, NULL);
	}

	for(;;) {
		mtx_unlock(&outputMutex);
		Pos p;
		long long n = atomic_load(&globalN);
		printf("Region %-12lld radius %-7d\n", n, spiral(n, &p)*512);
		mtx_lock(&outputMutex);
		thrd_sleep(&(struct timespec){.tv_sec=progressUpdateIntervalSeconds}, NULL);
	}

	fclose(f);
	return 0;
}


void makeSnapshot(long long n, int x, int z) {
	Range r;
	r.scale = 16;
	r.y = 256;
	r.x = x/r.scale - 128*2;
	r.z = z/r.scale - 128*2;
	r.sx = 512;
	r.sz = 512;
	r.sy = 0;
	int *biomeIds = allocCache(&g, r);
	genBiomes(&g, biomeIds, r);
	unsigned char biomeColors[256][3];
	initBiomeColors(biomeColors);
	unsigned char *rgb = (unsigned char *) malloc(3*r.sx*r.sz);
	biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, 1, 2);
	char fname[512];
	memset(fname, 0, 512);
	snprintf(fname, 511, "./out/%lld:%d:%d.ppm", n, x, z);
	savePPM(fname, rgb, r.sx, r.sz);
	free(biomeIds);
	free(rgb);
	printf("Saved: %s\n", fname);
	fflush(stdout);
}

static inline int spiral(long long n, Pos* ret) {
	long long r = floor((sqrt(n+1)-1)/2)+1;
	long long p = (8*r*(r-1))/2;
	long long en = r*2;
	long long a = (1+n-p)%(r*8);
	switch((long long)floor(a/(r*2))) {
		case 0:
		ret->x = a-r;
		ret->z = -r;
		break;
		case 1:
		ret->x = r;
		ret->z = (a % en) - r;
		break;
		case 2:
		ret->x = r - (a % en);
		ret->z = r;
		break;
		case 3:
		ret->x = -r;
		ret->z = r - (a % en);
		break;
		default:
		abort();
	}
	return r;
}
