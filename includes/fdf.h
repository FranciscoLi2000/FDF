#ifndef FDF_H
# define FDF_H

# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <math.h>
# include "../minilibx_linux/mlx.h"
# include "../libft/includes/libft.h"

/* ── Window ── */
# define WIN_W		1280
# define WIN_H		960
# define WIN_TITLE	"FdF"

/* ── Linux X11 key codes ── */
# define KEY_ESC	65307
# define KEY_W		119
# define KEY_A		97
# define KEY_S		115
# define KEY_D		100
# define KEY_Q		113
# define KEY_E		101
# define KEY_R		114
# define KEY_PLUS	61
# define KEY_MINUS	45
# define KEY_I		105
# define KEY_P		112

/* ── Projection modes ── */
# define ISO		0
# define PARALLEL	1

/* ── Default height-gradient colors ── */
# define C_LOW		0x0000FF
# define C_HIGH		0xFF4500

/* ── Camera step sizes ── */
# define MOVE_STEP	20.0
# define ROT_STEP	0.05
# define ZOOM_STEP	1.1

typedef struct s_point
{
	double	x;
	double	y;
	double	z;
	int		color;
}	t_point;

typedef struct s_map
{
	int		rows;
	int		cols;
	int		**z;
	int		**color;
	char	**has_color;
	int		z_max;
	int		z_min;
}	t_map;

typedef struct s_cam
{
	double	zoom;
	double	x_off;
	double	y_off;
	double	z_rot;
	int		proj;
}	t_cam;

typedef struct s_img
{
	void	*ptr;
	char	*addr;
	int		bpp;
	int		ll;
	int		endian;
}	t_img;

typedef struct s_fdf
{
	void	*mlx;
	void	*win;
	t_img	img;
	t_map	*map;
	t_cam	cam;
}	t_fdf;

/* Helper used only inside ft_draw_line (Bresenham state) */
typedef struct s_line
{
	int		x;
	int		y;
	int		dx;
	int		dy;
	int		sx;
	int		sy;
	int		err;
	int		steps;
	int		c0;
	int		c1;
}	t_line;

/* ft_parse.c */
t_map	*ft_parse_map(const char *file);
void	ft_free_map(t_map *map);

/* ft_project.c */
t_point	ft_project(int col, int row, t_fdf *fdf);
void	ft_init_cam(t_fdf *fdf);

/* ft_render.c */
void	ft_render(t_fdf *fdf);
void	ft_draw_line(t_fdf *fdf, t_point p0, t_point p1);

/* ft_hooks.c */
int		ft_keypress(int key, t_fdf *fdf);
int		ft_close(t_fdf *fdf);

/* ft_utils.c */
void	ft_error(const char *msg);
void	ft_free_fdf(t_fdf *fdf);
int		ft_lerp_color(int c1, int c2, double t);
void	ft_pixel_put(t_img *img, int x, int y, int color);
int		ft_hex_to_int(const char *hex);

#endif
