#ifndef FRAME_H
#define FRAME_H

#include <stddef.h>
#include <sys/types.h>

/**
 * @brief Send a length-prefixed frame over a file descriptor.
 *
 * The function first writes the length of the frame as a 32-bit value in
 * network byte order, followed by the frame data itself.
 *
 * @param fd           File descriptor to which the frame is written.
 * @param data_to_send Pointer to the data buffer; may be NULL when @p len is 0.
 * @param len          Number of bytes to send from @p data_to_send.
 *
 * @return 0 on success; -1 if writing fails or arguments are invalid.
 */
int send_frame(int fd, const char *data_to_send, size_t len);

/**
 * @brief Receive a length-prefixed frame from a file descriptor.
 *
 * The buffer pointed to by @p buf is resized as needed to hold the incoming
 * data plus a null terminator.
 *
 * @param fd  File descriptor from which to read the frame.
 * @param buf Address of a buffer pointer that will hold the frame data.
 *            The buffer is allocated or reallocated by the function.
 * @param cap Address of a variable containing the capacity of @p buf, updated
 *            when reallocation occurs.
 *
 * @return Number of bytes read on success; 0 on EOF or empty frame; -1 on
 *         error.
 */
ssize_t recv_frame(int fd, char **buf, size_t *cap);

#endif // FRAME_H
