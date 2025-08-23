#include "options.h"
#include "runner.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	options_t options = {0};
	if (parse_options(argc, argv, &options) < 0) {
		fprintf(stderr, "Error parsing options\n");
		return EXIT_FAILURE;
	}
	int result = run(&options);
	return result < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
