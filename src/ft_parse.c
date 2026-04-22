#include "fdf.h"

static int	ft_count_rows(const char *file)
{
	int		fd;
	int		rows;
	char	*line;

	fd = open(file, O_RDONLY);
	if (fd < 0)
		ft_error("Cannot open map file");
	rows = 0;
	line = get_next_line(fd);
	while (line)
	{
		if (line[0] != '\n')
			rows++;
		free(line);
		line = get_next_line(fd);
	}
	close(fd);
	return (rows);
}

static int	ft_count_cols(const char *line)
{
	char	**tokens;
	int		cols;
	int		i;

	tokens = ft_split(line, ' ');
	if (!tokens)
		return (0);
	cols = 0;
	while (tokens[cols])
		cols++;
	i = 0;
	while (tokens[i])
		free(tokens[i++]);
	free(tokens);
	return (cols);
}

int	ft_hex_to_int(const char *hex)
{
	int	result;

	result = 0;
	while (*hex)
	{
		result <<= 4;
		if (*hex >= '0' && *hex <= '9')
			result |= *hex - '0';
		else if (*hex >= 'a' && *hex <= 'f')
			result |= *hex - 'a' + 10;
		else if (*hex >= 'A' && *hex <= 'F')
			result |= *hex - 'A' + 10;
		else
			break ;
		hex++;
	}
	return (result);
}

static void	ft_parse_token(const char *token,
				int *z, int *color, char *has_c)
{
	int	i;

	*z = ft_atoi(token);
	*color = 0;
	*has_c = 0;
	i = 0;
	while (token[i] && token[i] != ',')
		i++;
	if (token[i] == ',')
	{
		i++;
		if (token[i] == '0' && (token[i + 1] == 'x' || token[i + 1] == 'X'))
			i += 2;
		*color = ft_hex_to_int(&token[i]);
		*has_c = 1;
	}
}

static void	ft_parse_row(t_map *map, const char *line, int row)
{
	char	**tokens;
	int		col;

	tokens = ft_split(line, ' ');
	if (!tokens)
		return ;
	col = 0;
	while (tokens[col] && col < map->cols)
	{
		ft_parse_token(tokens[col],
			&map->z[row][col],
			&map->color[row][col],
			&map->has_color[row][col]);
		if (map->z[row][col] > map->z_max)
			map->z_max = map->z[row][col];
		if (map->z[row][col] < map->z_min)
			map->z_min = map->z[row][col];
		free(tokens[col]);
		col++;
	}
	while (tokens[col])
		free(tokens[col++]);
	free(tokens);
}

static t_map	*ft_alloc_map(int rows, int cols)
{
	t_map	*map;
	int		i;

	map = ft_calloc(1, sizeof(t_map));
	if (!map)
		return (NULL);
	map->rows = rows;
	map->cols = cols;
	map->z = ft_calloc(rows, sizeof(int *));
	map->color = ft_calloc(rows, sizeof(int *));
	map->has_color = ft_calloc(rows, sizeof(char *));
	if (!map->z || !map->color || !map->has_color)
		return (ft_free_map(map), NULL);
	i = 0;
	while (i < rows)
	{
		map->z[i] = ft_calloc(cols, sizeof(int));
		map->color[i] = ft_calloc(cols, sizeof(int));
		map->has_color[i] = ft_calloc(cols, sizeof(char));
		if (!map->z[i] || !map->color[i] || !map->has_color[i])
			return (ft_free_map(map), NULL);
		i++;
	}
	return (map);
}

t_map	*ft_parse_map(const char *file)
{
	t_map	*map;
	int		fd;
	char	*line;
	int		row;
	int		cols;

	line = NULL;
	fd = open(file, O_RDONLY);
	if (fd < 0)
		ft_error("Cannot open map file");
	line = get_next_line(fd);
	if (line)
		cols = ft_count_cols(line);
	else
		ft_error("Map file is empty");
	free(line);
	close(fd);
	map = ft_alloc_map(ft_count_rows(file), cols);
	if (!map)
		ft_error("malloc failed");
	map->z_max = 0;
	map->z_min = 0;
	fd = open(file, O_RDONLY);
	if (fd < 0)
		ft_error("Cannot open map file");
	row = 0;
	line = get_next_line(fd);
	while (line)
	{
		if (line[0] != '\n' && row < map->rows)
			ft_parse_row(map, line, row++);
		free(line);
		line = get_next_line(fd);
	}
	close(fd);
	return (map);
}

void	ft_free_map(t_map *map)
{
	int	i;

	if (!map)
		return ;
	i = 0;
	while (map->z && i < map->rows)
	{
		free(map->z[i]);
		free(map->color[i]);
		free(map->has_color[i]);
		i++;
	}
	free(map->z);
	free(map->color);
	free(map->has_color);
	free(map);
}
