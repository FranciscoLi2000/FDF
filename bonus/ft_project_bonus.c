#include "fdf_bonus.h"

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
	fdf->cam.x_rot = 0.0;
	fdf->cam.y_rot = 0.0;
	fdf->cam.z_rot = 0.0;
	fdf->cam.proj = ISO;
	fdf->cam.color_scheme = SCHEME_FIRE;
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
	if (fdf->map->has_color[row][col])
		return (fdf->map->color[row][col]);
	if (fdf->map->z_max == fdf->map->z_min)
		return (0xFFFFFF);
	return (ft_scheme_color(fdf->map->z[row][col],
			fdf->map, fdf->cam.color_scheme));
}

/*
** ft_rotate_zx — apply Z-axis spin then X-axis tilt in place.
** Both rotations share a single trig evaluation pass to keep
** the function under 25 lines while staying branch-free.
*/
static void	ft_rotate_zx(double *x, double *y, double *z, t_cam *cam)
{
	double	xz;
	double	yz;
	double	yx;
	double	zx;

	xz = *x * cos(cam->z_rot) - *y * sin(cam->z_rot);
	yz = *x * sin(cam->z_rot) + *y * cos(cam->z_rot);
	yx = yz * cos(cam->x_rot) - *z * sin(cam->x_rot);
	zx = yz * sin(cam->x_rot) + *z * cos(cam->x_rot);
	*x = xz;
	*y = yx;
	*z = zx;
}

/* ft_rotate_y — apply Y-axis tilt in place. */
static void	ft_rotate_y(double *x, double *z, t_cam *cam)
{
	double	xr;
	double	zr;

	xr = *x * cos(cam->y_rot) + *z * sin(cam->y_rot);
	zr = -*x * sin(cam->y_rot) + *z * cos(cam->y_rot);
	*x = xr;
	*z = zr;
}

/*
** ft_apply_proj — map rotated 3-D coordinates to 2-D screen space.
**   ISO    — classic 30° isometric (same formula as mandatory)
**   PARALLEL — straight top-down orthographic
**   CONIC  — cavalier oblique at 30°, depth scale 0.5
**             (z is projected onto a 30° axis with half its length,
**             giving a "printed 3-D" look different from ISO)
*/
static void	ft_apply_proj(t_point *p, double x, double y,
				double z, t_fdf *fdf)
{
	double	zoom;
	double	c30;

	zoom = fdf->cam.zoom;
	c30 = cos(M_PI / 6.0);
	if (fdf->cam.proj == ISO)
	{
		p->x = (x - y) * c30 * zoom + fdf->cam.x_off;
		p->y = (x + y) * sin(M_PI / 6.0) * zoom - z + fdf->cam.y_off;
	}
	else if (fdf->cam.proj == PARALLEL)
	{
		p->x = x * zoom + fdf->cam.x_off;
		p->y = y * zoom + fdf->cam.y_off;
	}
	else
	{
		p->x = x * zoom + z * 0.5 * c30 + fdf->cam.x_off;
		p->y = y * zoom - z * 0.5 * sin(M_PI / 6.0) + fdf->cam.y_off;
	}
	p->z = z;
}

t_point	ft_project(int col, int row, t_fdf *fdf)
{
	t_point	p;
	double	x;
	double	y;
	double	z;

	x = col - fdf->map->cols / 2.0;
	y = row - fdf->map->rows / 2.0;
	z = ft_z_scale(fdf, row, col);
	ft_rotate_zx(&x, &y, &z, &fdf->cam);
	ft_rotate_y(&x, &z, &fdf->cam);
	p.color = ft_get_color(col, row, fdf);
	ft_apply_proj(&p, x, y, z, fdf);
	return (p);
}
