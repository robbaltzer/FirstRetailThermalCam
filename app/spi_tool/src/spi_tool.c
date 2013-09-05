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
		printf("loopback_mode    : %d\n", q.loopback_mode);
	}
}

int main(int argc, char **argv) {
	int fd;
//	char wr_buf[]={0xff,0x00,0x1f,0x0f};
//	char rd_buf[10];;

	if (argc < 2) {
		printf("Usage:\n%s [device]\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDWR);
	if (fd <= 0) {
		printf("%s: Device %s not found\n", argv[0], argv[1]);
		exit(1);
	}

	ioctl(fd, LEPTON_IOCTL_TRANSFER, 0);

	get_vars(fd);
	return 0;
}
