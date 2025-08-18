#ifndef RUNNER_H
#define RUNNER_H

#include "options.h"

/**
 * @brief Run the application with the provided options.
 *
 * @param options Runtime configuration.
 * @return Program exit status.
 */
int run(const options_t *options);

#endif // RUNNER_H
