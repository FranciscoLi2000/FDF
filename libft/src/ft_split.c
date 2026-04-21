#include "libft.h"

static size_t count_words(char const *s, char c)
{
size_t count;

count = 0;
while (*s)
{
while (*s == c)
s++;
if (*s)
{
count++;
while (*s && *s != c)
s++;
}
}
return (count);
}

static size_t word_len(char const *s, char c)
{
size_t len;

len = 0;
while (s[len] && s[len] != c)
len++;
return (len);
}

static char	**free_words(char **arr, size_t i)
{
while (i > 0)
{
i--;
free(arr[i]);
}
free(arr);
return (NULL);
}

static char	**fill_words(char **arr, char const *s, char c)
{
size_t i;
size_t len;

i = 0;
while (*s)
{
while (*s == c)
s++;
if (*s)
{
len = word_len(s, c);
arr[i] = ft_substr(s, 0, len);
if (!arr[i])
return (free_words(arr, i));
i++;
s += len;
}
}
arr[i] = NULL;
return (arr);
}

char	**ft_split(char const *s, char c)
{
char	**arr;
size_t count;

if (!s)
return (NULL);
count = count_words(s, c);
arr = (char **)malloc(sizeof(char *) * (count + 1));
if (!arr)
return (NULL);
return (fill_words(arr, s, c));
}
