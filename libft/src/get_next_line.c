#include "get_next_line.h"

static size_t	gnl_strlen(const char *s)
{
	size_t	n;

	n = 0;
	if (!s)
		return (0);
	while (s[n])
		n++;
	return (n);
}

static char	*gnl_strjoin(char *stash, char *buf, ssize_t n)
{
	size_t	sl;
	size_t	i;
	char	*res;

	sl = gnl_strlen(stash);
	res = malloc(sl + n + 1);
	if (!res)
		return (NULL);
	i = 0;
	while (stash && i < sl)
	{
		res[i] = stash[i];
		i++;
	}
	i = 0;
	while (i < (size_t)n)
	{
		res[sl + i] = buf[i];
		i++;
	}
	res[sl + i] = '\0';
	return (res);
}

static char	*extract_line(const char *stash)
{
	size_t	len;
	size_t	i;
	char	*line;

	len = 0;
	while (stash[len] && stash[len] != '\n')
		len++;
	if (stash[len] == '\n')
		len++;
	line = malloc(len + 1);
	if (!line)
		return (NULL);
	i = 0;
	while (i < len)
	{
		line[i] = stash[i];
		i++;
	}
	line[i] = '\0';
	return (line);
}

static char	*update_stash(char *stash)
{
	size_t	i;
	size_t	len;
	size_t	j;
	char	*ns;

	i = 0;
	while (stash[i] && stash[i] != '\n')
		i++;
	if (!stash[i])
	{
		free(stash);
		return (NULL);
	}
	i++;
	len = gnl_strlen(stash + i);
	if (len == 0)
	{
		free(stash);
		return (NULL);
	}
	ns = malloc(len + 1);
	if (!ns)
	{
		free(stash);
		return (NULL);
	}
	j = 0;
	while (j < len)
	{
		ns[j] = stash[i + j];
		j++;
	}
	ns[j] = '\0';
	free(stash);
	return (ns);
}

char	*get_next_line(int fd)
{
	static char	*stash;
	char		buf[BUFFER_SIZE];
	ssize_t		bytes;
	size_t		i;
	char		*tmp;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	while (1)
	{
		i = 0;
		while (stash && stash[i] && stash[i] != '\n')
			i++;
		if (stash && stash[i] == '\n')
			break ;
		bytes = read(fd, buf, BUFFER_SIZE);
		if (bytes <= 0)
			break ;
		tmp = gnl_strjoin(stash, buf, bytes);
		free(stash);
		stash = tmp;
	}
	if (!stash || stash[0] == '\0')
	{
		free(stash);
		stash = NULL;
		return (NULL);
	}
	tmp = extract_line(stash);
	stash = update_stash(stash);
	return (tmp);
}
