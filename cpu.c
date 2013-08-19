#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN        256
#define INTERVAL      1
#define FORMAT        "%c %3d"
#define MAXCPUS       8
#define DELIMETER     " "
#define PREFIX        ""
#define CPU_PATH      "/proc/stat"

static int cpu_info(const char *prefix, const char *format, bool avg, bool hasc, const char *delimeter)
{
	FILE *f = fopen(CPU_PATH, "r");
	if (!f) {
		fputs("Can't open " CPU_PATH, stderr);
		return EXIT_FAILURE;
	}
	bool first = false;
	int cpus = 0;
	int wj_ = 0, tj_ = 0;
	static int wj[MAXCPUS], tj[MAXCPUS];
	for (;;) {
		char line[MAXLEN];
		fgets(line, MAXLEN, f);
		if (!strncmp(line, "cpu", 3)) {
			char cpunum = 'a';
			char *p = line + 3;
			if (*p == ' ') {
				if (!avg) continue;	
			} else
				cpunum = *p++;
			p++;
			for (int i = 0; i < 7; i++, p++) {
				int v = strtol(p, &p, 10);
				if (i < 3) wj_ += v;
				tj_ += v;
			}
			int usage = ((wj_-wj[cpus])*100)/(tj_-tj[cpus]);
			printf("%s", first ? delimeter : prefix);
			if (usage < 0 || usage > 100) usage = 0;
			if (hasc)
				printf(format, cpunum, usage);
			else
				printf(format, usage);
			wj[cpus] = wj_;
			tj[cpus] = tj_;
			cpus++;
			first = true;
		} else
			break;
	}
	fclose(f);
	printf("\n");
	fflush(stdout);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	char *format = FORMAT;
	int interval = INTERVAL;
	bool snoop = false;
	bool avg = false;
	const char *delimeter = DELIMETER;
	const char *prefix = PREFIX;

	char opt;
	while ((opt = getopt(argc, argv, "hscad:p:f:i:")) != -1) {
		switch (opt) {
		case 'h':
			printf("cpu [-h|-s|-c|-a|-d DELIMETER|-p PREFIX|-f FORMAT|-i INTERVAL]\n");
			return EXIT_SUCCESS;
		case 'c': {
				FILE *f = fopen("/proc/stat", "r");
				int count = 0;
				do {
					char three[4];
					fgets(three, sizeof(three), f);
					count += !strcmp(three, "cpu");
				} while (!feof(f));
				fclose(f);
				printf("%d\n", count - 1);
			}
			return EXIT_SUCCESS;
		case 'a':
			avg = true;
			break;
		case 'd':
			delimeter = optarg;
			break;
		case 'p':
			prefix = optarg;
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

	char *p = format;
	int hasc = 0;
	while (*p)
		if (*p++ == '%') {
			if (*p == '%') p++; else hasc++;
		}
	if (hasc == 0 || hasc > 2)
		fprintf(stderr, "Invalid format.\n");

	if (snoop)
		while ((exit_code = cpu_info(prefix, format, avg, (hasc-1), delimeter))
			!= EXIT_FAILURE)
			sleep(interval);
	else
		exit_code = cpu_info(prefix, format, avg, (hasc-1), delimeter);

	return exit_code;
}
