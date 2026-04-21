#include "libft.h"

int	ft_numlen(long n)
{
	int	len;

	if (n < 0)
		n = -n;
	len = 1;
	while (n >= 10)
	{
		n /= 10;
		len++;
	}
	return (len);
}
