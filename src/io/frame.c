#include "frame.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <unistd.h>

/**
 * @brief Write all bytes from buf to the file descriptor fd.
 *
 * @param fd File descriptor to write to.
 * @param buf Buffer containing data to write.
 * @param len Length of data to write.
 * @return 0 on success, -1 on error.
 */

static int write_all(int fd, const void *buf, size_t len)
{
	const char *ptr = buf;
	size_t left = len;
	while (left > 0) {
		ssize_t written = write(fd, ptr, left);
		if (written < 0) {
			if (errno == EINTR) { // Interrupted by a signal, retry
				continue;
			}
			perror("write_all");
			return -1;
		}
		left -= written;
		ptr += written;
	}
	return 0;
}

/**
 * @brief Read all bytes from the file descriptor fd into buf.
 *
 * @param fd File descriptor to read from.
 * @param buf Buffer to store readed data.
 * @param len Length of data to read.
 * @return number of bytes read on success, 0 on EOF, -1 on error.
 */
static ssize_t read_all(int fd, void *buf, size_t len)
{
	char *ptr = buf;
	size_t left = len;
	while (left > 0) {
		ssize_t read_bytes = read(fd, ptr, left);
		if (read_bytes < 0) {
			if (errno == EINTR) { // Interrupted by a signal, retry
				continue;
			}
			perror("read_all");
			return -1;
		}
		if (read_bytes == 0) {
			break;
		}
		left -= read_bytes;
		ptr += read_bytes;
	}
	return (ssize_t)(len - left);
}

int send_frame(int fd, const char *data_to_send, size_t len)
{
	if (!data_to_send && len != 0) {
		fprintf(stderr, "Invalid data to send or length is zero\n");
		return -1;
	}
	uint32_t frame_length = htonl((uint32_t)len);
	if (write_all(fd, &frame_length, sizeof(frame_length)) < 0) {
		perror("send_frame: write frame length");
		return -1;
	}
	if (len == 0) {
		return 0;
	}
	if (write_all(fd, data_to_send, len) < 0) {
		perror("send_frame: write data");
		return -1;
	}
	return 0;
}

ssize_t recv_frame(int fd, char **buf, size_t *cap)
{
	if (!buf || !cap) {
		fprintf(stderr, "Invalid arguments to recv_frame\n");
		return -1;
	}
	uint32_t be_len = 0;
	ssize_t st = read_all(fd, &be_len, sizeof(be_len));
	if (st == 0) {
		return 0;
	}
	if (st < (ssize_t)sizeof(be_len)) {
		return 0;
	}
	if (st < 0) {
		return -1;
	}

	size_t need = (size_t)ntohl(be_len);
	if (*buf == NULL || *cap < need + 1) {
		size_t ncap = need + 1;
		char *nb = realloc(*buf, ncap);
		if (!nb) {
			return -1;
		}
		*buf = nb;
		*cap = ncap;
	}
	if (need == 0) {
		(*buf)[0] = '\0';
		return 0;
	}
	st = read_all(fd, *buf, need);
	if (st < 0) {
		return -1;
	}
	if ((size_t)st < need) {
		return -1;
	}

	(*buf)[need] = '\0';
	return (ssize_t)need;
}
