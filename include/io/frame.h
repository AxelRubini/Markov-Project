#ifndef FRAME_H
#define FRAME_H

#include <stddef.h>
#include <sys/types.h>

int send_frame(int fd, const char *data_to_send, size_t len);
ssize_t recv_frame(int fd, char **buf, size_t *cap);

#endif // FRAME_H
