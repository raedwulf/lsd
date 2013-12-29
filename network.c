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

#define INTERVAL       5
#define WIFI_INTERFACE "wlan0"
#define ETH_INTERFACE  "eth0"
#define FORMAT         "%s %d"
#define XFORMAT        "%s %s %d"
#define WIRED          "\U0001F50C"

static char *strength[] = { "\u2582", "\u2584", "\u2586", "\u2588" };

static char *format = FORMAT;
static int interval = INTERVAL;
static char name[IW_ESSID_MAX_SIZE + 1] = {0};
static int max_qual = 0;

void network_info(int fd, const char *wifi_interface, const char *eth_interface, bool xtended)
{
	bool has_wired = false;
	char path[256] = "/sys/class/net/";
	strncat(path, eth_interface, sizeof(path) - strlen(path) - sizeof("/carrier"));
	strcat(path, "/carrier"); 
	FILE *f = fopen(path, "r");
	if (f) {
		int c;
		if ((c = fgetc(f)) != EOF) {
			has_wired = (c == '1');
		} else
			perror("no network information");
		fclose(f);
		if (c == '1')
			return;
	}

	if (has_wired) {
		if (xtended)
			printf(format, WIRED, "wired", 100);
		else
			printf(format, "wired", 100);
		goto finish;
	}

	bool failed = false;
	struct iwreq request;	
	if (max_qual == 0) {
		struct iw_range range;
		memset(&request, 0, sizeof(struct iwreq));
		strcpy(request.ifr_name, wifi_interface);
		request.u.data.pointer = &range;
		request.u.data.length = sizeof(range);
		if (ioctl(fd, SIOCGIWRANGE, request) == -1) {
			perror("ioctl SIOCGIWRANGE");
			failed = true;
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
		failed = true;
	}

	struct iw_statistics stats;
	memset(&request, 0, sizeof(struct iwreq));
	strcpy(request.ifr_name, wifi_interface);
	request.u.data.pointer = &stats;
	request.u.data.length = sizeof(struct iw_statistics);
	if (ioctl(fd, SIOCGIWSTATS, &request) == -1) {
		perror("ioctl SIOCGIWSTATS");
		failed = true;
	}

	if (failed) {
		if (xtended)
			printf(format, "X", "disconnected", 0);
		else
			printf(format, "disconnected", 0);
		goto finish;
	}

	int q = (100*stats.qual.qual) / max_qual;
	if (xtended) {
		path[0] = '\0';
		int s = q / 25 + ((q % 25) > 12);
		for (int i = 0; i < s; i++)
			strncat(path, strength[i], sizeof(path) - strlen(path) - 1);
		for (int i = s; i < 4; i++)
			strncat(path, " ", sizeof(path) - strlen(path) - 1);
		printf(format, path, name, q);
	} else {
		printf(format, name, q);
	}

finish:
	putchar('\n');
	fflush(stdout);
}

int main(int argc, char *argv[])
{
	char *wifi_interface = WIFI_INTERFACE;
	char *eth_interface = ETH_INTERFACE;
	bool snoop = false;
	bool xtended = false;

	int opt;
	while ((opt = getopt(argc, argv, "hsxf:i:w:e:")) != -1) {
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
		case 'x':
			xtended = true;
			format = XFORMAT;
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
			network_info(fd, wifi_interface, eth_interface, xtended);
			sleep(interval);
			name[0] = '\0';
		}
	else
		network_info(fd, wifi_interface, eth_interface, xtended);

	close(fd);
	if (strlen(name) > 0)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
