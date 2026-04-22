#include "libft.h"

char *ft_strjoin(char const *s1, char const *s2)
{
char *join;
size_t len;

if (!s1 || !s2)
return (NULL);
len = ft_strlen(s1) + ft_strlen(s2) + 1;
join = (char *)malloc(len);
if (!join)
return (NULL);
ft_strlcpy(join, s1, len);
ft_strlcat(join, s2, len);
return (join);
}
