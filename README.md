# FdF — Fil de Fer · 3D 线框地图渲染器

> **42 Barcelona · Common Core Project**
> 本 README 同时作为项目的**实现教程（Tutorial）**，按照代码实际编写顺序，从强制部分（Mandatory）到 Bonus 逐步拆解每一个模块。

---

## 目录 Table of Contents

1. [项目概述 Overview](#1-项目概述-overview)
2. [文件结构 Project Structure](#2-文件结构-project-structure)
3. [编译与运行 Build & Run](#3-编译与运行-build--run)
4. [核心数学原理 Core Math](#4-核心数学原理-core-math)
5. [数据结构 Data Structures](#5-数据结构-data-structures)
6. [实现指南：强制部分 Mandatory](#6-实现指南强制部分-mandatory)
   - [Step 1 · libft — 依赖库](#step-1--libft--依赖库)
   - [Step 2 · fdf.h — 头文件与数据结构](#step-2--fdfh--头文件与数据结构)
   - [Step 3 · ft_parse.c — 地图解析器](#step-3--ft_parsec--地图解析器)
   - [Step 4 · ft_project.c — 投影与相机](#step-4--ft_projectc--投影与相机)
   - [Step 5 · ft_utils.c — 工具函数](#step-5--ft_utilsc--工具函数)
   - [Step 6 · ft_hooks.c — 键盘交互](#step-6--ft_hooksc--键盘交互)
   - [Step 7 · ft_render.c — 渲染循环 ⚠️ MLX + Bresenham](#step-7--ft_renderc--渲染循环--mlx--bresenham)
   - [Step 8 · ft_main.c — 程序入口与 MLX 初始化](#step-8--ft_mainc--程序入口与-mlx-初始化)
7. [实现指南：Bonus 部分](#7-实现指南bonus-部分)
8. [测试与调试 Testing](#8-测试与调试-testing)
9. [常见错误 Common Bugs](#9-常见错误-common-bugs)

---

## 1. 项目概述 Overview

FdF（*Fil de Fer*，法语"铁丝"）要求读取一个 `.fdf` 高度图文件，将其渲染为**等轴投影（Isometric Projection）的三维线框景观**，并通过键盘实时控制视角。

**程序管线（Pipeline）：**

```
.fdf 文件
    │
    ▼
地图解析 (Parser)
 读取行列，存储 Z 高度值与颜色
    │
    ▼
相机初始化 (Camera)
 计算初始缩放、偏移，居中地图
    │
    ▼
投影变换 (Projection)
 (x, y, z) ──等轴公式──▶ 屏幕 (px, py)
    │
    ▼
MLX 图像缓冲 (Image Buffer)
 使用 mlx_new_image 创建离屏画布
    │
    ▼
Bresenham 连线 (draw_line)
 在画布上逐像素绘制边（横向 + 纵向）
    │
    ▼
推送到窗口 (mlx_put_image_to_window)
    │
    ▼
事件循环 (mlx_loop)
 监听键盘 → 更新相机 → 重新渲染
```

---

## 2. 文件结构 Project Structure

```
FDF/
├── en.subject.pdf/          # 三份项目 PDF（FDF / So Long / fract'ol）
├── includes/
│   ├── fdf.h                # 所有结构体、宏定义、函数原型
│   └── fdf_bonus.h          # Bonus 专用头文件
├── src/                     # 强制部分源文件（ft_*.c）
│   ├── ft_main.c            # 程序入口 + MLX 初始化
│   ├── ft_parse.c           # .fdf 文件解析器
│   ├── ft_project.c         # 等轴/平行投影 + 相机初始化
│   ├── ft_render.c          # 渲染循环 + Bresenham 连线
│   ├── ft_hooks.c           # 键盘事件处理
│   └── ft_utils.c           # 工具函数（error、free、颜色插值、像素写入）
├── bonus/                   # Bonus 源文件（ft_*_bonus.c）
│   ├── ft_main_bonus.c
│   ├── ft_parse_bonus.c
│   ├── ft_render_bonus.c
│   └── ft_utils_bonus.c
├── libft/                   # libft 依赖库
├── minilibx_linux/          # MiniLibX（Linux）图形库
└── Makefile
```

---

## 3. 编译与运行 Build & Run

```bash
# 编译强制部分
make

# 编译 Bonus
make bonus

# 运行（需要一个 .fdf 地图文件）
./fdf maps/42.fdf

# 清理目标文件
make clean

# 清理所有（目标文件 + 可执行文件）
make fclean

# 重新编译
make re
```

**键位说明 Keybindings：**

| 按键 | 功能 |
|------|------|
| `ESC` | 退出程序 |
| `W / S` | 上 / 下平移 |
| `A / D` | 左 / 右平移 |
| `Q / E` | 逆时针 / 顺时针旋转（Z 轴） |
| `+ / -` | 放大 / 缩小 |
| `R` | 重置视角 |
| `I` | 切换到等轴投影 |
| `P` | 切换到平行俯视投影 |

---

## 4. 核心数学原理 Core Math

### 4.1 等轴投影 Isometric Projection

等轴投影将三维坐标 `(x, y, z)` 映射到二维屏幕坐标 `(px, py)`，使三个轴看起来等长且成 120° 夹角。

**公式：**

```
px = (x - y) × cos(30°) × zoom  +  x_offset
py = (x + y) × sin(30°) × zoom  -  z  +  y_offset
```

其中 `cos(30°) = √3/2 ≈ 0.866`，`sin(30°) = 0.5`。

**可视化：**

```
        Z（高度）
        │
        │   Y（地图列向前）
        │  /
        │ /
        └──────── X（地图行向右）

等轴视图下三轴均成 120°，Z 轴垂直向上（影响 py）
```

### 4.2 Z 轴旋转（可选增强）

在投影前先绕 Z 轴旋转 `θ` 角：

```
x' = x × cos(θ) - y × sin(θ)
y' = x × sin(θ) + y × cos(θ)
```

再将 `(x', y')` 代入等轴公式，即可实现地图任意角度旋转。

### 4.3 初始缩放计算

地图在等轴视图下的近似屏幕宽度 ≈ `(cols + rows) × zoom × cos(30°)`。
为使地图居中填满窗口 80%：

```c
zoom = (WIN_W < WIN_H ? WIN_W : WIN_H) * 0.8
       / ((map->cols + map->rows) * cos(M_PI / 6.0));
```

---

## 5. 数据结构 Data Structures

所有结构体集中在 `includes/fdf.h`，遵循 Norminette 命名规范（`t_` 前缀）。

### `t_point` — 屏幕投影点

```c
typedef struct s_point
{
    double  x;      // 投影后屏幕 X 坐标
    double  y;      // 投影后屏幕 Y 坐标
    double  z;      // 缩放后的 Z 值（用于颜色插值）
    int     color;  // 该点的像素颜色（0xRRGGBB）
}   t_point;
```

### `t_map` — 地图数据

```c
typedef struct s_map
{
    int     rows;       // 行数（Y 方向）
    int     cols;       // 列数（X 方向）
    int     **z;        // 二维高度矩阵 z[row][col]
    int     **color;    // 二维颜色矩阵（显式颜色，无则为 0）
    char    **has_color;// 标记该格是否有显式颜色（1=有，0=无）
    int     z_max;      // 最大高度（用于颜色归一化）
    int     z_min;      // 最小高度
}   t_map;
```

> **为什么用 `int **` 而非 `int *`（一维）？**
> 两种方式均可。`int **` 在访问时更直观（`z[row][col]`），
> `int *`（一维，寻址公式 `z[row * cols + col]`）分配更简洁、缓存友好。
> 本项目选用 `int **` 提高可读性，通过一次性分配每行来保持内存局部性。

### `t_cam` — 相机参数

```c
typedef struct s_cam
{
    double  zoom;   // 缩放倍数
    double  x_off;  // 屏幕 X 偏移（用于居中 + 平移）
    double  y_off;  // 屏幕 Y 偏移
    double  z_rot;  // 绕 Z 轴旋转角度（弧度）
    int     proj;   // 投影模式：ISO(0) 或 PARALLEL(1)
}   t_cam;
```

### `t_img` — MLX 图像缓冲

```c
typedef struct s_img
{
    void    *ptr;     // mlx_new_image 返回的指针
    char    *addr;    // 像素数组首地址（mlx_get_data_addr）
    int     bpp;      // 每像素位数（通常为 32）
    int     ll;       // 每行字节数（line_length）
    int     endian;   // 字节序
}   t_img;
```

### `t_fdf` — 总管家结构体

```c
typedef struct s_fdf
{
    void    *mlx;   // MLX 连接实例
    void    *win;   // 窗口指针
    t_img   img;    // 图像缓冲（离屏渲染）
    t_map   *map;   // 地图数据
    t_cam   cam;    // 相机参数
}   t_fdf;
```

> **Norminette 传参技巧：** 单个函数最多 4 个参数。
> 只传 `t_fdf *fdf`，所有子系统都从这一个指针访问，无需多参数。

---

## 6. 实现指南：强制部分 Mandatory

### Step 1 · libft — 依赖库

FdF 需要 libft 提供以下函数（放在 `libft/` 目录，由 `libft/Makefile` 编译为 `libft.a`）：

| 函数 | 用途 |
|------|------|
| `ft_split(s, c)` | 按分隔符切割字符串（解析地图行） |
| `ft_atoi(str)` | 字符串转整数（读取 Z 值） |
| `ft_strlen(s)` | 字符串长度 |
| `ft_strdup(s)` | 字符串复制 |
| `ft_strjoin(s1, s2)` | 字符串拼接（GNL 内部使用） |
| `ft_calloc(n, size)` | 分配并清零内存 |
| `ft_bzero(ptr, n)` | 清零内存块 |
| `ft_memset(ptr, c, n)` | 填充内存块 |
| `ft_putstr_fd(s, fd)` | 向 fd 输出字符串（错误信息） |
| `ft_putendl_fd(s, fd)` | 同上 + 换行 |
| `ft_itoa(n)` | 整数转字符串（调试用） |
| `get_next_line(fd)` | 逐行读取文件 |

**`libft/Makefile` 关键规则：**

```makefile
NAME   = libft.a
CC     = cc
CFLAGS = -Wall -Wextra -Werror
SRCS   = ft_memset.c ft_bzero.c ft_calloc.c ft_strlen.c ft_strdup.c \
         ft_strjoin.c ft_split.c ft_atoi.c ft_itoa.c    \
         ft_putstr_fd.c ft_putendl_fd.c get_next_line.c
OBJS   = $(SRCS:.c=.o)

all: $(NAME)
$(NAME): $(OBJS)
ar rcs $(NAME) $(OBJS)
%.o: %.c libft.h
$(CC) $(CFLAGS) -c $< -o $@
clean:
rm -f $(OBJS)
fclean: clean
rm -f $(NAME)
re: fclean all
.PHONY: all clean fclean re
```

---

### Step 2 · fdf.h — 头文件与数据结构

`includes/fdf.h` 是整个项目的"合同"——所有结构体定义、宏、函数原型都在这里集中声明。

```c
#ifndef FDF_H
# define FDF_H

# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <math.h>
# include "../minilibx_linux/mlx.h"
# include "../libft/libft.h"

/* ── 窗口参数 ── */
# define WIN_W     1280
# define WIN_H     960
# define WIN_TITLE "FdF"

/* ── Linux X11 键码 ── */
# define KEY_ESC   65307
# define KEY_W     119
# define KEY_A     97
# define KEY_S     115
# define KEY_D     100
# define KEY_Q     113
# define KEY_E     101
# define KEY_R     114
# define KEY_PLUS  61
# define KEY_MINUS 45
# define KEY_I     105
# define KEY_P     112

/* ── 投影模式 ── */
# define ISO      0
# define PARALLEL 1

/* ── 默认颜色（高度渐变：低=蓝，高=橙红） ── */
# define C_LOW  0x0000FF
# define C_HIGH 0xFF4500

/* ── 相机步进量 ── */
# define MOVE_STEP  20.0
# define ROT_STEP   0.05
# define ZOOM_STEP  1.1

typedef struct s_point
{
    double  x;
    double  y;
    double  z;
    int     color;
}   t_point;

typedef struct s_map
{
    int     rows;
    int     cols;
    int     **z;
    int     **color;
    char    **has_color;
    int     z_max;
    int     z_min;
}   t_map;

typedef struct s_cam
{
    double  zoom;
    double  x_off;
    double  y_off;
    double  z_rot;
    int     proj;
}   t_cam;

typedef struct s_img
{
    void    *ptr;
    char    *addr;
    int     bpp;
    int     ll;
    int     endian;
}   t_img;

typedef struct s_fdf
{
    void    *mlx;
    void    *win;
    t_img   img;
    t_map   *map;
    t_cam   cam;
}   t_fdf;

/* ── ft_parse.c ── */
t_map   *ft_parse_map(const char *file);
void    ft_free_map(t_map *map);

/* ── ft_project.c ── */
t_point ft_project(int col, int row, t_fdf *fdf);
void    ft_init_cam(t_fdf *fdf);

/* ── ft_render.c ── */
void    ft_render(t_fdf *fdf);
void    ft_draw_line(t_fdf *fdf, t_point p0, t_point p1);

/* ── ft_hooks.c ── */
int     ft_keypress(int key, t_fdf *fdf);
int     ft_close(t_fdf *fdf);

/* ── ft_utils.c ── */
void    ft_error(const char *msg);
void    ft_free_fdf(t_fdf *fdf);
int     ft_lerp_color(int c1, int c2, double t);
void    ft_pixel_put(t_img *img, int x, int y, int color);
int     ft_hex_to_int(const char *hex);

#endif
```

---

### Step 3 · ft_parse.c — 地图解析器

解析器采用**双路读取法（Two-Pass）**：
- **第一遍**：只数行数（`rows`）和列数（`cols`），同时校验每行宽度一致。
- **第二遍**：分配内存，填充 `z`、`color`、`has_color` 矩阵。

**.fdf 文件格式：**

```
0  0  0  0  0
0  1  1  1  0
0  1  2  1  0
0  1  1  1  0
0  0  0  0  0
```

带颜色的格式（`z值,0xRRGGBB`）：

```
0,0x000000  0,0x0000FF  0
1,0xFF0000  2,0x00FF00  1
```

**解析流程：**

```
open(file)
    │
    ├─ 第一遍 ─▶  逐行 get_next_line
    │              ├─ 数行数 rows++
    │              └─ 第一行 ft_split → 数 cols（后续行校验一致性）
    │            close(fd)
    │
    ├─ 分配内存 ─▶  ft_alloc_map(rows, cols)
    │               为 z[][]、color[][]、has_color[][] 各分配二维数组
    │
    └─ 第二遍 ─▶  再次 open(file)
                   逐行 get_next_line → ft_split → ft_parse_token
                   close(fd)
```

**`ft_parse_token` 解析单个格子：**

```c
// token 格式：  "5"  或  "5,0xFF0000"
//              └─z─┘    └──z──┘└色┘
static void ft_parse_token(const char *token,
                            int *z, int *color, char *has_c)
{
    int i;

    *z     = ft_atoi(token);   // ft_atoi 遇到 ',' 自动停止
    *color = 0;
    *has_c = 0;
    i = 0;
    while (token[i] && token[i] != ',')
        i++;
    if (token[i] == ',')       // 有显式颜色
    {
        i++;
        if (token[i] == '0' && (token[i + 1] == 'x' || token[i + 1] == 'X'))
            i += 2;
        *color = ft_hex_to_int(&token[i]);
        *has_c = 1;
    }
}
```

**内存分配（`ft_alloc_map`）：**

```c
// 分配 rows × cols 的二维数组（每层单独 malloc 以便逐行 free）
static t_map *ft_alloc_map(int rows, int cols)
{
    t_map *map;
    int    i;

    map = ft_calloc(1, sizeof(t_map));
    map->rows = rows;
    map->cols = cols;
    map->z         = ft_calloc(rows, sizeof(int *));
    map->color     = ft_calloc(rows, sizeof(int *));
    map->has_color = ft_calloc(rows, sizeof(char *));
    i = 0;
    while (i < rows)
    {
        map->z[i]         = ft_calloc(cols, sizeof(int));
        map->color[i]     = ft_calloc(cols, sizeof(int));
        map->has_color[i] = ft_calloc(cols, sizeof(char));
        i++;
    }
    return (map);
}
```

**内存释放（`ft_free_map`）：**

```c
void ft_free_map(t_map *map)
{
    int i;

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
```

---

### Step 4 · ft_project.c — 投影与相机

#### `ft_init_cam` — 相机初始化

```c
void ft_init_cam(t_fdf *fdf)
{
    double span;

    // 地图在等轴视图下的近似对角宽度
    span = (fdf->map->cols + fdf->map->rows) * cos(M_PI / 6.0);
    fdf->cam.zoom  = (WIN_W < WIN_H ? WIN_W : WIN_H) * 0.75 / span;
    if (fdf->cam.zoom < 1.0)
        fdf->cam.zoom = 1.0;
    fdf->cam.x_off = WIN_W / 2.0;
    fdf->cam.y_off = WIN_H / 2.0;
    fdf->cam.z_rot = 0.0;
    fdf->cam.proj  = ISO;
}
```

#### `ft_project` — 坐标变换核心

```c
// 将地图格子 (col, row) 变换为屏幕坐标 t_point
t_point ft_project(int col, int row, t_fdf *fdf)
{
    t_point p;
    double  x, y, z;
    double  cos_r, sin_r, xr, yr;

    // 1. 将地图原点居中
    x = col - fdf->map->cols / 2.0;
    y = row - fdf->map->rows / 2.0;

    // 2. Z 轴旋转
    cos_r = cos(fdf->cam.z_rot);
    sin_r = sin(fdf->cam.z_rot);
    xr = x * cos_r - y * sin_r;
    yr = x * sin_r + y * cos_r;

    // 3. Z 高度缩放（z_scale 控制山峰夸张程度）
    z = ft_z_scale(fdf, row, col);

    // 4. 投影
    if (fdf->cam.proj == ISO)
    {
        p.x = (xr - yr) * cos(M_PI / 6.0) * fdf->cam.zoom + fdf->cam.x_off;
        p.y = (xr + yr) * sin(M_PI / 6.0) * fdf->cam.zoom - z + fdf->cam.y_off;
    }
    else // PARALLEL（平行俯视）
    {
        p.x = xr * fdf->cam.zoom + fdf->cam.x_off;
        p.y = yr * fdf->cam.zoom + fdf->cam.y_off;
    }

    // 5. 取得颜色（显式颜色或高度渐变色）
    p.color = ft_get_color(col, row, fdf);
    p.z = z;
    return (p);
}
```

**Z 轴缩放辅助：**

```c
// 自适应 z_scale：确保高度感知在不同地图上一致
static double ft_z_scale(t_fdf *fdf, int row, int col)
{
    int    z_range;
    double scale;

    z_range = fdf->map->z_max - fdf->map->z_min;
    if (z_range == 0)
        return (0.0);
    scale = fdf->cam.zoom
            * (fdf->map->cols > fdf->map->rows
                ? fdf->map->cols : fdf->map->rows)
            * 0.4 / z_range;
    return ((double)(fdf->map->z[row][col] - fdf->map->z_min) * scale);
}
```

---

### Step 5 · ft_utils.c — 工具函数

#### `ft_error` — 优雅报错退出

```c
void ft_error(const char *msg)
{
    ft_putstr_fd("Error: ", 2);
    ft_putendl_fd((char *)msg, 2);
    exit(EXIT_FAILURE);
}
```

#### `ft_free_fdf` — 释放所有资源

```c
void ft_free_fdf(t_fdf *fdf)
{
    if (!fdf)
        return ;
    if (fdf->img.ptr)
        mlx_destroy_image(fdf->mlx, fdf->img.ptr);
    if (fdf->win)
        mlx_destroy_window(fdf->mlx, fdf->win);
    if (fdf->mlx)
    {
        mlx_destroy_display(fdf->mlx);
        free(fdf->mlx);
    }
    ft_free_map(fdf->map);
    free(fdf);
}
```

#### `ft_lerp_color` — 颜色线性插值

```c
// 将颜色 c1 和 c2 按比例 t（0.0~1.0）混合
// 用于：高度渐变色（低=C_LOW, 高=C_HIGH）
//       以及 Bresenham 连线时两端点颜色过渡
int ft_lerp_color(int c1, int c2, double t)
{
    int r;
    int g;
    int b;

    r = (int)((1 - t) * ((c1 >> 16) & 0xFF) + t * ((c2 >> 16) & 0xFF));
    g = (int)((1 - t) * ((c1 >> 8)  & 0xFF) + t * ((c2 >> 8)  & 0xFF));
    b = (int)((1 - t) * (c1 & 0xFF)         + t * (c2 & 0xFF));
    return ((r << 16) | (g << 8) | b);
}
```

#### `ft_hex_to_int` — 十六进制字符串转整数

```c
// 将 "FF0000" 或 "ff0000" 转换为 0xFF0000
int ft_hex_to_int(const char *hex)
{
    int result;

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
```

#### `ft_pixel_put` — 向 MLX 图像缓冲写入像素

```c
// 直接操作 mlx_get_data_addr 返回的内存地址，写入单个像素颜色
// 比 mlx_pixel_put 快得多（避免了每次系统调用的开销）
void ft_pixel_put(t_img *img, int x, int y, int color)
{
    char *dst;

    if (x < 0 || x >= WIN_W || y < 0 || y >= WIN_H)
        return ;
    dst = img->addr + (y * img->ll + x * (img->bpp / 8));
    *(unsigned int *)dst = color;
}
```

> **为什么不用 `mlx_pixel_put`？**
> `mlx_pixel_put` 每次调用都触发 X11 系统调用，渲染整张地图会产生数万次调用，导致严重闪屏和卡顿。
> 使用 `mlx_new_image` + `mlx_get_data_addr` 先在内存缓冲区绘制完整帧，
> 最后一次性 `mlx_put_image_to_window`，即**双缓冲（Double Buffering）**技术。

---

### Step 6 · ft_hooks.c — 键盘交互

键盘处理函数在按键后更新相机参数，再调用 `ft_render` 重绘。

```c
int ft_keypress(int key, t_fdf *fdf)
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

int ft_close(t_fdf *fdf)
{
    ft_free_fdf(fdf);
    exit(EXIT_SUCCESS);
}
```

**MLX 事件钩子注册（在 `ft_main.c` 中）：**

```c
// X11 事件号：2 = KeyPress，17 = DestroyNotify（点击窗口关闭按钮）
mlx_hook(fdf->win, 2,  1L << 0, ft_keypress, fdf);
mlx_hook(fdf->win, 17, 0,       ft_close,    fdf);
```

---

### Step 7 · ft_render.c — 渲染循环 ⚠️ MLX + Bresenham

> **⚠️ 此步骤需要在下一阶段实现 `ft_draw_line`（Bresenham 算法）和 MLX 图像缓冲写像素。**
> **实现前请确认，届时会提供完整的算法讲解。**

**渲染循环结构（`ft_render`）：**

```c
void ft_render(t_fdf *fdf)
{
    int     row;
    int     col;
    t_point curr;
    t_point next;

    // 1. 清空图像缓冲（用黑色填充）
    ft_memset(fdf->img.addr, 0,
        WIN_W * WIN_H * (fdf->img.bpp / 8));

    // 2. 遍历每个格子，连横线（→）和竖线（↓）
    row = 0;
    while (row < fdf->map->rows)
    {
        col = 0;
        while (col < fdf->map->cols)
        {
            curr = ft_project(col, row, fdf);
            if (col + 1 < fdf->map->cols)   // 连右邻
            {
                next = ft_project(col + 1, row, fdf);
                ft_draw_line(fdf, curr, next);  // ← Bresenham
            }
            if (row + 1 < fdf->map->rows)   // 连下邻
            {
                next = ft_project(col, row + 1, fdf);
                ft_draw_line(fdf, curr, next);  // ← Bresenham
            }
            col++;
        }
        row++;
    }

    // 3. 一次性将缓冲推送到窗口（双缓冲）
    mlx_put_image_to_window(fdf->mlx, fdf->win, fdf->img.ptr, 0, 0);
}
```

**Bresenham 算法概要（待实现）：**

```
给定屏幕上两点 P0(x0,y0) 和 P1(x1,y1)，
沿两点之间的直线逐像素填充颜色（包含颜色插值）。

核心思想：
  dx = |x1 - x0|,  dy = |y1 - y0|
  每步沿主方向（步进量更大的轴）走 1 个像素，
  用"决策参数"判断次方向是否也需要步进。
  时间复杂度 O(max(dx,dy))，只用整数加减，无浮点。
```

---

### Step 8 · ft_main.c — 程序入口与 MLX 初始化

```c
#include "fdf.h"

static t_fdf *ft_init_fdf(const char *file)
{
    t_fdf *fdf;

    fdf = ft_calloc(1, sizeof(t_fdf));
    if (!fdf)
        ft_error("malloc failed");
    fdf->map = ft_parse_map(file);              // 解析地图
    fdf->mlx = mlx_init();                      // 建立 X-Server 连接
    if (!fdf->mlx)
        ft_error("mlx_init failed");
    fdf->win = mlx_new_window(fdf->mlx, WIN_W, WIN_H, WIN_TITLE);
    if (!fdf->win)
        ft_error("mlx_new_window failed");
    fdf->img.ptr  = mlx_new_image(fdf->mlx, WIN_W, WIN_H);
    if (!fdf->img.ptr)
        ft_error("mlx_new_image failed");
    // 获取图像缓冲内存地址（双缓冲的关键）
    fdf->img.addr = mlx_get_data_addr(fdf->img.ptr,
                        &fdf->img.bpp, &fdf->img.ll, &fdf->img.endian);
    ft_init_cam(fdf);                           // 初始化相机
    return (fdf);
}

int main(int argc, char **argv)
{
    t_fdf *fdf;

    if (argc != 2)
        ft_error("Usage: ./fdf <map.fdf>");
    fdf = ft_init_fdf(argv[1]);
    mlx_hook(fdf->win, 2,  1L << 0, ft_keypress, fdf);
    mlx_hook(fdf->win, 17, 0,       ft_close,    fdf);
    ft_render(fdf);
    mlx_loop(fdf->mlx);
    return (0);
}
```

**MLX 初始化流程图：**

```
mlx_init()
    └─▶ 返回 mlx 实例指针（建立与 X-Server 的连接）

mlx_new_window(mlx, W, H, title)
    └─▶ 返回窗口指针，在屏幕上创建一个 W×H 的窗口

mlx_new_image(mlx, W, H)
    └─▶ 在内存中分配一块 W×H 的像素缓冲区（不显示）

mlx_get_data_addr(img, &bpp, &ll, &endian)
    └─▶ 返回缓冲区首地址 addr
        bpp = 每像素位数（通常 32），用于计算字节偏移
        ll  = 每行字节数（line_length），用于计算行偏移
        像素地址 = addr + (y * ll + x * bpp/8)

ft_render(fdf)         ← 首次绘制
mlx_loop(mlx)          ← 进入事件循环，阻塞等待键盘/窗口事件
```

---

## 7. 实现指南：Bonus 部分

> Bonus 部分的源文件放在 `bonus/`，头文件为 `includes/fdf_bonus.h`，编译目标为 `fdf_bonus`。

### Bonus 功能列表

| 功能 | 说明 |
|------|------|
| **额外投影** | 新增锥形投影（Conic）或斜二测画法（Cavalier） |
| **鼠标缩放** | 滚轮缩放，鼠标拖拽平移 |
| **额外旋转轴** | 增加 X 轴和 Y 轴旋转（原版只有 Z 轴） |
| **颜色渐变** | 高度颜色更丰富（低=蓝→中=绿→高=白） |
| **信息 HUD** | 在窗口角落显示当前 zoom / 旋转角度等参数 |

### Bonus 架构调整

```
bonus/
├── ft_main_bonus.c     # main + MLX init（复用 mandatory 逻辑）
├── ft_render_bonus.c   # 额外投影模式 + 鼠标事件渲染
├── ft_parse_bonus.c    # 同 mandatory（或直接符号链接）
└── ft_utils_bonus.c    # HUD 绘制、鼠标回调函数
```

**鼠标事件钩子（X11 事件号）：**

```c
// 4 = ButtonPress（包含滚轮），6 = MotionNotify（鼠标移动）
mlx_hook(fdf->win, 4, 1L << 2, ft_mouse_press,  fdf);
mlx_hook(fdf->win, 6, 1L << 6, ft_mouse_motion, fdf);
```

**滚轮缩放（鼠标按键编号）：**

```c
int ft_mouse_press(int button, int x, int y, t_fdf *fdf)
{
    (void)x;
    (void)y;
    if (button == 4)         // 滚轮向上 → 放大
        fdf->cam.zoom *= ZOOM_STEP;
    else if (button == 5)    // 滚轮向下 → 缩小
        fdf->cam.zoom /= ZOOM_STEP;
    ft_render(fdf);
    return (0);
}
```

---

## 8. 测试与调试 Testing

### 测试地图

```bash
# 42 校徽（大型地图，测试性能）
./fdf test_maps/42.fdf

# 火星地形（高度变化剧烈，测试 z_scale）
./fdf test_maps/mars.fdf

# 金字塔（测试对角线渲染）
./fdf test_maps/pyramide.fdf

# 平面（所有 Z = 0，测试基础网格）
./fdf test_maps/flat.fdf
```

### 内存检查

```bash
# Linux（推荐）
valgrind --leak-check=full --show-leak-kinds=all ./fdf maps/42.fdf

# 检查文件描述符泄漏
valgrind --track-fds=yes ./fdf maps/42.fdf
```

### 边界测试清单

- [ ] 文件不存在 → 程序应输出 `Error: Cannot open map file` 后退出
- [ ] 空文件 → 程序应报错，不 Segfault
- [ ] 每行列数不一致 → 程序应报错退出
- [ ] 所有 Z = 0（平坦地图）→ 正常渲染网格，无颜色渐变
- [ ] Z 值为负数 → 正常渲染，低于基准面
- [ ] 点击窗口 × 按钮 → 程序正常退出，无内存泄漏
- [ ] 快速连续按键 → 不崩溃

---

## 9. 常见错误 Common Bugs

| 错误现象 | 原因 | 解决方案 |
|----------|------|----------|
| 渲染结果在窗口角落 | `x_off` / `y_off` 初始化为 0 | `ft_init_cam` 中将偏移设为 `WIN_W/2`、`WIN_H/2` |
| 山峰过于夸张像"刺猬" | `z_scale` 没有按地图大小归一化 | 在 `ft_z_scale` 中除以 `z_range`，乘以地图尺寸 |
| 只有横线没有竖线 | 渲染循环只连了右邻，漏掉下邻 | 在内层循环中同时连 `(col+1, row)` 和 `(col, row+1)` |
| 最后一行/列没有渲染 | `< rows` 写成 `<= rows`（越界） | 检查循环终止条件，使用 `< map->rows` |
| 画面闪烁 | 直接用 `mlx_pixel_put` 渲染 | 改用 `mlx_new_image` + `addr` 内存写入 + `mlx_put_image_to_window` |
| GNL 内存泄漏 | 每次 `get_next_line` 后未 `free(line)` | 保证每条 GNL 返回的行都被 `free` 掉 |
| `ft_split` 后内存泄漏 | 只释放了 `tokens` 指针，未释放各元素 | 先遍历 `free(tokens[i])`，再 `free(tokens)` |
| `z_min` 初始化为 0 | 对全负值地图 `z_min` 永远不更新 | 初始化为 `map->z[0][0]`，或用 `INT_MAX`/`INT_MIN` |
| 旋转后图像消失 | 旋转后坐标超出屏幕范围，`ft_pixel_put` 越界保护未生效 | 确认 `ft_pixel_put` 中的边界检查正确 |

---

> **下一步 Next Step**
>
> 所有基础架构和工具函数已就位。
> 现在需要实现最核心的两个部分：
>
> 1. **`ft_pixel_put`** — MLX 图像缓冲直接内存写入（了解 `bpp` / `ll` / `endian`）
> 2. **`ft_draw_line`** — Bresenham 线段算法（整数运算，支持颜色插值）
>
> 请确认后，我们将进入这两个函数的完整实现！

