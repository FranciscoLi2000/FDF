#include "libft.h"

void *ft_memmove(void *dst, const void *src, size_t len)
{
size_t i;
unsigned char *d;
unsigned char *s;

if (!dst && !src)
return (NULL);
d = (unsigned char *)dst;
s = (unsigned char *)src;
if (d > s)
{
i = len;
while (i > 0)
{
i--;
d[i] = s[i];
}
}
else
ft_memcpy(dst, src, len);
return (dst);
}
