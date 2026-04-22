#include "fdf_bonus.h"

/*
** ft_key_move — handle pan and zoom keys.
** Returns without re-rendering; the caller (ft_keypress) does that.
*/
static void	ft_key_move(int key, t_fdf *fdf)
{
	if (key == KEY_W)
		fdf->cam.y_off -= MOVE_STEP;
	else if (key == KEY_S)
		fdf->cam.y_off += MOVE_STEP;
	else if (key == KEY_A)
		fdf->cam.x_off -= MOVE_STEP;
	else if (key == KEY_D)
		fdf->cam.x_off += MOVE_STEP;
	else if (key == KEY_PLUS)
		fdf->cam.zoom *= ZOOM_STEP;
	else if (key == KEY_MINUS)
		fdf->cam.zoom /= ZOOM_STEP;
}

/* ft_key_rot — handle all three rotation axes. */
static void	ft_key_rot(int key, t_fdf *fdf)
{
	if (key == KEY_Q)
		fdf->cam.z_rot -= ROT_STEP;
	else if (key == KEY_E)
		fdf->cam.z_rot += ROT_STEP;
	else if (key == KEY_J)
		fdf->cam.x_rot += ROT_STEP;
	else if (key == KEY_K)
		fdf->cam.x_rot -= ROT_STEP;
	else if (key == KEY_U)
		fdf->cam.y_rot -= ROT_STEP;
	else if (key == KEY_O)
		fdf->cam.y_rot += ROT_STEP;
}

/*
** ft_key_proj — cycle projection mode and color scheme.
**   I → ISO, P → PARALLEL, C → CONIC
**   G → cycle through FIRE / GREY / HEAT palettes
*/
static void	ft_key_proj(int key, t_fdf *fdf)
{
	if (key == KEY_I)
		fdf->cam.proj = ISO;
	else if (key == KEY_P)
		fdf->cam.proj = PARALLEL;
	else if (key == KEY_C)
		fdf->cam.proj = CONIC;
	else if (key == KEY_G)
		fdf->cam.color_scheme = (fdf->cam.color_scheme + 1) % 3;
}

int	ft_keypress(int key, t_fdf *fdf)
{
	if (key == KEY_ESC)
		ft_close(fdf);
	if (key == KEY_R)
		ft_init_cam(fdf);
	ft_key_move(key, fdf);
	ft_key_rot(key, fdf);
	ft_key_proj(key, fdf);
	ft_render(fdf);
	return (0);
}

int	ft_close(t_fdf *fdf)
{
	ft_free_fdf(fdf);
	exit(EXIT_SUCCESS);
}

/*
** ft_mouse_press — scroll wheel zooms; left-click starts a drag.
** We return early on drag start to avoid an unnecessary re-render.
*/
int	ft_mouse_press(int btn, int x, int y, t_fdf *fdf)
{
	if (btn == BTN_SCROLL_UP)
		fdf->cam.zoom *= ZOOM_STEP;
	else if (btn == BTN_SCROLL_DOWN)
		fdf->cam.zoom /= ZOOM_STEP;
	else if (btn == BTN_LEFT)
	{
		fdf->mouse.drag = 1;
		fdf->mouse.prev_x = x;
		fdf->mouse.prev_y = y;
		return (0);
	}
	ft_render(fdf);
	return (0);
}

/* ft_mouse_release — stop dragging when the left button is released. */
int	ft_mouse_release(int btn, int x, int y, t_fdf *fdf)
{
	(void)x;
	(void)y;
	if (btn == BTN_LEFT)
		fdf->mouse.drag = 0;
	return (0);
}

/*
** ft_mouse_move — while dragging, map horizontal delta → Z rotation
** and vertical delta → X tilt, then re-render.
*/
int	ft_mouse_move(int x, int y, t_fdf *fdf)
{
	int	dx;
	int	dy;

	if (!fdf->mouse.drag)
		return (0);
	dx = x - fdf->mouse.prev_x;
	dy = y - fdf->mouse.prev_y;
	fdf->cam.z_rot += dx * 0.01;
	fdf->cam.x_rot += dy * 0.01;
	fdf->mouse.prev_x = x;
	fdf->mouse.prev_y = y;
	ft_render(fdf);
	return (0);
}
