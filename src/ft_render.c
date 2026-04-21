#include "fdf.h"

/*
** ft_line_init — populate a t_line from two projected points.
**
** Bresenham works entirely in integer screen pixels.  We round
** p0/p1 coordinates here so the rest of the algorithm stays
** branch-free and portable.
*/
static t_line	ft_line_init(t_point p0, t_point p1)
{
	t_line	l;

	l.x = (int)round(p0.x);
	l.y = (int)round(p0.y);
	l.dx = abs((int)round(p1.x) - l.x);
	l.dy = abs((int)round(p1.y) - l.y);
	l.sx = (p1.x > p0.x) ? 1 : -1;
	l.sy = (p1.y > p0.y) ? 1 : -1;
	l.err = l.dx - l.dy;
	l.steps = (l.dx > l.dy) ? l.dx : l.dy;
	l.c0 = p0.color;
	l.c1 = p1.color;
	return (l);
}

/*
** ft_draw_line — Bresenham line algorithm with linear color interpolation.
**
** Algorithm summary
** ─────────────────
**   Given two screen-space integers (x0,y0) and (x1,y1) the algorithm
**   avoids floating-point by maintaining an error accumulator `err`.
**
**   Each iteration:
**     1. Paint current pixel with interpolated color.
**     2. Double the error term and decide which axis to advance:
**        • if 2*err > -dy  → step along X, subtract dy from err
**        • if 2*err <  dx  → step along Y, add    dx to err
**   Both conditions can trigger in the same iteration (diagonal step).
**
**   Color at step i = lerp(c0, c1, i / total_steps).
*/
void	ft_draw_line(t_fdf *fdf, t_point p0, t_point p1)
{
	t_line	l;
	int		e2;
	int		i;
	double	t;

	l = ft_line_init(p0, p1);
	i = 0;
	while (1)
	{
		t = (l.steps > 0) ? (double)i / (double)l.steps : 0.0;
		ft_pixel_put(&fdf->img, l.x, l.y, ft_lerp_color(l.c0, l.c1, t));
		if (l.x == (int)round(p1.x) && l.y == (int)round(p1.y))
			break ;
		e2 = 2 * l.err;
		if (e2 > -l.dy)
		{
			l.err -= l.dy;
			l.x += l.sx;
		}
		if (e2 < l.dx)
		{
			l.err += l.dx;
			l.y += l.sy;
		}
		i++;
	}
}

void	ft_render(t_fdf *fdf)
{
	int		row;
	int		col;
	t_point	curr;
	t_point	next;

	ft_bzero(fdf->img.addr,
		WIN_W * WIN_H * (fdf->img.bpp / 8));
	row = 0;
	while (row < fdf->map->rows)
	{
		col = 0;
		while (col < fdf->map->cols)
		{
			curr = ft_project(col, row, fdf);
			if (col + 1 < fdf->map->cols)
			{
				next = ft_project(col + 1, row, fdf);
				ft_draw_line(fdf, curr, next);
			}
			if (row + 1 < fdf->map->rows)
			{
				next = ft_project(col, row + 1, fdf);
				ft_draw_line(fdf, curr, next);
			}
			col++;
		}
		row++;
	}
	mlx_put_image_to_window(fdf->mlx, fdf->win, fdf->img.ptr, 0, 0);
}
