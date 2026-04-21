#include "libft.h"

void *ft_calloc(size_t count, size_t size)
{
void *ptr;

if (size != 0 && count > ((size_t)-1 / size))
return (NULL);
ptr = malloc(count * size);
if (!ptr)
return (NULL);
ft_bzero(ptr, count * size);
return (ptr);
}
