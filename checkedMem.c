#include "checkedMem.h"
#include <stdlib.h>
#include <stdio.h>

/* A quick function to check the result of calls to malloc, terminating if they fail. */
void *checked_malloc(size_t size) 
{
	void *mem = malloc( size );
	if( mem == NULL )
	{
		fprintf(stderr, "Call to malloc failed.\n");
		fprintf(stderr, "Terminating program.\n");
		exit(EXIT_FAILURE);
	}
	else return mem;

}
/* Ditto for realloc */
void *checked_realloc(void *ptr, size_t size)
{
	void *mem = realloc(ptr, size);
	if(mem == NULL)
	{
		fprintf(stderr, "Call to realloc failed.\n");
		fprintf(stderr, "Terminating program.\n");
		exit(EXIT_FAILURE);
	}	

	return mem;
}

/* Ditto for calloc */
void *checked_calloc(size_t nmemb, size_t size)
{
	void *mem = calloc(nmemb, size);
	if(mem == NULL)
	{
		fprintf(stderr, "Call to calloc failed.\n");
		fprintf(stderr, "Terminating program.\n");
		exit(EXIT_FAILURE);
	}
	
	return mem;
}


