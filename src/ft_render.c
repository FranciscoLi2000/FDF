#include "fdf.h"

void	ft_draw_line(t_fdf *fdf, t_point p0, t_point p1)
{
	/*
	** ⚠️  TODO — Bresenham's line algorithm
	** This function will be implemented in the next step.
	** It draws a straight line from p0 to p1 on the MLX image buffer,
	** interpolating the color between p0.color and p1.color.
	*/
	(void)fdf;
	(void)p0;
	(void)p1;
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
