#include "fdf.h"

void	ft_init_cam(t_fdf *fdf)
{
	double	span;

	span = (fdf->map->cols + fdf->map->rows) * cos(M_PI / 6.0);
	if (span < 1.0)
		span = 1.0;
	fdf->cam.zoom = (WIN_W < WIN_H ? WIN_W : WIN_H) * 0.75 / span;
	if (fdf->cam.zoom < 1.0)
		fdf->cam.zoom = 1.0;
	fdf->cam.x_off = WIN_W / 2.0;
	fdf->cam.y_off = WIN_H / 2.0;
	fdf->cam.z_rot = 0.0;
	fdf->cam.proj = ISO;
}

static double	ft_z_scale(t_fdf *fdf, int row, int col)
{
	int		z_range;
	int		map_span;
	double	scale;

	z_range = fdf->map->z_max - fdf->map->z_min;
	if (z_range == 0)
		return (0.0);
	map_span = fdf->map->cols > fdf->map->rows
		? fdf->map->cols : fdf->map->rows;
	scale = fdf->cam.zoom * map_span * 0.4 / z_range;
	return ((double)(fdf->map->z[row][col] - fdf->map->z_min) * scale);
}

static int	ft_get_color(int col, int row, t_fdf *fdf)
{
	double	t;

	if (fdf->map->has_color[row][col])
		return (fdf->map->color[row][col]);
	if (fdf->map->z_max == fdf->map->z_min)
		return (0xFFFFFF);
	t = (double)(fdf->map->z[row][col] - fdf->map->z_min)
		/ (double)(fdf->map->z_max - fdf->map->z_min);
	return (ft_lerp_color(C_LOW, C_HIGH, t));
}

t_point	ft_project(int col, int row, t_fdf *fdf)
{
	t_point	p;
	double	x;
	double	y;
	double	z;
	double	cos_r;
	double	sin_r;
	double	xr;
	double	yr;

	x = col - fdf->map->cols / 2.0;
	y = row - fdf->map->rows / 2.0;
	z = ft_z_scale(fdf, row, col);
	cos_r = cos(fdf->cam.z_rot);
	sin_r = sin(fdf->cam.z_rot);
	xr = x * cos_r - y * sin_r;
	yr = x * sin_r + y * cos_r;
	if (fdf->cam.proj == ISO)
	{
		p.x = (xr - yr) * cos(M_PI / 6.0) * fdf->cam.zoom + fdf->cam.x_off;
		p.y = (xr + yr) * sin(M_PI / 6.0) * fdf->cam.zoom - z + fdf->cam.y_off;
	}
	else
	{
		p.x = xr * fdf->cam.zoom + fdf->cam.x_off;
		p.y = yr * fdf->cam.zoom + fdf->cam.y_off;
	}
	p.z = z;
	p.color = ft_get_color(col, row, fdf);
	return (p);
}
