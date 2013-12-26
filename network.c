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
#define WIFI_INTERFACE "wlan0"
#define ETH_INTERFACE  "eth0"
#define FORMAT     "%s %d"

static char *format = FORMAT;
static int interval = INTERVAL;
static char name[IW_ESSID_MAX_SIZE + 1] = {0};
static int max_qual = 0;

void network_info(int fd, const char *wifi_interface, const char *eth_interface)
{
	struct iwreq request;	

	char path[256] = "/sys/class/net/";
	strncat(path, eth_interface, sizeof(path) - strlen(path) - sizeof("/carrier"));
	strcat(path, "/carrier"); 
	FILE *f = fopen(path, "r");
	if (f) {
		int c;
		if ((c = fgetc(f)) != EOF) {
			printf(format, "wired", c - '0');
			putchar('\n');
			fflush(stdout);
		} else
			perror("no network information");
		fclose(f);
		if (c == '1')
			return;
	}

	if (max_qual == 0) {
		struct iw_range range;
		memset(&request, 0, sizeof(struct iwreq));
		strcpy(request.ifr_name, wifi_interface);
		request.u.data.pointer = &range;
		request.u.data.length = sizeof(range);
		if (ioctl(fd, SIOCGIWRANGE, request) == -1) {
			perror("ioctl SIOCGIWRANGE");
		}
		max_qual = range.max_qual.qual;
	}

	memset(&request, 0, sizeof(struct iwreq));
	strcpy(request.ifr_name, wifi_interface);
	request.u.essid.pointer = name;
	request.u.essid.length = IW_ESSID_MAX_SIZE + 1;
	if (ioctl(fd, SIOCGIWESSID, &request) == -1) {
		strcpy(name, "ERROR");
		perror("ioctl SIOCGIWESSID");
	}

	struct iw_statistics stats;
	memset(&request, 0, sizeof(struct iwreq));
	strcpy(request.ifr_name, wifi_interface);
	request.u.data.pointer = &stats;
	request.u.data.length = sizeof(struct iw_statistics);
	if (ioctl(fd, SIOCGIWSTATS, &request) == -1) {
		perror("ioctl SIOCGIWSTATS");
	}
	int q = (100*stats.qual.qual) / max_qual;
	printf(format, name, q);
	putchar('\n');
	fflush(stdout);
}

int main(int argc, char *argv[])
{
	char *wifi_interface = WIFI_INTERFACE;
	char *eth_interface = ETH_INTERFACE;
	bool snoop = false;

	char opt;
	while ((opt = getopt(argc, argv, "hsf:i:w:e:")) != -1) {
		switch (opt) {
		case 'h':
			printf("network [-h|-s|-i INTERVAL|-f FORMAT|-w WIFI_INTERFACE|-e ETH_INTERFACE]\n");
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
			wifi_interface = optarg;
			break;
		case 'e':
			eth_interface = optarg;
			break;
		}
	}

	int fd;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (snoop)
		while (true) {
			network_info(fd, wifi_interface, eth_interface);
			sleep(interval);
			name[0] = '\0';
		}
	else
		network_info(fd, wifi_interface, eth_interface);

	close(fd);
	if (strlen(name) > 0)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
