#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <stdio.h>
#include "lepton.h"

void get_vars(int fd) {
	lepton_iotcl_t q;

	if (ioctl(fd, QUERY_GET_VARIABLES, &q) == -1) {
		perror("query_apps ioctl get");
	} else {
		printf("num_transfers : %d\n", q.num_transfers);
		printf("transfer_size: %d\n", q.transfer_size);
		printf("loopback_mode: %d\n", q.loopback_mode);
	}
}

int main(int argc, char **argv) {
	int fd;
	lepton_iotcl_t q;

	if (argc < 4) {
		printf("Usage:\n%s [packet_size] [repeats] [quiet:1/0]\n", argv[0]);
		exit(1);
	}

	fd = open("/dev/lepton", O_RDWR);
	if (fd <= 0) {
		printf("Device not found /dev/lepton\n");
		exit(1);
	}


	q.transfer_size = atoi(argv[1]);
	q.num_transfers = atoi(argv[2]);
	q.quiet = (atoi(argv[3]) == 1) ? true : false;
	q.loopback_mode = false;

	if (ioctl(fd, QUERY_SET_VARIABLES, &q) == -1) {
		perror("query_apps ioctl set");
	}
	ioctl(fd, LEPTON_IOCTL_TRANSFER, 0);
	return 0;
}
