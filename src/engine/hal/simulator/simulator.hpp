#pragma once

#include <SDL2/SDL.h>
#include "deps/lvgl/lvgl.h"
#include "deps/lv_drivers/sdl/sdl.h"

extern void* buf;

extern lv_coord_t hor_res;
extern lv_coord_t ver_res;

using read_cb = void (*)(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

extern read_cb read_cb1;
extern read_cb read_cb2;
extern read_cb read_cb3;
