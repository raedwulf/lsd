#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/wireless.h>

#define INTERVAL   5
#define INTERFACE  "wlan0"
#define FORMAT     "%s %d"

static char *format = FORMAT;
static int interval = INTERVAL;
static char name[IW_ESSID_MAX_SIZE + 1] = {0};
static int max_qual = 0;

void wifi_info(int fd, const char *interface)
{
	struct iwreq request;
	memset(&request, 0, sizeof(struct iwreq));
	strcpy(request.ifr_name, interface);
	request.u.essid.pointer = name;
	request.u.essid.length = IW_ESSID_MAX_SIZE + 1;
	if (ioctl(fd, SIOCGIWESSID, &request) == -1) {
		perror("ioctl SIOCGIWESSID");
		exit(EXIT_FAILURE);
	}

	struct iw_statistics stats;
	memset(&request, 0, sizeof(struct iwreq));
	strcpy(request.ifr_name, interface);
	request.u.data.pointer = &stats;
	request.u.data.length = sizeof(struct iw_statistics);
	if (ioctl(fd, SIOCGIWSTATS, request) == -1) {
		perror("ioctl SIOCGIWSTATS");
		exit(EXIT_FAILURE);
	}
	int q = (100*stats.qual.qual) / max_qual;
	printf(format, name, q);
	putchar('\n');
	fflush(stdout);
}

int main(int argc, char *argv[])
{
	char *interface = INTERFACE;
	bool snoop = false;

	char opt;
	while ((opt = getopt(argc, argv, "hsf:i:w:")) != -1) {
		switch (opt) {
		case 'h':
			printf("essid [-h|-s|-i INTERVAL|-f FORMAT|-w INTERFACE]\n");
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
		case 'w':
			interface = optarg;
			break;
		}
	}

	int fd;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	struct iwreq request;	
	struct iw_range range;
	memset(&request, 0, sizeof(struct iwreq));
	strcpy(request.ifr_name, interface);
	request.u.data.pointer = &range;
	request.u.data.length = sizeof(range);
	if (ioctl(fd, SIOCGIWRANGE, request) == -1) {
		perror("ioctl SIOCGIWRANGE");
		exit(EXIT_FAILURE);
	}
	max_qual = range.max_qual.qual;

	if (snoop)
		while (true) {
			wifi_info(fd, interface);
			sleep(interval);
			name[0] = '\0';
		}
	else
		wifi_info(fd, interface);

	close(fd);
	if (strlen(name) > 0)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
