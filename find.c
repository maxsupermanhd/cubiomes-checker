
// gcc find.c cubiomes/libcubiomes.a -Icubiomes/ -lm -Ofast -o find && ./find

#include "generator.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

struct sampleBiome {
	int64_t x, y, z;
	int biome;
};

typedef struct sampleBiome sampleBiome_t;

int main() {
	printf("Hello world!\n");
	setvbuf(stdout, NULL, _IONBF, 0);

	const int version = MC_1_18;
	size_t sampleLen = 6;
	sampleBiome_t* sample = malloc(sizeof(sampleBiome_t)*sampleLen);
	sample[0] = (sampleBiome_t){15448, 72, 27401, savanna};
	sample[1] = (sampleBiome_t){15725, 63, 27445, lukewarm_ocean};
	sample[2] = (sampleBiome_t){15610, 63, 28446, beach};
	sample[3] = (sampleBiome_t){15177, 97, 28478, savanna};
	sample[4] = (sampleBiome_t){15290, 67, 29013, plains};
	sample[5] = (sampleBiome_t){15402, 69, 29163, forest};


	Generator g;
	setupGenerator(&g, version, 0);

	FILE* f = fopen("./potentialSeeds.txt", "r");
	if(f == NULL) {
		printf("Failed to open file: %s\n", strerror(errno));
		return 0;
	}
	uint64_t tops = 0, toph = 0, s = 0;
	while(fscanf(f, "%ld\n", &s) != EOF) {
		applySeed(&g, 0, s);
		char succeeded = 0;
		for(int s = 0; s < sampleLen; s++) {
			succeeded += getBiomeAt(&g, 1, sample[s].x, sample[s].y, sample[s].z) == sample[s].biome;
		}
		printf("Seed % 16ld % 3d hits\n", s, succeeded);
		if(succeeded == sampleLen) {
			printf("\n\nSeed found!\n\n\n");
		}
		if(toph < succeeded) {
			tops = s;
			toph = succeeded;
		}
	}
	printf("Top match % 16ld with % 3ld hits out of % 3ld samples\n", tops, toph, sampleLen);

	free(sample);
	fclose(f);
	return 0;
}