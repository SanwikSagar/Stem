#ifndef STEM_UI_THEME_H
#define STEM_UI_THEME_H

#include "lvgl/lvgl.h"

/* Color palette */
#define COLOR_BG           lv_color_hex(0x0a0a0a)
#define COLOR_BG_ELEVATED  lv_color_hex(0x141414)
#define COLOR_BG_PANEL     lv_color_hex(0x1a1a1a)
#define COLOR_BG_HOVER     lv_color_hex(0x252525)
#define COLOR_BG_ACTIVE    lv_color_hex(0x2d2d2d)
#define COLOR_BORDER       lv_color_hex(0x2a2a2a)
#define COLOR_BORDER_HOVER lv_color_hex(0x3a3a3a)

#define COLOR_TEXT_PRIMARY   lv_color_hex(0xe8e8e8)
#define COLOR_TEXT_SECONDARY lv_color_hex(0x9e9e9e)
#define COLOR_TEXT_TERTIARY  lv_color_hex(0x6e6e6e)

#define COLOR_ACCENT       lv_color_hex(0x3b82f6)
#define COLOR_ACCENT_HOVER lv_color_hex(0x2563eb)
#define COLOR_SUCCESS      lv_color_hex(0x10b981)
#define COLOR_WARNING      lv_color_hex(0xf59e0b)
#define COLOR_ERROR        lv_color_hex(0xef4444)

/* Spacing */
#define SPACING_XS  4
#define SPACING_SM  8
#define SPACING_MD  12
#define SPACING_LG  16
#define SPACING_XL  24
#define SPACING_2XL 32
#define SPACING_3XL 48

/* Border radius */
#define RADIUS_SM 6
#define RADIUS_MD 8
#define RADIUS_LG 12
#define RADIUS_XL 16

/* Initialize theme */
void ui_theme_init(void);

/* Apply theme to object */
void ui_theme_apply(lv_obj_t *obj);

/* Style helpers */
lv_style_t* ui_style_button_primary(void);
lv_style_t* ui_style_button_secondary(void);
lv_style_t* ui_style_button_ghost(void);
lv_style_t* ui_style_card(void);
lv_style_t* ui_style_input(void);
lv_style_t* ui_style_panel(void);

#endif /* STEM_UI_THEME_H */
