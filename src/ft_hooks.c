#include "fdf.h"

int	ft_keypress(int key, t_fdf *fdf)
{
	if (key == KEY_ESC)
		ft_close(fdf);
	else if (key == KEY_W)
		fdf->cam.y_off -= MOVE_STEP;
	else if (key == KEY_S)
		fdf->cam.y_off += MOVE_STEP;
	else if (key == KEY_A)
		fdf->cam.x_off -= MOVE_STEP;
	else if (key == KEY_D)
		fdf->cam.x_off += MOVE_STEP;
	else if (key == KEY_Q)
		fdf->cam.z_rot -= ROT_STEP;
	else if (key == KEY_E)
		fdf->cam.z_rot += ROT_STEP;
	else if (key == KEY_PLUS)
		fdf->cam.zoom *= ZOOM_STEP;
	else if (key == KEY_MINUS)
		fdf->cam.zoom /= ZOOM_STEP;
	else if (key == KEY_R)
		ft_init_cam(fdf);
	else if (key == KEY_I)
		fdf->cam.proj = ISO;
	else if (key == KEY_P)
		fdf->cam.proj = PARALLEL;
	ft_render(fdf);
	return (0);
}

int	ft_close(t_fdf *fdf)
{
	ft_free_fdf(fdf);
	exit(EXIT_SUCCESS);
}
