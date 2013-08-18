#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN        256
#define INTERVAL      1
#define FORMAT        "%d"
#define DELIMETER     " "
#define PREFIX        ""
#define MAXTHERMAL    10
#define THERMAL_PATH  "/sys/class/thermal/thermal_zone%d/temp"

static int thermal_info(const char *prefix, const char *format, const char *delimeter)
{
	bool first = false;
	for (int i = 0; i < MAXTHERMAL; i++) {
		char path[MAXLEN];
		snprintf(path, MAXLEN, THERMAL_PATH, i);
		FILE *f = fopen(path, "r");
		if (!f) break;
		int temp;
		if (fscanf(f, "%d", &temp) == 1) {
			printf("%s", first ? delimeter : prefix);
			printf(format, temp / 1000);
			first = true;
		}
		fclose(f);
	}
	putchar('\n');
	fflush(stdout);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	char *format = FORMAT;
	int interval = INTERVAL;
	bool snoop = false;
	const char *delimeter = DELIMETER;
	const char *prefix = PREFIX;

	char opt;
	while ((opt = getopt(argc, argv, "hsd:p:f:i:")) != -1) {
		switch (opt) {
		case 'h':
			printf("cpu [-h|-s|-a|-d DELIMETER|-p PREFIX|-f FORMAT|-i INTERVAL]\n");
			return EXIT_SUCCESS;
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

	if (snoop)
		while ((exit_code = thermal_info(prefix, format, delimeter))
			!= EXIT_FAILURE)
			sleep(interval);
	else
		exit_code = thermal_info(prefix, format, delimeter);

	return exit_code;
}
