#include "theme.h"
#include <stdlib.h>

/* Global styles */
static lv_style_t style_button_primary;
static lv_style_t style_button_secondary;
static lv_style_t style_button_ghost;
static lv_style_t style_card;
static lv_style_t style_input;
static lv_style_t style_panel;

void ui_theme_init(void) {
    /* Set default theme properties */
    lv_theme_t *theme = lv_theme_default_init(
        NULL,
        COLOR_ACCENT,
        COLOR_ERROR,
        true,  /* dark mode */
        LV_FONT_DEFAULT
    );
    lv_display_set_theme(lv_display_get_default(), theme);
    
    /* Initialize button primary style */
    lv_style_init(&style_button_primary);
    lv_style_set_bg_color(&style_button_primary, COLOR_ACCENT);
    lv_style_set_bg_opa(&style_button_primary, LV_OPA_COVER);
    lv_style_set_radius(&style_button_primary, RADIUS_MD);
    lv_style_set_pad_all(&style_button_primary, SPACING_MD);
    lv_style_set_text_color(&style_button_primary, lv_color_white());
    lv_style_set_shadow_width(&style_button_primary, 4);
    lv_style_set_shadow_color(&style_button_primary, lv_color_black());
    lv_style_set_shadow_opa(&style_button_primary, LV_OPA_30);
    
    /* Initialize button secondary style */
    lv_style_init(&style_button_secondary);
    lv_style_set_bg_color(&style_button_secondary, COLOR_BG_ELEVATED);
    lv_style_set_bg_opa(&style_button_secondary, LV_OPA_COVER);
    lv_style_set_radius(&style_button_secondary, RADIUS_MD);
    lv_style_set_pad_all(&style_button_secondary, SPACING_MD);
    lv_style_set_text_color(&style_button_secondary, COLOR_TEXT_PRIMARY);
    lv_style_set_border_width(&style_button_secondary, 1);
    lv_style_set_border_color(&style_button_secondary, COLOR_BORDER);
    
    /* Initialize button ghost style */
    lv_style_init(&style_button_ghost);
    lv_style_set_bg_opa(&style_button_ghost, LV_OPA_TRANSP);
    lv_style_set_radius(&style_button_ghost, RADIUS_MD);
    lv_style_set_pad_all(&style_button_ghost, SPACING_MD);
    lv_style_set_text_color(&style_button_ghost, COLOR_TEXT_SECONDARY);
    
    /* Initialize card style */
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_BG_PANEL);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, RADIUS_XL);
    lv_style_set_pad_all(&style_card, SPACING_LG);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, COLOR_BORDER);
    lv_style_set_shadow_width(&style_card, 2);
    lv_style_set_shadow_color(&style_card, lv_color_black());
    lv_style_set_shadow_opa(&style_card, LV_OPA_25);
    
    /* Initialize input style */
    lv_style_init(&style_input);
    lv_style_set_bg_color(&style_input, COLOR_BG_ELEVATED);
    lv_style_set_bg_opa(&style_input, LV_OPA_COVER);
    lv_style_set_radius(&style_input, RADIUS_MD);
    lv_style_set_pad_all(&style_input, SPACING_MD);
    lv_style_set_text_color(&style_input, COLOR_TEXT_PRIMARY);
    lv_style_set_border_width(&style_input, 1);
    lv_style_set_border_color(&style_input, COLOR_BORDER);
    
    /* Initialize panel style */
    lv_style_init(&style_panel);
    lv_style_set_bg_color(&style_panel, COLOR_BG_ELEVATED);
    lv_style_set_bg_opa(&style_panel, LV_OPA_COVER);
    lv_style_set_pad_all(&style_panel, 0);
    lv_style_set_border_width(&style_panel, 1);
    lv_style_set_border_color(&style_panel, COLOR_BORDER);
}

lv_style_t* ui_style_button_primary(void) {
    return &style_button_primary;
}

lv_style_t* ui_style_button_secondary(void) {
    return &style_button_secondary;
}

lv_style_t* ui_style_button_ghost(void) {
    return &style_button_ghost;
}

lv_style_t* ui_style_card(void) {
    return &style_card;
}

lv_style_t* ui_style_input(void) {
    return &style_input;
}

lv_style_t* ui_style_panel(void) {
    return &style_panel;
}
