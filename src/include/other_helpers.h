#ifndef OTHER_HELPERS_H_
#define OTHER_HELPERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>

// for handling errors
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFFLEN 1600
#define SMALL_BUFF 10

#endif