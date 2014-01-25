#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN        256
#define INTERVAL      3
#define FORMAT        "%s %i"
#define TOKSEP        "=\n"
#define BAT_PATH      "/sys/class/power_supply/BAT%i/uevent"
#define BAT_INDEX     0
#define KEY_PREFIX    "POWER_SUPPLY_"
#define STATUS_KEY    KEY_PREFIX "STATUS"
#define CAPACITY_KEY  KEY_PREFIX "CAPACITY"

#define CHARGING      "\u26A1"
#define WIRED         "\u2607"
#define BATTERY       "\U0001F50B"
#define UNKNOWN       "\uFFFD"

int bat_info(char *path, char *format, bool emoji, bool quiet)
{
	char status[MAXLEN] = {0};
	bool found_status = false, found_capacity = false;
	int capacity = -1;
	FILE *bf = fopen(path, "r");
	if (bf) {
		char line[MAXLEN] = {0};
		while (fgets(line, sizeof(line), bf) != NULL) {
			char *key = strtok(line, TOKSEP);
			if (key != NULL) {
				if (!found_status && strcmp(key, STATUS_KEY) == 0) {
					strncpy(status, strtok(NULL, TOKSEP), sizeof(status));
					found_status = true;
				} else if (capacity == -1 && strcmp(key, CAPACITY_KEY) == 0) {
					capacity = atoi(strtok(NULL, TOKSEP));
					found_capacity = true;
				}
			}
		}
		fclose(bf);
	} else {
		if (!quiet)
			fprintf(stderr, "Can't open '%s'.\n", path);
	}
	char *s = status;
	if (!found_capacity || !found_status) {
		s = emoji ? WIRED : "Wired";
		capacity = 100;
	} else if (emoji) {
		if (!strcmp(status, "Charging"))
			s = CHARGING;
		else if (!strcmp(status, "Full"))
			s = WIRED;
		else if (!strcmp(status, "Discharging"))
			s = BATTERY;
		else
			s = UNKNOWN;
	}
	printf(format, s, capacity);
	printf("\n");
	fflush(stdout);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	char *path = BAT_PATH;
	char *format = FORMAT;
	int index = BAT_INDEX;
	int interval = INTERVAL;
	bool snoop = false;
	bool emoji = false;
	bool quiet = false;

	int opt;
	while ((opt = getopt(argc, argv, "hseqf:i:p:n:")) != -1) {
		switch (opt) {
		case 'h':
			printf("battery [-h|-s|-e|-q|-f FORMAT|-i INTERVAL|-p PATH|-n INDEX]\n");
			exit(EXIT_SUCCESS);
			break;
		case 's':
			snoop = true;
			break;
		case 'f':
			format = optarg;
			break;
		case 'p':
			path = optarg;
			break;
		case 'i':
			interval = atoi(optarg);
			break;
		case 'n':
			index = atoi(optarg);
			break;
		case 'e':
			emoji = true;
			break;
		case 'q':
			quiet = true;
			break;
		}
	}

	char real_path[MAXLEN] = {0};
	snprintf(real_path, sizeof(real_path), path, index);

	int exit_code;

	if (snoop)
		while ((exit_code = bat_info(real_path, format, emoji, quiet)) != EXIT_FAILURE)
			sleep(interval);
	else
		exit_code = bat_info(real_path, format, emoji, quiet);

	return exit_code;
}
