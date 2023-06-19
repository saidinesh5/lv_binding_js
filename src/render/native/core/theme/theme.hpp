#pragma once

extern "C" {
    #include "lvgl.h"
    #include "quickjs-libc.h"
    #include "sjs.h"
};

#include "native/core/style/font/font.hpp"

extern lv_theme_t theme_default;

extern bool theme_default_init;

void NativeThemeInit (JSContext* ctx, JSValue& ns);
