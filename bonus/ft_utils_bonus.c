#include "fdf_bonus.h"

void	ft_error(const char *msg)
{
	ft_putstr_fd("Error: ", 2);
	ft_putendl_fd((char *)msg, 2);
	exit(EXIT_FAILURE);
}

void	ft_free_fdf(t_fdf *fdf)
{
	if (!fdf)
		return ;
	if (fdf->img.ptr)
		mlx_destroy_image(fdf->mlx, fdf->img.ptr);
	if (fdf->win)
		mlx_destroy_window(fdf->mlx, fdf->win);
	if (fdf->mlx)
	{
		mlx_destroy_display(fdf->mlx);
		free(fdf->mlx);
	}
	ft_free_map(fdf->map);
	free(fdf);
}

int	ft_lerp_color(int c1, int c2, double t)
{
	int	r;
	int	g;
	int	b;

	r = (int)((1 - t) * ((c1 >> 16) & 0xFF) + t * ((c2 >> 16) & 0xFF));
	g = (int)((1 - t) * ((c1 >> 8) & 0xFF) + t * ((c2 >> 8) & 0xFF));
	b = (int)((1 - t) * (c1 & 0xFF) + t * (c2 & 0xFF));
	return ((r << 16) | (g << 8) | b);
}

void	ft_pixel_put(t_img *img, int x, int y, int color)
{
	char	*dst;

	if (x < 0 || x >= WIN_W || y < 0 || y >= WIN_H)
		return ;
	dst = img->addr + (y * img->ll + x * (img->bpp / 8));
	*(unsigned int *)dst = color;
}

int	ft_scheme_color(int z, t_map *map, int scheme)
{
	double	t;
	int		low;
	int		high;

	if (map->z_max == map->z_min)
		return (0xFFFFFF);
	t = (double)(z - map->z_min) / (double)(map->z_max - map->z_min);
	if (scheme == SCHEME_GREY)
	{
		low = GREY_LOW;
		high = GREY_HIGH;
	}
	else if (scheme == SCHEME_HEAT)
	{
		low = HEAT_LOW;
		high = HEAT_HIGH;
	}
	else
	{
		low = FIRE_LOW;
		high = FIRE_HIGH;
	}
	return (ft_lerp_color(low, high, t));
}
