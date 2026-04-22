#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t dstsize)
{
size_t dlen;
size_t slen;
size_t i;

dlen = ft_strlen(dst);
slen = ft_strlen(src);
if (dstsize <= dlen)
return (dstsize + slen);
i = 0;
while (src[i] && (dlen + i + 1) < dstsize)
{
dst[dlen + i] = src[i];
i++;
}
dst[dlen + i] = '\0';
return (dlen + slen);
}
