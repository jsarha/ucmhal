#include <stdint.h>
#include <stdio.h>

#include "UcmHalDev.h"

using namespace UcmHal;

int main(int argc, char *argv[]) {

	if (argc != 3) {
		fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
		return -1;
	}

    return 0;
}
