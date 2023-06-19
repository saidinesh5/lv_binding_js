#include "./dji.hpp"

#include "./dji_display.h"
#include "./dji_services.h"
#include "./evdev.h"
#include <stdio.h>

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 810
#define BYTES_PER_PIXEL 4
#define PLANE_ID 6
#define BUFSIZE WIDTH * 100

static dji_display_state_t *dji_display;
static void *fb0_addr;
static void *fb1_addr;

static lv_indev_t * keypad_indev;

void dji_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p) {
  //printf("%p - %p - %p\n", fb0_addr, fb1_addr, (void*)color_p);
  //printf("x1 - %d \t x2 \t - %d \t y1 - %d \t y2 %d\n", area->x1, area->x2, area->y1, area->y2);
  //printf("w - %d \t h - %d\n", area->x2 - area->x1, area->y2 - area->y1);
  char* fb = (char *) color_p;
  char* otherfb = ((void*)fb == (void*)fb0_addr ? (char *)fb1_addr : (char *)fb0_addr);
  bool whichfb = ((void*)fb == (void*)fb0_addr ? 0 : 1);

  if(lv_disp_flush_is_last(drv)) {
    lv_disp_t * display = _lv_refr_get_disp_refreshing();
    for(int a = 0; a<display->inv_p; a++) {
      if(display->inv_area_joined[a]) {
        continue;
      }
      lv_area_t invarea = display->inv_areas[a];
      //printf("x1 - %d \t x2 \t - %d \t y1 - %d \t y2 %d\n", invarea.x1, invarea.x2, invarea.y1, invarea.y2);

      for(int y = invarea.y1; y <= invarea.y2; y++) {
        int start = (y*SCREEN_WIDTH*BYTES_PER_PIXEL)+(invarea.x1*BYTES_PER_PIXEL)+3;
        int end = (y*SCREEN_WIDTH*BYTES_PER_PIXEL)+(invarea.x2*BYTES_PER_PIXEL)+3;
        for(int x = start; x <= end; x=x+BYTES_PER_PIXEL) {
          fb[x] = ~fb[x];
          /*otherfb[x] = fb[x];
          otherfb[x-1] = fb[x-1];
          otherfb[x-2] = fb[x-2];
          otherfb[x-3] = fb[x-3];*/
        }
        //i don't know if this is actually faster than the commented out bit above
        //it might also depend on the invalidated regions size
        memcpy((void*)(&otherfb[start-3]), (void*)(&fb[start-3]), end-start);
      }
    }
    dji_display_push_frame(dji_display, whichfb);
  }

  lv_disp_flush_ready(drv);
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
void hal_init() {
  dji_display = dji_display_state_alloc(dji_goggles_are_v2());
  dji_display_open_framebuffer(dji_display, PLANE_ID);

  /*Create a display buffer*/

  static lv_disp_draw_buf_t disp_buf1;
  fb0_addr = dji_display_get_fb_address(dji_display, 0);
  fb1_addr = dji_display_get_fb_address(dji_display, 1);
  memset(fb0_addr, 0x000000FF, SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
  memset(fb1_addr, 0x000000FF, SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
  lv_disp_draw_buf_init(&disp_buf1, fb0_addr, fb1_addr, SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);

  /*Create a display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv); /*Basic initialization*/
  disp_drv.draw_buf = &disp_buf1;
  disp_drv.flush_cb = dji_display_flush;
  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;
  //disp_drv.set_px_cb = dji_set_px_cb;
  disp_drv.full_refresh = 0;
  disp_drv.direct_mode = 1;
  //you'd think we'd want transp = 1 but it behaves real weird
  //also if set to 1 a clear_cb is required otherwise we get a sigsev
  //which makes sense if you read the code
  disp_drv.screen_transp = 0;

  lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

  lv_theme_t *th = lv_theme_default_init(
      disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
      LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
  lv_disp_set_theme(disp, th);



  evdev_init();
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_KEYPAD;
  indev_drv.read_cb = evdev_read;
  keypad_indev = lv_indev_drv_register(&indev_drv);
  
  
  lv_group_t *g = lv_group_create();
  lv_group_set_default(g);
  lv_indev_set_group(keypad_indev, g);
}
