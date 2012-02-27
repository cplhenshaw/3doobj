#include <stddef.h>

void *checked_malloc(size_t size);
void *checked_realloc(void *ptr, size_t size);
void *checked_calloc(size_t nmemb, size_t size);
