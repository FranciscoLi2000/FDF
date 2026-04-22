#include "fdf_bonus.h"

static t_fdf	*ft_init_fdf(const char *file)
{
	t_fdf	*fdf;

	fdf = ft_calloc(1, sizeof(t_fdf));
	if (!fdf)
		ft_error("malloc failed");
	fdf->map = ft_parse_map(file);
	fdf->mlx = mlx_init();
	if (!fdf->mlx)
		ft_error("mlx_init failed");
	fdf->win = mlx_new_window(fdf->mlx, WIN_W, WIN_H, WIN_TITLE);
	if (!fdf->win)
		ft_error("mlx_new_window failed");
	fdf->img.ptr = mlx_new_image(fdf->mlx, WIN_W, WIN_H);
	if (!fdf->img.ptr)
		ft_error("mlx_new_image failed");
	fdf->img.addr = mlx_get_data_addr(fdf->img.ptr,
			&fdf->img.bpp, &fdf->img.ll, &fdf->img.endian);
	ft_init_cam(fdf);
	return (fdf);
}

int	main(int argc, char **argv)
{
	t_fdf	*fdf;

	if (argc != 2)
		ft_error("Usage: ./fdf_bonus <map.fdf>");
	fdf = ft_init_fdf(argv[1]);
	mlx_hook(fdf->win, 2, 1L << 0, ft_keypress, fdf);
	mlx_hook(fdf->win, 17, 0, ft_close, fdf);
	mlx_hook(fdf->win, EVT_BTN_PRESS, 1L << 2, ft_mouse_press, fdf);
	mlx_hook(fdf->win, EVT_BTN_RELEASE, 1L << 3, ft_mouse_release, fdf);
	mlx_hook(fdf->win, EVT_MOUSE_MOVE, 1L << 6, ft_mouse_move, fdf);
	ft_render(fdf);
	mlx_loop(fdf->mlx);
	return (0);
}
