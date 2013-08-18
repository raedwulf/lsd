#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN        256
#define INTERVAL      5
#define FORMAT        "%d"
#define TOKSEP        ":"
#define MEM_PATH      "/proc/meminfo"
#define MEMTOTAL_KEY  "MemTotal"
#define MEMFREE_KEY   "MemFree"
#define BUFFERS_KEY   "Buffers"
#define CACHED_KEY    "Cached"
#define SLEN(x)       (sizeof(x)-1)

int mem_info(char *format)
{
	FILE *f = fopen(MEM_PATH, "r");
	if (f == NULL) {
		fprintf(stderr, "Can't open '%s'.\n", MEM_PATH);
		return EXIT_FAILURE;
	}
	int mem[4];
	char *keys[4] = {MEMTOTAL_KEY, MEMFREE_KEY, BUFFERS_KEY, CACHED_KEY};
	int lens[4] = {SLEN(MEMTOTAL_KEY), SLEN(MEMFREE_KEY),
		SLEN(BUFFERS_KEY), SLEN(CACHED_KEY)};
	do {
		char line[MAXLEN];
		fgets(line, MAXLEN, f);
		for (int i = 0; i < 4; i++) {
			if (!strncmp(keys[i], line, lens[i])) {
				const char *p = line + lens[i];
				if (*p++ == ':') {
					mem[i] = atoi(p);
					break;
				}
			}
		}
	} while (!feof(f));
	fclose(f);
	
	printf(format, ((mem[0] - mem[1] - mem[2] - mem[3])*100)/mem[0]);
	putchar('\n');

	fflush(stdout);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	char *format = FORMAT;
	int interval = INTERVAL;
	bool snoop = false;

	char opt;
	while ((opt = getopt(argc, argv, "hsf:i:p:n:")) != -1) {
		switch (opt) {
		case 'h':
			printf("battery [-h|-s|-f FORMAT|-i INTERVAL\n");
			exit(EXIT_SUCCESS);
			break;
		case 's':
			snoop = true;
			break;
		case 'f':
			format = optarg;
			break;
		case 'i':
			interval = atoi(optarg);
			break;
		}
	}

	int exit_code;

	if (snoop)
		while ((exit_code = mem_info(format)) != EXIT_FAILURE)
			sleep(interval);
	else
		exit_code = mem_info(format);

	return exit_code;
}
