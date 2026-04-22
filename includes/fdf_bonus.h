#ifndef FDF_BONUS_H
# define FDF_BONUS_H

# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <math.h>
# include "../minilibx_linux/mlx.h"
# include "../libft/includes/libft.h"

/* ── Window ── */
# define WIN_W			1280
# define WIN_H			960
# define WIN_TITLE		"FdF Bonus"

/* ── Linux X11 key codes ── */
# define KEY_ESC		65307
# define KEY_W			119
# define KEY_A			97
# define KEY_S			115
# define KEY_D			100
# define KEY_Q			113
# define KEY_E			101
# define KEY_R			114
# define KEY_PLUS		61
# define KEY_MINUS		45
# define KEY_I			105
# define KEY_P			112
# define KEY_J			106
# define KEY_K			107
# define KEY_U			117
# define KEY_O			111
# define KEY_C			99
# define KEY_G			103

/* ── MLX mouse event numbers (X11) ── */
# define EVT_BTN_PRESS		4
# define EVT_BTN_RELEASE	5
# define EVT_MOUSE_MOVE		6

/* ── Mouse button codes ── */
# define BTN_LEFT		1
# define BTN_SCROLL_UP		4
# define BTN_SCROLL_DOWN	5

/* ── Projection modes ── */
# define ISO		0
# define PARALLEL	1
# define CONIC		2

/* ── Color schemes ── */
# define SCHEME_FIRE	0
# define SCHEME_GREY	1
# define SCHEME_HEAT	2

/* ── Color palette endpoints ── */
# define FIRE_LOW	0x0000FF
# define FIRE_HIGH	0xFF4500
# define GREY_LOW	0x303030
# define GREY_HIGH	0xFFFFFF
# define HEAT_LOW	0x00FFFF
# define HEAT_HIGH	0xFFFF00

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
	double	x_rot;
	double	y_rot;
	double	z_rot;
	int		proj;
	int		color_scheme;
}	t_cam;

typedef struct s_img
{
	void	*ptr;
	char	*addr;
	int		bpp;
	int		ll;
	int		endian;
}	t_img;

typedef struct s_mouse
{
	int		drag;
	int		prev_x;
	int		prev_y;
}	t_mouse;

typedef struct s_fdf
{
	void	*mlx;
	void	*win;
	t_img	img;
	t_map	*map;
	t_cam	cam;
	t_mouse	mouse;
}	t_fdf;

/* Helper used inside ft_draw_line (Bresenham state) */
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

/* ft_parse_bonus.c */
t_map	*ft_parse_map(const char *file);
void	ft_free_map(t_map *map);
int		ft_hex_to_int(const char *hex);

/* ft_project_bonus.c */
t_point	ft_project(int col, int row, t_fdf *fdf);
void	ft_init_cam(t_fdf *fdf);

/* ft_render_bonus.c */
void	ft_render(t_fdf *fdf);
void	ft_draw_line(t_fdf *fdf, t_point p0, t_point p1);

/* ft_hooks_bonus.c */
int		ft_keypress(int key, t_fdf *fdf);
int		ft_mouse_press(int btn, int x, int y, t_fdf *fdf);
int		ft_mouse_release(int btn, int x, int y, t_fdf *fdf);
int		ft_mouse_move(int x, int y, t_fdf *fdf);
int		ft_close(t_fdf *fdf);

/* ft_utils_bonus.c */
void	ft_error(const char *msg);
void	ft_free_fdf(t_fdf *fdf);
int		ft_lerp_color(int c1, int c2, double t);
void	ft_pixel_put(t_img *img, int x, int y, int color);
int		ft_scheme_color(int z, t_map *map, int scheme);

#endif
