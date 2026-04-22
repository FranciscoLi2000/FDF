#include "fdf_bonus.h"

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
** ft_draw_line — Bresenham algorithm with linear color interpolation.
** Identical logic to the mandatory version; the struct types differ
** because this compilation unit uses fdf_bonus.h.
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

static void	ft_draw_edges(t_fdf *fdf, int col, int row)
{
	t_point	curr;
	t_point	next;

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
}

/*
** ft_hud_state — render dynamic state lines (zoom, projection, palette).
** Called by ft_draw_hud after the static control-hint lines.
*/
static void	ft_hud_state(t_fdf *fdf)
{
	char	*proj_s;
	char	*col_s;
	char	*zoom_s;
	char	*zoom_l;

	if (fdf->cam.proj == ISO)
		proj_s = "Mode: ISO";
	else if (fdf->cam.proj == PARALLEL)
		proj_s = "Mode: PARALLEL";
	else
		proj_s = "Mode: CONIC";
	if (fdf->cam.color_scheme == SCHEME_GREY)
		col_s = "Color: GREY";
	else if (fdf->cam.color_scheme == SCHEME_HEAT)
		col_s = "Color: HEAT";
	else
		col_s = "Color: FIRE";
	zoom_s = ft_itoa((int)fdf->cam.zoom);
	if (!zoom_s)
		return ;
	zoom_l = ft_strjoin("Zoom: ", zoom_s);
	free(zoom_s);
	if (zoom_l)
	{
		mlx_string_put(fdf->mlx, fdf->win, 10, 60, 0xFFFF00, zoom_l);
		free(zoom_l);
	}
	mlx_string_put(fdf->mlx, fdf->win, 10, 80, 0xFFFF00, proj_s);
	mlx_string_put(fdf->mlx, fdf->win, 10, 100, 0xFFFF00, col_s);
}

/*
** ft_draw_hud — overlay the on-screen help text and live state info.
** Text is placed with mlx_string_put, which draws on top of the image
** (no pixel-buffer involvement, so no ft_pixel_put clipping needed).
*/
static void	ft_draw_hud(t_fdf *fdf)
{
	mlx_string_put(fdf->mlx, fdf->win, 10, 20, 0xAAAAAA,
		"WASD:pan  QE:spin  JK:tilt  UO:yaw  +-:zoom  R:reset");
	mlx_string_put(fdf->mlx, fdf->win, 10, 40, 0xAAAAAA,
		"I:iso  P:parallel  C:conic  G:color  drag:rotate  ESC:quit");
	ft_hud_state(fdf);
}

void	ft_render(t_fdf *fdf)
{
	int	row;
	int	col;

	ft_bzero(fdf->img.addr, WIN_W * WIN_H * (fdf->img.bpp / 8));
	row = 0;
	while (row < fdf->map->rows)
	{
		col = 0;
		while (col < fdf->map->cols)
			ft_draw_edges(fdf, col++, row);
		row++;
	}
	mlx_put_image_to_window(fdf->mlx, fdf->win, fdf->img.ptr, 0, 0);
	ft_draw_hud(fdf);
}
