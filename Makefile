NAME		= fdf
BONUS_NAME	= fdf_bonus

CC		= cc
CFLAGS	= -Wall -Wextra -Werror

INCLUDES	= -I includes -I minilibx_linux -I libft
LIBS		= -L minilibx_linux -lmlx_Linux -L libft -lft -lXext -lX11 -lm -lz

SRC_DIR		= src
BONUS_DIR	= bonus
OBJ_DIR		= obj

SRCS		= $(wildcard $(SRC_DIR)/ft_*.c)
BONUS_SRCS	= $(wildcard $(BONUS_DIR)/ft_*_bonus.c)

OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BONUS_OBJS	= $(BONUS_SRCS:$(BONUS_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	make -C libft
	make -C minilibx_linux
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)

bonus: $(BONUS_OBJS)
	make -C libft
	make -C minilibx_linux
	$(CC) $(CFLAGS) $(BONUS_OBJS) $(LIBS) -o $(BONUS_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(BONUS_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	make -C libft clean
	make -C minilibx_linux clean

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)
	make -C libft fclean

re: fclean all

.PHONY: all bonus clean fclean re
