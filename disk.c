#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <errno.h>

#define INTERVAL   5
#define DISK_PATH  "/"
#define FORMAT     "%d%%"

enum {
	DISK_PERCENTAGE_FREE,      /* single %d */
	DISK_FREE_SPACE_REMAINING, /* %g %s */
	DISK_FREE_SPACE_OVER_ALL   /* %g %g %s */
};

static char *format = FORMAT;
static int interval = INTERVAL;

int disk_info(const char *path, const char *format, int format_type, bool has_string)
{
	struct statvfs s;
	if (statvfs(path, &s))
		return EXIT_FAILURE;
	
	double size, mib_size, gib_size;

	switch (format_type) {
	case DISK_PERCENTAGE_FREE:
		printf(format, (s.f_bavail * 100) / s.f_blocks);
		break;
	case DISK_FREE_SPACE_REMAINING:
		size = (double)(s.f_bavail * s.f_bsize);
		mib_size = size / (double)(1 << 20);		
		gib_size = size / (double)(1 << 30);
		if (has_string)
			printf(format, gib_size < 1.0 ? mib_size : gib_size,
				gib_size < 1.0 ? "MiB" : "GiB");
		else
			printf(format, gib_size < 1.0 ? mib_size : gib_size);
		break;
	case DISK_FREE_SPACE_OVER_ALL:
		size = ((double)s.f_bavail * s.f_bsize) / (double)(1 << 30);
		gib_size = ((double)s.f_bavail * s.f_bsize) / (double)(1 << 30);
		if (has_string)
			printf(format, gib_size, size, "GiB");
		else
			printf(format, gib_size, size);
		break;
	}
	putchar('\n');
	fflush(stdout);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	char *path = DISK_PATH;
	bool snoop = false;

	char opt;
	while ((opt = getopt(argc, argv, "hsf:i:w:")) != -1) {
		switch (opt) {
		case 'h':
			printf("disk [-h|-s|-i INTERVAL|-f FORMAT|-p PATH]\n");
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			interval = atoi(optarg);
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
		}
	}

	int exit_code;
	int format_type = -1;
	
	const char *p = format;
	bool in_format = false;
	bool has_string = false;
	while (*p) {
		switch (*p) {
			case '%':
				in_format = !in_format;
				break;
			case 'd':
				if (in_format) {
					format_type = DISK_PERCENTAGE_FREE;
					in_format = false;
				}
				break;
			case 'f':
			case 'g':
				if (in_format) {
					if (format_type == -1)
						format_type = DISK_FREE_SPACE_REMAINING;
					else
						format_type = DISK_FREE_SPACE_OVER_ALL;
					in_format = false;
				}
				break;
			case 's':
				if (in_format) {
					if (has_string || format_type == -1) {
						fprintf(stderr, "Invalid format.\n");
						return EXIT_FAILURE;
					}
					has_string = true;
					in_format = false;
				}
				break;
			default:
				if (in_format && *p >= 'a' && *p <= 'z') {
					fprintf(stderr, "Invalid format.\n");
					return EXIT_FAILURE;
				}	
		}
		p++;
	}
	
	if (snoop)
		while (exit_code = disk_info(path, format, format_type, has_string)
			!= EXIT_FAILURE) sleep(interval);
	else
		disk_info(path, format, format_type, has_string);

	return EXIT_SUCCESS;
}
