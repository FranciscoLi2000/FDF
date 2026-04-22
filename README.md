# FdF — Fil de Fer 線框地形渲染器

> **繁體中文實作指南**  
> 本文件以「從零開始寫出這個專案」為目標撰寫。  
> 閱讀完本文並對照原始碼，你應該能夠在沒有任何外部協助的情況下，獨立完整重現這個專案。

---

## 目錄

1. [專案概覽](#1-專案概覽)
2. [環境依賴與編譯](#2-環境依賴與編譯)
3. [專案目錄結構](#3-專案目錄結構)
4. [資料結構設計](#4-資料結構設計)
5. [強制部分：逐步實作](#5-強制部分逐步實作)
   - 5.1 頭文件 `fdf.h`
   - 5.2 地圖解析 `ft_parse.c`
   - 5.3 投影與相機 `ft_project.c`
   - 5.4 Bresenham 畫線與渲染 `ft_render.c`
   - 5.5 鍵盤事件 `ft_hooks.c`
   - 5.6 工具函式 `ft_utils.c`
   - 5.7 程式入口 `ft_main.c`
6. [Bonus 部分：逐步實作](#6-bonus-部分逐步實作)
   - 6.1 頭文件 `fdf_bonus.h`
   - 6.2 三軸旋轉 `ft_project_bonus.c`
   - 6.3 鍵盤＋滑鼠事件 `ft_hooks_bonus.c`
   - 6.4 渲染 + HUD `ft_render_bonus.c`
   - 6.5 工具函式（含配色） `ft_utils_bonus.c`
   - 6.6 程式入口 `ft_main_bonus.c`
7. [核心演算法詳解](#7-核心演算法詳解)
8. [常見錯誤與修正](#8-常見錯誤與修正)
9. [記憶體管理檢查清單](#9-記憶體管理檢查清單)
10. [操作快捷鍵總表](#10-操作快捷鍵總表)

---

## 1. 專案概覽

FdF（法語：Fil de Fer，「鐵絲」之意）是一個 **線框地形視覺化工具**。  
程式讀取一份純文字格式的高度地圖（`.fdf` 檔），  
把每個格點依照其高度（Z 值）轉換為三維座標，  
再用等角投影（Isometric Projection）投影到二維螢幕，  
最後以 Bresenham 畫線演算法連接相鄰格點，形成線框網格。

**最終視覺效果：**

```
          .*   *.
        .* . . . *.
      .* . . . . . *.
    .* . . . / \ . . *.
      .* . . \ / . *.
        .* . . . *.
          .* . *.
```

---

## 2. 環境依賴與編譯

### 系統需求

- Linux（推薦 Ubuntu 20.04+）
- 安裝 X11 開發函式庫：

```bash
sudo apt-get install libx11-dev libxext-dev
```

### 目錄依賴

本專案依賴以下兩個子目錄，請確保它們存在並可正常編譯：

| 目錄 | 用途 |
|------|------|
| `libft/` | 42 標準函式庫（`ft_calloc`、`ft_split`、`get_next_line` 等） |
| `minilibx_linux/` | MiniLibX Linux 圖形函式庫 |

### 編譯指令

```bash
# 編譯強制部分 → 產生 ./fdf
make

# 編譯 Bonus 部分 → 產生 ./fdf_bonus
make bonus

# 清除目標檔
make clean

# 清除目標檔 + 二進位檔
make fclean

# 完整重新編譯
make re
```

### 執行

```bash
./fdf maps/42.fdf
./fdf_bonus maps/mars.fdf
```

### Makefile 說明

```makefile
NAME      = fdf
BONUS_NAME = fdf_bonus

CC     = cc
CFLAGS = -Wall -Wextra -Werror

INCLUDES = -I includes -I minilibx_linux -I libft/includes
LIBS     = -L minilibx_linux -lmlx_Linux -L libft -lft -lXext -lX11 -lm -lz
```

**重點：** `-lm` 是 `<math.h>` 所需的數學函式庫，務必加上。  
`-lXext -lX11` 是 MiniLibX 所需的 X11 函式庫。

---

## 3. 專案目錄結構

```
FDF/
├── Makefile
├── includes/
│   ├── fdf.h              # 強制部分頭文件
│   └── fdf_bonus.h        # Bonus 頭文件
├── src/                   # 強制部分原始碼
│   ├── ft_main.c          # main() + MLX 初始化
│   ├── ft_parse.c         # .fdf 地圖解析
│   ├── ft_project.c       # 座標投影 + 相機初始化
│   ├── ft_render.c        # 渲染迴圈 + Bresenham 畫線
│   ├── ft_hooks.c         # 鍵盤事件
│   └── ft_utils.c         # 工具函式
├── bonus/                 # Bonus 部分原始碼
│   ├── ft_main_bonus.c    # main() + 5 個 MLX 事件鉤子
│   ├── ft_parse_bonus.c   # 與強制部分相同
│   ├── ft_project_bonus.c # 三軸旋轉 + 三種投影模式
│   ├── ft_hooks_bonus.c   # 鍵盤 + 滑鼠拖拽 + 滾輪縮放
│   ├── ft_render_bonus.c  # Bresenham + HUD 疊加層
│   └── ft_utils_bonus.c   # 工具 + ft_scheme_color 三配色
├── libft/
└── minilibx_linux/
```

---

## 4. 資料結構設計

在開始撰寫任何函式之前，先把所有資料結構設計清楚，放在頭文件裡。  
這是整個專案的骨架，理解它等於理解了整個程式。

### 強制部分（`includes/fdf.h`）

#### `t_map` — 地圖資料

```c
typedef struct s_map
{
    int     rows;       // 總行數（.fdf 檔案有幾行）
    int     cols;       // 總列數（每行有幾個格點）
    int     **z;        // z[row][col] — 每個格點的高度值
    int     **color;    // color[row][col] — 格點顏色（若檔案有指定）
    char    **has_color;// has_color[row][col] — 1 = 檔案有指定顏色
    int     z_max;      // 地圖中最大高度
    int     z_min;      // 地圖中最小高度
}   t_map;
```

**為什麼 `z`、`color`、`has_color` 都是二維指標陣列？**  
因為我們不知道地圖大小，必須動態配置。  
做法是先配置一個「指標陣列」（rows 個 `int*`），  
再對每一行配置「整數陣列」（cols 個 `int`）。

```
z[0] → [0][1][2][3]...
z[1] → [0][1][2][3]...
z[2] → [0][1][2][3]...
```

#### `t_cam` — 相機狀態

```c
typedef struct s_cam
{
    double  zoom;   // 縮放倍率（像素 / 格點單位）
    double  x_off;  // 畫面水平偏移（平移）
    double  y_off;  // 畫面垂直偏移（平移）
    double  z_rot;  // Z 軸旋轉角（弧度）
    int     proj;   // 投影模式：ISO 或 PARALLEL
}   t_cam;
```

#### `t_img` — MiniLibX 影像緩衝區

```c
typedef struct s_img
{
    void    *ptr;    // mlx_new_image() 回傳的指標
    char    *addr;   // mlx_get_data_addr() 回傳的像素記憶體位址
    int     bpp;     // bits per pixel（通常是 32）
    int     ll;      // line length（每行的位元組數）
    int     endian;  // 位元組序（0 = little-endian）
}   t_img;
```

**關鍵理解：**  
`addr` 是整個畫面的像素記憶體起點。  
第 (x, y) 個像素的位置 = `addr + y * ll + x * (bpp / 8)`。  
直接寫入這個位址就能畫像素，比 `mlx_pixel_put()` 快得多。

#### `t_point` — 投影後的螢幕座標點

```c
typedef struct s_point
{
    double  x;     // 螢幕 X 座標
    double  y;     // 螢幕 Y 座標
    double  z;     // 保留深度值（畫線時用不到，但方便擴充）
    int     color; // 該點的顏色
}   t_point;
```

#### `t_line` — Bresenham 畫線狀態

```c
typedef struct s_line
{
    int     x;     // 目前像素 X
    int     y;     // 目前像素 Y
    int     dx;    // |x1 - x0|（X 方向距離）
    int     dy;    // |y1 - y0|（Y 方向距離）
    int     sx;    // X 步進方向（+1 或 -1）
    int     sy;    // Y 步進方向（+1 或 -1）
    int     err;   // 誤差累積器
    int     steps; // 總步數（max(dx, dy)）
    int     c0;    // 起點顏色
    int     c1;    // 終點顏色
}   t_line;
```

#### `t_fdf` — 整個程式的主結構

```c
typedef struct s_fdf
{
    void    *mlx;  // mlx_init() 的連線指標
    void    *win;  // 視窗指標
    t_img   img;   // 影像緩衝區
    t_map   *map;  // 地圖資料（heap 配置）
    t_cam   cam;   // 相機狀態（直接嵌入，非指標）
}   t_fdf;
```

### Bonus 額外擴充

Bonus 的 `t_cam` 多出三個欄位：

```c
double  x_rot;        // X 軸傾斜角（J/K 鍵）
double  y_rot;        // Y 軸偏轉角（U/O 鍵）
int     color_scheme; // 配色方案（0=FIRE, 1=GREY, 2=HEAT）
```

Bonus 新增 `t_mouse` 結構記錄滑鼠拖拽狀態：

```c
typedef struct s_mouse
{
    int     drag;    // 1 = 正在拖拽
    int     prev_x;  // 上一幀滑鼠 X
    int     prev_y;  // 上一幀滑鼠 Y
}   t_mouse;
```

---

## 5. 強制部分：逐步實作

> **實作順序建議：**  
> `fdf.h` → `ft_utils.c` → `ft_parse.c` → `ft_project.c` → `ft_render.c` → `ft_hooks.c` → `ft_main.c`

---

### 5.1 頭文件 `fdf.h`

把所有常數、結構體、函式宣告集中在一個頭文件，  
確保所有 `.c` 檔只需要 `#include "fdf.h"` 就能取得全部定義。

**重要常數：**

```c
/* 視窗大小 */
# define WIN_W   1280
# define WIN_H   960
# define WIN_TITLE "FdF"

/* Linux X11 鍵盤代碼（用 xev 指令可以查詢任意按鍵的代碼） */
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

/* 投影模式 */
# define ISO      0
# define PARALLEL 1

/* 高度漸層顏色（低海拔=藍，高海拔=橙紅） */
# define C_LOW    0x0000FF
# define C_HIGH   0xFF4500

/* 相機移動步幅 */
# define MOVE_STEP 20.0
# define ROT_STEP  0.05
# define ZOOM_STEP 1.1
```

**如何找按鍵代碼？**

```bash
# 在終端機執行 xev，然後按下你想要的鍵，就會顯示 keycode
xev | grep -A2 --line-buffered '^KeyPress'
```

---

### 5.2 地圖解析 `ft_parse.c`

地圖格式範例（`test.fdf`）：

```
0  0  0  0  0
0  1  2  1  0
0  2  4  2  0
0  1  2  1  0
0  0  0  0  0
```

帶自訂顏色的格式：

```
0,0xFF0000  1,0x00FF00  0
```

**實作步驟：**

#### 步驟一：計算行數（`ft_count_rows`）

```c
static int ft_count_rows(const char *file)
{
    int   fd;
    int   rows;
    char  *line;

    fd = open(file, O_RDONLY);
    if (fd < 0)
        ft_error("Cannot open map file");
    rows = 0;
    line = get_next_line(fd);
    while (line)
    {
        if (line[0] != '\n')  // 跳過空行
            rows++;
        free(line);           // 每次 get_next_line 回傳的字串都要 free
        line = get_next_line(fd);
    }
    close(fd);
    return (rows);
}
```

**注意：** `get_next_line` 每次呼叫都 `malloc` 一個新字串，  
你必須在每次迴圈結束時 `free(line)`，否則會有記憶體洩漏。

#### 步驟二：計算列數（`ft_count_cols`）

```c
static int ft_count_cols(const char *line)
{
    char  **tokens;
    int   cols;
    int   i;

    tokens = ft_split(line, ' ');  // 以空格分割
    if (!tokens)
        return (0);
    cols = 0;
    while (tokens[cols])           // 計算 token 數量
        cols++;
    i = 0;
    while (tokens[i])              // 釋放每個 token
        free(tokens[i++]);
    free(tokens);                  // 釋放指標陣列
    return (cols);
}
```

#### 步驟三：配置記憶體（`ft_alloc_map`）

```c
static t_map *ft_alloc_map(int rows, int cols)
{
    t_map *map;
    int   i;

    map = ft_calloc(1, sizeof(t_map));
    if (!map)
        return (NULL);
    map->rows = rows;
    map->cols = cols;
    // 先配置「指標陣列」（每個元素是 int*）
    map->z         = ft_calloc(rows, sizeof(int *));
    map->color     = ft_calloc(rows, sizeof(int *));
    map->has_color = ft_calloc(rows, sizeof(char *));
    if (!map->z || !map->color || !map->has_color)
        return (ft_free_map(map), NULL);  // 配置失敗立刻釋放
    i = 0;
    while (i < rows)
    {
        // 再為每一行配置「整數陣列」
        map->z[i]         = ft_calloc(cols, sizeof(int));
        map->color[i]     = ft_calloc(cols, sizeof(int));
        map->has_color[i] = ft_calloc(cols, sizeof(char));
        if (!map->z[i] || !map->color[i] || !map->has_color[i])
            return (ft_free_map(map), NULL);
        i++;
    }
    return (map);
}
```

**為什麼用 `ft_calloc` 而不是 `malloc`？**  
`ft_calloc` 會把配置的記憶體清零（`z_min` 初始為 0，`has_color` 初始為 0 等）。

#### 步驟四：解析單一 token（`ft_parse_token`）

每個 token 的格式是 `<高度值>` 或 `<高度值>,0x<顏色>`：

```c
static void ft_parse_token(const char *token,
                           int *z, int *color, char *has_c)
{
    int i;

    *z = ft_atoi(token);   // 解析整數高度（可以是負數）
    *color = 0;
    *has_c = 0;
    i = 0;
    while (token[i] && token[i] != ',')
        i++;
    if (token[i] == ',')   // 有顏色指定
    {
        i++;
        if (token[i] == '0' && (token[i + 1] == 'x' || token[i + 1] == 'X'))
            i += 2;        // 跳過 "0x" 前綴
        *color = ft_hex_to_int(&token[i]);
        *has_c = 1;
    }
}
```

#### 步驟五：十六進位字串轉整數（`ft_hex_to_int`）

```c
int ft_hex_to_int(const char *hex)
{
    int result;

    result = 0;
    while (*hex)
    {
        result <<= 4;   // 左移 4 位 = 乘以 16
        if (*hex >= '0' && *hex <= '9')
            result |= *hex - '0';
        else if (*hex >= 'a' && *hex <= 'f')
            result |= *hex - 'a' + 10;
        else if (*hex >= 'A' && *hex <= 'F')
            result |= *hex - 'A' + 10;
        else
            break;      // 遇到非十六進位字元即停止
        hex++;
    }
    return (result);
}
```

**範例：** `"FF4500"` 解析過程：  
`F` → result = 15  
`F` → result = 15*16 + 15 = 255  
`4` → result = 255*16 + 4 = 4084  
... 最終 = 16729344 = `0xFF4500`

#### 步驟六：主解析函式（`ft_parse_map`）

```c
t_map *ft_parse_map(const char *file)
{
    t_map *map;
    int   fd;
    char  *line;
    int   row;
    int   cols;

    // 第一次開檔：讀第一行確定列數
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

    // 配置記憶體
    map = ft_alloc_map(ft_count_rows(file), cols);
    if (!map)
        ft_error("malloc failed");
    map->z_max = 0;
    map->z_min = 0;

    // 第二次開檔：實際解析所有資料
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
```

**為什麼要開檔兩次？**  
第一次確定行列數以便配置記憶體，  
第二次才填入實際數值。  
這樣可以避免動態重新配置（realloc）的複雜性。

#### 步驟七：釋放地圖記憶體（`ft_free_map`）

```c
void ft_free_map(t_map *map)
{
    int i;

    if (!map)
        return;
    i = 0;
    // 先釋放每一行的整數陣列
    while (map->z && i < map->rows)
    {
        free(map->z[i]);
        free(map->color[i]);
        free(map->has_color[i]);
        i++;
    }
    // 再釋放指標陣列
    free(map->z);
    free(map->color);
    free(map->has_color);
    free(map);
}
```

**重要原則：** 配置和釋放必須成對，配置的層數有幾層，釋放也要幾層。

---

### 5.3 投影與相機 `ft_project.c`

這是整個專案最核心的數學部分。  
任務：把地圖的格點 `(col, row)` 轉換為螢幕上的像素座標 `(x, y)`。

#### 步驟一：相機初始化（`ft_init_cam`）

```c
void ft_init_cam(t_fdf *fdf)
{
    double span;

    // span = 地圖對角線在 ISO 投影後的大約寬度
    span = (fdf->map->cols + fdf->map->rows) * cos(M_PI / 6.0);
    if (span < 1.0)
        span = 1.0;
    // 讓地圖剛好佔視窗的 75%
    fdf->cam.zoom = (WIN_W < WIN_H ? WIN_W : WIN_H) * 0.75 / span;
    if (fdf->cam.zoom < 1.0)
        fdf->cam.zoom = 1.0;
    // 地圖置中
    fdf->cam.x_off = WIN_W / 2.0;
    fdf->cam.y_off = WIN_H / 2.0;
    fdf->cam.z_rot = 0.0;
    fdf->cam.proj = ISO;
}
```

**zoom 計算原理：**  
等角投影下，地圖寬度約為 `(cols + rows) * cos(30°)` 個「地圖單位」。  
用視窗較小邊的 75% 除以這個值，就能讓整張地圖剛好填滿視窗。

#### 步驟二：Z 值縮放（`ft_z_scale`）

```c
static double ft_z_scale(t_fdf *fdf, int row, int col)
{
    int    z_range;
    int    map_span;
    double scale;

    z_range = fdf->map->z_max - fdf->map->z_min;
    if (z_range == 0)
        return (0.0);                        // 平坦地圖，不需要高度
    map_span = fdf->map->cols > fdf->map->rows
        ? fdf->map->cols : fdf->map->rows;
    // 縮放比例：讓高度在視覺上佔地圖大小的 40%
    scale = fdf->cam.zoom * map_span * 0.4 / z_range;
    return ((double)(fdf->map->z[row][col] - fdf->map->z_min) * scale);
}
```

**為什麼要縮放 Z？**  
.fdf 檔的 Z 值可能是 0～255，也可能是 -100～100，單位不定。  
我們需要把它轉換成螢幕像素單位，讓視覺上的高度感覺合理。

#### 步驟三：等角投影（Isometric Projection）

等角投影是一種特殊的軸測投影，三條軸在投影後夾角相等（各 120°）。  
X 軸和 Y 軸在螢幕上各呈 30° 斜角，Z 軸垂直向上。

**數學公式（先做 Z 軸旋轉，再做等角投影）：**

```
1. 以地圖中心為原點：
   x = col - cols/2
   y = row - rows/2

2. Z 軸旋轉（繞螢幕法線旋轉）：
   xr = x * cos(z_rot) - y * sin(z_rot)
   yr = x * sin(z_rot) + y * cos(z_rot)

3. 等角投影（30° 斜軸）：
   screen_x = (xr - yr) * cos(30°) * zoom + x_off
   screen_y = (xr + yr) * sin(30°) * zoom - z + y_off
```

**為什麼螢幕 Y 要減去 Z？**  
螢幕 Y 軸向下為正，但地形高度越大應該越往上顯示，所以減去 Z。

```c
t_point ft_project(int col, int row, t_fdf *fdf)
{
    t_point p;
    double  x, y, z;
    double  cos_r, sin_r, xr, yr;

    // 以地圖中心為原點
    x = col - fdf->map->cols / 2.0;
    y = row - fdf->map->rows / 2.0;
    z = ft_z_scale(fdf, row, col);

    // Z 軸旋轉
    cos_r = cos(fdf->cam.z_rot);
    sin_r = sin(fdf->cam.z_rot);
    xr = x * cos_r - y * sin_r;
    yr = x * sin_r + y * cos_r;

    // 等角投影
    if (fdf->cam.proj == ISO)
    {
        p.x = (xr - yr) * cos(M_PI / 6.0) * fdf->cam.zoom + fdf->cam.x_off;
        p.y = (xr + yr) * sin(M_PI / 6.0) * fdf->cam.zoom - z + fdf->cam.y_off;
    }
    else  // PARALLEL：直接俯視
    {
        p.x = xr * fdf->cam.zoom + fdf->cam.x_off;
        p.y = yr * fdf->cam.zoom + fdf->cam.y_off;
    }
    p.z = z;
    p.color = ft_get_color(col, row, fdf);
    return (p);
}
```

#### 步驟四：顏色取得（`ft_get_color`）

```c
static int ft_get_color(int col, int row, t_fdf *fdf)
{
    double t;

    // 優先使用地圖檔自訂顏色
    if (fdf->map->has_color[row][col])
        return (fdf->map->color[row][col]);
    // 平坦地圖用白色
    if (fdf->map->z_max == fdf->map->z_min)
        return (0xFFFFFF);
    // 根據高度做線性插值（低=藍，高=橙紅）
    t = (double)(fdf->map->z[row][col] - fdf->map->z_min)
        / (double)(fdf->map->z_max - fdf->map->z_min);
    return (ft_lerp_color(C_LOW, C_HIGH, t));
}
```

**t 的意義：**  
`t = 0` 時為最低點（藍色），  
`t = 1` 時為最高點（橙紅色），  
`0 < t < 1` 時為線性插值的中間色。

---

### 5.4 Bresenham 畫線與渲染 `ft_render.c`

#### 核心概念：雙緩衝渲染

**不要使用 `mlx_pixel_put()`！** 它每畫一個像素都會呼叫一次系統呼叫，  
畫幾千個像素時會非常慢，螢幕會閃爍。

**正確做法：雙緩衝（off-screen buffer）**

```
1. 建立一個離屏影像（mlx_new_image）
2. 取得像素記憶體指標（mlx_get_data_addr）
3. 直接用指標寫入像素（ft_pixel_put）
4. 一次性把整個影像送到螢幕（mlx_put_image_to_window）
```

#### 步驟一：像素寫入（`ft_pixel_put`）

```c
void ft_pixel_put(t_img *img, int x, int y, int color)
{
    char *dst;

    // 邊界檢查：超出視窗範圍就不畫
    if (x < 0 || x >= WIN_W || y < 0 || y >= WIN_H)
        return;
    // 計算像素在記憶體中的位址
    // ll = line length（每行的位元組數，可能有對齊填充，不等於 WIN_W * 4）
    dst = img->addr + (y * img->ll + x * (img->bpp / 8));
    *(unsigned int *)dst = color;
}
```

**記憶體佈局（以 32 位元色彩為例）：**

```
addr[0..3]   → 第 0 行第 0 列像素（ARGB）
addr[4..7]   → 第 0 行第 1 列像素
...
addr[ll..ll+3] → 第 1 行第 0 列像素
```

注意 `ll` 不一定等於 `WIN_W * 4`，因為記憶體可能有對齊填充，  
**必須用 `mlx_get_data_addr` 回傳的 `ll` 值，不能自己算**。

#### 步驟二：顏色線性插值（`ft_lerp_color`）

畫線時，起點和終點可能是不同顏色（高度不同），  
需要在線段上做顏色漸變。

```c
int ft_lerp_color(int c1, int c2, double t)
{
    int r, g, b;

    // 把 32 位元顏色拆成 R、G、B 三個 8 位元分量
    // c1 = 0xRRGGBB，所以：
    //   R = (c1 >> 16) & 0xFF
    //   G = (c1 >> 8)  & 0xFF
    //   B =  c1        & 0xFF
    r = (int)((1 - t) * ((c1 >> 16) & 0xFF) + t * ((c2 >> 16) & 0xFF));
    g = (int)((1 - t) * ((c1 >> 8)  & 0xFF) + t * ((c2 >> 8)  & 0xFF));
    b = (int)((1 - t) * (c1 & 0xFF)         + t * (c2 & 0xFF));
    // 重新組合
    return ((r << 16) | (g << 8) | b);
}
```

#### 步驟三：Bresenham 畫線演算法（`ft_draw_line`）

Bresenham 演算法的核心思想：  
用整數加減法近似一條直線，避免使用浮點除法。

**誤差累積器原理：**

```
想畫從 (0,0) 到 (5,2) 的直線：
理論上 y = 2x/5，每步 x+1 時 y 增加 2/5

Bresenham 的做法：
  維護一個誤差值 err = dx - dy = 5 - 2 = 3
  每步：
    若 2*err > -dy → 在 X 方向步進，err -= dy
    若 2*err < dx  → 在 Y 方向步進，err += dx
  兩個條件在同一步可以都觸發（對角線步）
```

```c
// 初始化 Bresenham 狀態
static t_line ft_line_init(t_point p0, t_point p1)
{
    t_line l;

    l.x  = (int)round(p0.x);       // 起點（取整）
    l.y  = (int)round(p0.y);
    l.dx = abs((int)round(p1.x) - l.x);  // X 距離（絕對值）
    l.dy = abs((int)round(p1.y) - l.y);  // Y 距離（絕對值）
    l.sx = (p1.x > p0.x) ? 1 : -1;      // X 步進方向
    l.sy = (p1.y > p0.y) ? 1 : -1;      // Y 步進方向
    l.err = l.dx - l.dy;                 // 初始誤差
    l.steps = (l.dx > l.dy) ? l.dx : l.dy;  // 總步數
    l.c0 = p0.color;
    l.c1 = p1.color;
    return (l);
}

void ft_draw_line(t_fdf *fdf, t_point p0, t_point p1)
{
    t_line l;
    int    e2;
    int    i;
    double t;

    l = ft_line_init(p0, p1);
    i = 0;
    while (1)
    {
        // 計算插值比例並畫像素
        t = (l.steps > 0) ? (double)i / (double)l.steps : 0.0;
        ft_pixel_put(&fdf->img, l.x, l.y, ft_lerp_color(l.c0, l.c1, t));

        // 到達終點，停止
        if (l.x == (int)round(p1.x) && l.y == (int)round(p1.y))
            break;

        e2 = 2 * l.err;
        if (e2 > -l.dy)     // X 方向步進
        {
            l.err -= l.dy;
            l.x += l.sx;
        }
        if (e2 < l.dx)      // Y 方向步進
        {
            l.err += l.dx;
            l.y += l.sy;
        }
        i++;
    }
}
```

#### 步驟四：渲染整張地圖（`ft_render`）

```c
void ft_render(t_fdf *fdf)
{
    int     row, col;
    t_point curr, next;

    // 清空影像緩衝（全部填黑）
    ft_bzero(fdf->img.addr, WIN_W * WIN_H * (fdf->img.bpp / 8));

    // 遍歷所有格點
    row = 0;
    while (row < fdf->map->rows)
    {
        col = 0;
        while (col < fdf->map->cols)
        {
            curr = ft_project(col, row, fdf);

            // 連接右鄰格點（水平邊）
            if (col + 1 < fdf->map->cols)
            {
                next = ft_project(col + 1, row, fdf);
                ft_draw_line(fdf, curr, next);
            }
            // 連接下鄰格點（垂直邊）
            if (row + 1 < fdf->map->rows)
            {
                next = ft_project(col, row + 1, fdf);
                ft_draw_line(fdf, curr, next);
            }
            col++;
        }
        row++;
    }

    // 把離屏緩衝推送到螢幕
    mlx_put_image_to_window(fdf->mlx, fdf->win, fdf->img.ptr, 0, 0);
}
```

**為什麼只連右鄰和下鄰，而不連四個方向？**  
如果連接所有方向，每條邊會被畫兩次（浪費效能）。  
只連右（→）和下（↓），每條邊剛好被畫一次。

---

### 5.5 鍵盤事件 `ft_hooks.c`

#### MiniLibX 事件系統

MLX 的事件是透過 `mlx_hook` 函式綁定的：

```c
mlx_hook(win, event_id, event_mask, handler_function, param);
```

| 事件 ID | 含義 | 掩碼 |
|---------|------|------|
| 2 | KeyPress（按下鍵盤） | `1L << 0` |
| 17 | DestroyNotify（關閉視窗按鈕） | `0` |

```c
int ft_keypress(int key, t_fdf *fdf)
{
    if (key == KEY_ESC)
        ft_close(fdf);            // 關閉程式
    else if (key == KEY_W)
        fdf->cam.y_off -= MOVE_STEP;  // 向上移動
    else if (key == KEY_S)
        fdf->cam.y_off += MOVE_STEP;  // 向下移動
    else if (key == KEY_A)
        fdf->cam.x_off -= MOVE_STEP;  // 向左移動
    else if (key == KEY_D)
        fdf->cam.x_off += MOVE_STEP;  // 向右移動
    else if (key == KEY_Q)
        fdf->cam.z_rot -= ROT_STEP;   // 逆時針旋轉
    else if (key == KEY_E)
        fdf->cam.z_rot += ROT_STEP;   // 順時針旋轉
    else if (key == KEY_PLUS)
        fdf->cam.zoom *= ZOOM_STEP;   // 放大
    else if (key == KEY_MINUS)
        fdf->cam.zoom /= ZOOM_STEP;   // 縮小
    else if (key == KEY_R)
        ft_init_cam(fdf);             // 重置相機
    else if (key == KEY_I)
        fdf->cam.proj = ISO;          // 切換等角投影
    else if (key == KEY_P)
        fdf->cam.proj = PARALLEL;     // 切換平行投影
    ft_render(fdf);   // 每次按鍵都重新渲染
    return (0);
}

int ft_close(t_fdf *fdf)
{
    ft_free_fdf(fdf);
    exit(EXIT_SUCCESS);
}
```

---

### 5.6 工具函式 `ft_utils.c`

#### `ft_error` — 輸出錯誤並退出

```c
void ft_error(const char *msg)
{
    ft_putstr_fd("Error: ", 2);   // 輸出到 stderr（fd=2）
    ft_putendl_fd((char *)msg, 2);
    exit(EXIT_FAILURE);
}
```

#### `ft_free_fdf` — 釋放所有資源

**釋放順序很重要：** 必須先釋放依賴 `mlx` 的資源，再釋放 `mlx` 本身。

```c
void ft_free_fdf(t_fdf *fdf)
{
    if (!fdf)
        return;
    if (fdf->img.ptr)
        mlx_destroy_image(fdf->mlx, fdf->img.ptr);  // 先釋放影像
    if (fdf->win)
        mlx_destroy_window(fdf->mlx, fdf->win);      // 再釋放視窗
    if (fdf->mlx)
    {
        mlx_destroy_display(fdf->mlx);               // 關閉 X11 連線
        free(fdf->mlx);                              // 釋放 mlx 結構
    }
    ft_free_map(fdf->map);  // 釋放地圖
    free(fdf);              // 釋放主結構
}
```

---

### 5.7 程式入口 `ft_main.c`

```c
static t_fdf *ft_init_fdf(const char *file)
{
    t_fdf *fdf;

    fdf = ft_calloc(1, sizeof(t_fdf));
    if (!fdf)
        ft_error("malloc failed");

    // 1. 解析地圖
    fdf->map = ft_parse_map(file);

    // 2. 初始化 MiniLibX（建立 X11 連線）
    fdf->mlx = mlx_init();
    if (!fdf->mlx)
        ft_error("mlx_init failed");

    // 3. 建立視窗
    fdf->win = mlx_new_window(fdf->mlx, WIN_W, WIN_H, WIN_TITLE);
    if (!fdf->win)
        ft_error("mlx_new_window failed");

    // 4. 建立離屏影像緩衝
    fdf->img.ptr = mlx_new_image(fdf->mlx, WIN_W, WIN_H);
    if (!fdf->img.ptr)
        ft_error("mlx_new_image failed");

    // 5. 取得像素記憶體指標（必須在 mlx_new_image 之後呼叫）
    fdf->img.addr = mlx_get_data_addr(fdf->img.ptr,
                        &fdf->img.bpp, &fdf->img.ll, &fdf->img.endian);

    // 6. 初始化相機（計算初始 zoom 和偏移）
    ft_init_cam(fdf);
    return (fdf);
}

int main(int argc, char **argv)
{
    t_fdf *fdf;

    if (argc != 2)
        ft_error("Usage: ./fdf <map.fdf>");
    fdf = ft_init_fdf(argv[1]);

    // 綁定事件處理函式
    mlx_hook(fdf->win, 2, 1L << 0, ft_keypress, fdf); // 鍵盤
    mlx_hook(fdf->win, 17, 0, ft_close, fdf);          // 視窗關閉

    // 第一次渲染
    ft_render(fdf);

    // 進入事件迴圈（此函式不會返回）
    mlx_loop(fdf->mlx);
    return (0);
}
```

**`mlx_loop` 的作用：**  
啟動 X11 事件迴圈，不斷等待和分派事件（鍵盤、滑鼠、視窗關閉等）。  
這個函式永遠不會返回，所以 `return (0)` 只是為了滿足編譯器。

---

## 6. Bonus 部分：逐步實作

Bonus 部分在強制部分的基礎上增加以下功能：

| 功能 | 實作位置 |
|------|----------|
| X 軸 / Y 軸旋轉 | `ft_project_bonus.c` |
| CONIC 斜二測投影 | `ft_project_bonus.c` |
| 鼠標拖拽旋轉 | `ft_hooks_bonus.c` |
| 滾輪縮放 | `ft_hooks_bonus.c` |
| 三種配色方案 | `ft_utils_bonus.c` |
| HUD 狀態顯示 | `ft_render_bonus.c` |

---

### 6.1 頭文件 `fdf_bonus.h`

在強制版 `fdf.h` 的基礎上，添加以下內容：

**新常數：**

```c
/* Bonus 新增按鍵 */
# define KEY_J  106   // X 軸正向旋轉
# define KEY_K  107   // X 軸負向旋轉
# define KEY_U  117   // Y 軸負向旋轉
# define KEY_O  111   // Y 軸正向旋轉
# define KEY_C  99    // CONIC 投影
# define KEY_G  103   // 切換配色方案

/* X11 滑鼠事件號 */
# define EVT_BTN_PRESS    4   // ButtonPress（含滾輪）
# define EVT_BTN_RELEASE  5   // ButtonRelease
# define EVT_MOUSE_MOVE   6   // MotionNotify（滑鼠移動）

/* 滑鼠按鍵代碼 */
# define BTN_LEFT         1
# define BTN_SCROLL_UP    4   // 滾輪向上
# define BTN_SCROLL_DOWN  5   // 滾輪向下

/* 投影模式（新增 CONIC） */
# define ISO      0
# define PARALLEL 1
# define CONIC    2

/* 配色方案 */
# define SCHEME_FIRE  0
# define SCHEME_GREY  1
# define SCHEME_HEAT  2

/* 各配色的顏色端點 */
# define FIRE_LOW   0x0000FF  // 藍
# define FIRE_HIGH  0xFF4500  // 橙紅
# define GREY_LOW   0x303030  // 深灰
# define GREY_HIGH  0xFFFFFF  // 白
# define HEAT_LOW   0x00FFFF  // 青
# define HEAT_HIGH  0xFFFF00  // 黃
```

**擴充後的 `t_cam`：**

```c
typedef struct s_cam
{
    double  zoom;
    double  x_off;
    double  y_off;
    double  x_rot;         // ← 新增：X 軸旋轉角
    double  y_rot;         // ← 新增：Y 軸旋轉角
    double  z_rot;
    int     proj;          // 0=ISO, 1=PARALLEL, 2=CONIC
    int     color_scheme;  // ← 新增：配色方案
}   t_cam;
```

**新增 `t_mouse`：**

```c
typedef struct s_mouse
{
    int     drag;
    int     prev_x;
    int     prev_y;
}   t_mouse;

typedef struct s_fdf
{
    void    *mlx;
    void    *win;
    t_img   img;
    t_map   *map;
    t_cam   cam;
    t_mouse mouse;  // ← 新增
}   t_fdf;
```

---

### 6.2 三軸旋轉 `ft_project_bonus.c`

#### 三維旋轉矩陣原理

在三維空間中，繞各軸旋轉的矩陣如下：

```
繞 Z 軸旋轉 θ：
  x' =  x·cos(θ) - y·sin(θ)
  y' =  x·sin(θ) + y·cos(θ)
  z' =  z

繞 X 軸旋轉 φ：
  x' =  x
  y' =  y·cos(φ) - z·sin(φ)
  z' =  y·sin(φ) + z·cos(φ)

繞 Y 軸旋轉 ψ：
  x' =  x·cos(ψ) + z·sin(ψ)
  y' =  y
  z' = -x·sin(ψ) + z·cos(ψ)
```

**實作：先 Z 後 X，合併在一個函式**

```c
static void ft_rotate_zx(double *x, double *y, double *z, t_cam *cam)
{
    double xz, yz, yx, zx;

    // 先做 Z 軸旋轉
    xz = *x * cos(cam->z_rot) - *y * sin(cam->z_rot);
    yz = *x * sin(cam->z_rot) + *y * cos(cam->z_rot);

    // 再做 X 軸旋轉（作用在 Z 旋轉後的 y 和 z 上）
    yx = yz * cos(cam->x_rot) - *z * sin(cam->x_rot);
    zx = yz * sin(cam->x_rot) + *z * cos(cam->x_rot);

    *x = xz;
    *y = yx;
    *z = zx;
}
```

**為什麼把 Z 和 X 合併？**  
兩次旋轉可以用不同的中間變數一起算，  
避免兩次獨立函式呼叫時需要重新讀取被修改的 `*y`。

**Y 軸旋轉（獨立函式）：**

```c
static void ft_rotate_y(double *x, double *z, t_cam *cam)
{
    double xr, zr;

    xr =  (*x) * cos(cam->y_rot) + (*z) * sin(cam->y_rot);
    zr = -(*x) * sin(cam->y_rot) + (*z) * cos(cam->y_rot);
    *x = xr;
    *z = zr;
}
```

#### CONIC 投影（斜二測畫法）

斜二測畫法是一種繪圖技術：
- XY 平面保持正交（正視圖）
- Z 軸以 30° 角投影，長度縮為 0.5 倍

```
screen_x = x·zoom + z·0.5·cos(30°) + x_off
screen_y = y·zoom - z·0.5·sin(30°) + y_off
```

```c
static void ft_apply_proj(t_point *p, double x, double y,
                          double z, t_fdf *fdf)
{
    double zoom = fdf->cam.zoom;
    double c30  = cos(M_PI / 6.0);  // cos(30°) ≈ 0.866

    if (fdf->cam.proj == ISO)
    {
        p->x = (x - y) * c30 * zoom + fdf->cam.x_off;
        p->y = (x + y) * sin(M_PI / 6.0) * zoom - z + fdf->cam.y_off;
    }
    else if (fdf->cam.proj == PARALLEL)
    {
        p->x = x * zoom + fdf->cam.x_off;
        p->y = y * zoom + fdf->cam.y_off;
    }
    else  // CONIC（斜二測）
    {
        p->x = x * zoom + z * 0.5 * c30 + fdf->cam.x_off;
        p->y = y * zoom - z * 0.5 * sin(M_PI / 6.0) + fdf->cam.y_off;
    }
    p->z = z;
}
```

#### 完整投影流程

```c
t_point ft_project(int col, int row, t_fdf *fdf)
{
    t_point p;
    double  x, y, z;

    // 步驟 1：以地圖中心為原點
    x = col - fdf->map->cols / 2.0;
    y = row - fdf->map->rows / 2.0;
    z = ft_z_scale(fdf, row, col);

    // 步驟 2：三軸旋轉
    ft_rotate_zx(&x, &y, &z, &fdf->cam);
    ft_rotate_y(&x, &z, &fdf->cam);

    // 步驟 3：取得顏色
    p.color = ft_get_color(col, row, fdf);

    // 步驟 4：投影到螢幕
    ft_apply_proj(&p, x, y, z, fdf);
    return (p);
}
```

---

### 6.3 鍵盤＋滑鼠事件 `ft_hooks_bonus.c`

#### 鍵盤（新增三軸旋轉和 CONIC 投影）

```c
static void ft_key_rot(int key, t_fdf *fdf)
{
    if (key == KEY_Q)
        fdf->cam.z_rot -= ROT_STEP;  // Z 軸（與強制部分相同）
    else if (key == KEY_E)
        fdf->cam.z_rot += ROT_STEP;
    else if (key == KEY_J)
        fdf->cam.x_rot += ROT_STEP;  // X 軸：向前傾
    else if (key == KEY_K)
        fdf->cam.x_rot -= ROT_STEP;  // X 軸：向後仰
    else if (key == KEY_U)
        fdf->cam.y_rot -= ROT_STEP;  // Y 軸：向左偏
    else if (key == KEY_O)
        fdf->cam.y_rot += ROT_STEP;  // Y 軸：向右偏
}

static void ft_key_proj(int key, t_fdf *fdf)
{
    if (key == KEY_I)
        fdf->cam.proj = ISO;
    else if (key == KEY_P)
        fdf->cam.proj = PARALLEL;
    else if (key == KEY_C)
        fdf->cam.proj = CONIC;
    else if (key == KEY_G)
        // (color_scheme + 1) % 3 → 0→1→2→0 循環
        fdf->cam.color_scheme = (fdf->cam.color_scheme + 1) % 3;
}
```

#### 滑鼠事件

**事件 4 (ButtonPress)：** 含滾輪和按鍵按下

```c
int ft_mouse_press(int btn, int x, int y, t_fdf *fdf)
{
    if (btn == BTN_SCROLL_UP)         // 滾輪向上 → 放大
        fdf->cam.zoom *= ZOOM_STEP;
    else if (btn == BTN_SCROLL_DOWN)  // 滾輪向下 → 縮小
        fdf->cam.zoom /= ZOOM_STEP;
    else if (btn == BTN_LEFT)         // 左鍵按下 → 開始拖拽
    {
        fdf->mouse.drag = 1;
        fdf->mouse.prev_x = x;
        fdf->mouse.prev_y = y;
        return (0);   // 不重新渲染（只是記錄起始位置）
    }
    ft_render(fdf);
    return (0);
}
```

**事件 5 (ButtonRelease)：** 停止拖拽

```c
int ft_mouse_release(int btn, int x, int y, t_fdf *fdf)
{
    (void)x;
    (void)y;
    if (btn == BTN_LEFT)
        fdf->mouse.drag = 0;  // 鬆開左鍵時停止拖拽
    return (0);
}
```

**為什麼需要 ButtonRelease 事件？**  
如果沒有這個事件，使用者鬆開滑鼠後移動游標，  
`drag` 仍然為 1，視圖會繼續旋轉（不是我們想要的行為）。

**事件 6 (MotionNotify)：** 滑鼠移動時的拖拽旋轉

```c
int ft_mouse_move(int x, int y, t_fdf *fdf)
{
    int dx, dy;

    if (!fdf->mouse.drag)   // 不在拖拽狀態，忽略
        return (0);
    dx = x - fdf->mouse.prev_x;  // 水平移動量
    dy = y - fdf->mouse.prev_y;  // 垂直移動量
    // 0.01 rad/pixel 的靈敏度
    fdf->cam.z_rot += dx * 0.01;  // 水平拖動 → Z 軸旋轉
    fdf->cam.x_rot += dy * 0.01;  // 垂直拖動 → X 軸傾斜
    fdf->mouse.prev_x = x;        // 更新記錄位置
    fdf->mouse.prev_y = y;
    ft_render(fdf);
    return (0);
}
```

---

### 6.4 渲染 + HUD `ft_render_bonus.c`

#### HUD（抬頭顯示器）原理

`mlx_string_put` 直接把文字畫在**視窗**上（不是影像緩衝區），  
所以它必須在 `mlx_put_image_to_window` **之後**呼叫，  
否則影像緩衝送到螢幕時會覆蓋文字。

```c
void ft_render(t_fdf *fdf)
{
    int row, col;

    // 步驟 1：清空影像緩衝
    ft_bzero(fdf->img.addr, WIN_W * WIN_H * (fdf->img.bpp / 8));

    // 步驟 2：畫所有邊線
    row = 0;
    while (row < fdf->map->rows)
    {
        col = 0;
        while (col < fdf->map->cols)
            ft_draw_edges(fdf, col++, row);
        row++;
    }

    // 步驟 3：把影像緩衝送到螢幕
    mlx_put_image_to_window(fdf->mlx, fdf->win, fdf->img.ptr, 0, 0);

    // 步驟 4：在螢幕最上層疊加 HUD 文字
    ft_draw_hud(fdf);   // ← 必須在 put_image 之後！
}
```

**HUD 狀態顯示（動態字串）：**

```c
static void ft_hud_state(t_fdf *fdf)
{
    char *proj_s;
    char *col_s;
    char *zoom_s;
    char *zoom_l;

    // 選擇顯示文字
    if (fdf->cam.proj == ISO)         proj_s = "Mode: ISO";
    else if (fdf->cam.proj == PARALLEL) proj_s = "Mode: PARALLEL";
    else                              proj_s = "Mode: CONIC";

    if (fdf->cam.color_scheme == SCHEME_GREY)     col_s = "Color: GREY";
    else if (fdf->cam.color_scheme == SCHEME_HEAT) col_s = "Color: HEAT";
    else                                           col_s = "Color: FIRE";

    // zoom 需要轉字串（動態配置，用完要 free）
    zoom_s = ft_itoa((int)fdf->cam.zoom);
    if (!zoom_s)
        return;
    zoom_l = ft_strjoin("Zoom: ", zoom_s);
    free(zoom_s);
    if (zoom_l)
    {
        mlx_string_put(fdf->mlx, fdf->win, 10, 60, 0xFFFF00, zoom_l);
        free(zoom_l);
    }
    mlx_string_put(fdf->mlx, fdf->win, 10, 80,  0xFFFF00, proj_s);
    mlx_string_put(fdf->mlx, fdf->win, 10, 100, 0xFFFF00, col_s);
}
```

---

### 6.5 工具函式（含配色）`ft_utils_bonus.c`

#### 三種配色方案（`ft_scheme_color`）

```c
int ft_scheme_color(int z, t_map *map, int scheme)
{
    double t;
    int    low, high;

    if (map->z_max == map->z_min)
        return (0xFFFFFF);

    // t 是高度的歸一化值：0 = 最低，1 = 最高
    t = (double)(z - map->z_min) / (double)(map->z_max - map->z_min);

    if (scheme == SCHEME_GREY)
    {
        low  = GREY_LOW;   // 0x303030（深灰）
        high = GREY_HIGH;  // 0xFFFFFF（白）
    }
    else if (scheme == SCHEME_HEAT)
    {
        low  = HEAT_LOW;   // 0x00FFFF（青）
        high = HEAT_HIGH;  // 0xFFFF00（黃）
    }
    else  // SCHEME_FIRE（預設）
    {
        low  = FIRE_LOW;   // 0x0000FF（藍）
        high = FIRE_HIGH;  // 0xFF4500（橙紅）
    }
    return (ft_lerp_color(low, high, t));
}
```

| 配色 | 低點顏色 | 高點顏色 | 視覺效果 |
|------|---------|---------|---------|
| FIRE | 藍 `#0000FF` | 橙紅 `#FF4500` | 岩漿流 |
| GREY | 深灰 `#303030` | 白 `#FFFFFF` | 地形高程圖 |
| HEAT | 青 `#00FFFF` | 黃 `#FFFF00` | 熱力圖 |

---

### 6.6 程式入口 `ft_main_bonus.c`

Bonus 版注冊了 **5 個** 事件鉤子（強制版只有 2 個）：

```c
int main(int argc, char **argv)
{
    t_fdf *fdf;

    if (argc != 2)
        ft_error("Usage: ./fdf_bonus <map.fdf>");
    fdf = ft_init_fdf(argv[1]);

    // 事件 2：鍵盤按下
    mlx_hook(fdf->win, 2,  1L << 0, ft_keypress,     fdf);
    // 事件 17：視窗關閉按鈕
    mlx_hook(fdf->win, 17, 0,       ft_close,         fdf);
    // 事件 4：滑鼠按鍵按下（含滾輪）
    mlx_hook(fdf->win, EVT_BTN_PRESS,    1L << 2, ft_mouse_press,   fdf);
    // 事件 5：滑鼠按鍵鬆開
    mlx_hook(fdf->win, EVT_BTN_RELEASE,  1L << 3, ft_mouse_release, fdf);
    // 事件 6：滑鼠移動
    mlx_hook(fdf->win, EVT_MOUSE_MOVE,   1L << 6, ft_mouse_move,    fdf);

    ft_render(fdf);
    mlx_loop(fdf->mlx);
    return (0);
}
```

**MLX 事件掩碼（event mask）說明：**  
掩碼告訴 X11 我們想要監聽哪些事件類型。  
`1L << 6` = `0x40` = X11 的 `PointerMotionMask`，表示監聽滑鼠移動。

---

## 7. 核心演算法詳解

### 7.1 Bresenham 畫線算法視覺化

以從 (0, 0) 到 (6, 2) 為例：

```
理論直線：y = 2x/6 = x/3

步驟  x  y  err    2*err   動作
────  ─  ─  ───    ─────   ─────────────────────────────
  0   0  0   4       8     畫 (0,0)；8 > -2 → x++, err-=2；8 < 6 → y++, err+=6
  1   1  1   8       -     畫 (1,1)；...
...
```

**記憶口訣：**
- 若 `2*err > -dy` → X 步進（橫著走）
- 若 `2*err <  dx` → Y 步進（斜著走）
- 兩個都滿足 → 對角線步進

### 7.2 等角投影推導

**為什麼等角投影用 cos(30°) 和 sin(30°)？**

等角投影把三條軸等角分布：
```
      Y 軸（向上）
       ↑
       │
  X ───┼─── Z
（左斜）   （右斜）
```

在二維螢幕上：
- X 軸對應螢幕左下角方向，角度 210°（= 180° + 30°）
- Y 軸對應螢幕右下角方向，角度 330°（= 360° - 30°）
- Z 軸對應螢幕正上方，角度 90°

把 `(x, y)` 投影到螢幕：
```
screen_x = x·cos(210°) + y·cos(330°) = -x·cos(30°) + y·cos(30°) = (y - x)·cos(30°)
screen_y = x·sin(210°) + y·sin(330°) = -x·sin(30°) - y·sin(30°) = -(x + y)·sin(30°)
```

（螢幕 Y 軸向下，所以正負號會翻轉，配合偏移量 `y_off`）

### 7.3 顏色格式

MiniLibX 使用 **0xAARRGGBB** 格式（A = alpha，通常忽略）：

```
0xFF4500 = 0x00 FF 45 00
            A  R  G  B

提取 R：(color >> 16) & 0xFF = 0xFF = 255
提取 G：(color >> 8)  & 0xFF = 0x45 = 69
提取 B：color         & 0xFF = 0x00 = 0
```

---

## 8. 常見錯誤與修正

| 錯誤現象 | 根本原因 | 正確做法 |
|---------|---------|---------|
| 地圖渲染在視窗角落 | `x_off`/`y_off` 初始為 0 | `ft_init_cam` 中設為 `WIN_W/2`、`WIN_H/2` |
| 山峰像「刺蝟」一樣過高 | Z 縮放沒有歸一化 | `z_scale` 除以 `z_range`，乘以地圖尺寸 |
| 只有橫線沒有直線 | 渲染迴圈只連了右鄰 | 同時連右鄰 `(col+1, row)` 和下鄰 `(col, row+1)` |
| 最後一行或最後一列缺失 | 迴圈條件寫成 `<= rows` | 改成 `< map->rows`（嚴格小於）|
| 畫面閃爍 | 直接用 `mlx_pixel_put` | 改用離屏緩衝 + `mlx_put_image_to_window` |
| GNL 記憶體洩漏 | 每次 `get_next_line` 後未 `free` | 在迴圈末尾 `free(line)` |
| `ft_split` 記憶體洩漏 | 只釋放了指標陣列，未釋放各元素 | 先 `free(tokens[i])`，再 `free(tokens)` |
| 全負值地圖 `z_min` 永遠是 0 | `z_min` 初始值為 0，負值被跳過 | 初始化為 `z[0][0]`（或 `INT_MAX`） |
| HUD 文字被影像覆蓋 | `mlx_string_put` 在 `put_image` 之前呼叫 | 先 `mlx_put_image_to_window`，再 `ft_draw_hud` |
| 拖拽旋轉不會停止 | 沒有綁定 ButtonRelease 事件 | 新增 `ft_mouse_release` 將 `drag` 設回 0 |
| `ft_free_fdf` Segfault | 釋放 `mlx` 後還存取 `img.ptr` | 先釋放 img 和 win，再釋放 mlx |

---

## 9. 記憶體管理檢查清單

完成實作後，用 Valgrind 確認沒有記憶體洩漏：

```bash
# 基本洩漏檢查
valgrind --leak-check=full --show-leak-kinds=all ./fdf maps/42.fdf

# 追蹤檔案描述符洩漏
valgrind --track-fds=yes ./fdf maps/42.fdf
```

**逐項確認：**

- [ ] `get_next_line` 的每個回傳值都有 `free`
- [ ] `ft_split` 的每個 token 都有 `free`，指標陣列也有 `free`
- [ ] `ft_alloc_map` 失敗時呼叫 `ft_free_map` 清理已配置的記憶體
- [ ] `ft_free_map` 釋放順序：各行陣列 → 指標陣列 → `map` 本身
- [ ] `ft_free_fdf` 釋放順序：`img` → `win` → `mlx` → `map` → `fdf`
- [ ] HUD 使用的 `ft_itoa` 和 `ft_strjoin` 的回傳值用完後 `free`
- [ ] 程式在按 ESC 和點擊視窗 × 按鈕時，都會呼叫 `ft_free_fdf`

---

## 10. 操作快捷鍵總表

### 強制部分（`./fdf`）

| 按鍵 | 功能 |
|------|------|
| `W / S` | 畫面向上 / 向下移動 |
| `A / D` | 畫面向左 / 向右移動 |
| `Q / E` | 逆時針 / 順時針旋轉（Z 軸）|
| `+ / -` | 放大 / 縮小 |
| `R` | 重置相機到初始狀態 |
| `I` | 等角投影（Isometric）|
| `P` | 平行投影（Parallel）|
| `ESC` | 退出程式 |

### Bonus 部分（`./fdf_bonus`）

| 按鍵 / 操作 | 功能 |
|------------|------|
| `W / S / A / D` | 平移 |
| `Q / E` | Z 軸旋轉 |
| `J / K` | X 軸傾斜（前後）|
| `U / O` | Y 軸偏轉（左右）|
| `+ / -` | 縮放 |
| `R` | 重置所有相機參數 |
| `I / P / C` | 切換 ISO / PARALLEL / CONIC 投影 |
| `G` | 循環切換配色方案（FIRE → GREY → HEAT）|
| 左鍵拖拽 | 旋轉視角（水平 → Z 軸，垂直 → X 軸）|
| 滾輪 ↑ / ↓ | 放大 / 縮小 |
| `ESC` 或 × | 退出程式 |

---

## 建置系統備忘

```bash
# 只重新建置強制部分
make re

# 只建置 Bonus
make fclean && make bonus

# 查詢按鍵代碼（在 X11 環境）
xev | grep keycode
```

---

*本 README 以台灣繁體中文撰寫，作為從零實作 FdF 專案的完整程式設計指南。*
