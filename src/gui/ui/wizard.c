#include "screens.h"
#include "theme.h"
#include "stem/stem.h"
#include <string.h>

static lv_obj_t *wizard_screen = NULL;
static lv_obj_t *storage_input = NULL;

static void on_create_clicked(lv_event_t *e) {
    const char *storage_url = lv_textarea_get_text(storage_input);
    
    if (!storage_url || strlen(storage_url) == 0) {
        /* Show error */
        return;
    }
    
    /* TODO: Initialize repository with storage URL */
    
    /* Switch to main screen */
    lv_obj_del(wizard_screen);
    ui_show_main_screen();
}

static void on_open_clicked(lv_event_t *e) {
    /* TODO: Show file picker */
}

void ui_show_wizard_screen(void) {
    wizard_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(wizard_screen, COLOR_BG, 0);
    lv_obj_clear_flag(wizard_screen, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Center container */
    lv_obj_t *container = lv_obj_create(wizard_screen);
    lv_obj_set_size(container, 600, 500);
    lv_obj_center(container);
    lv_obj_add_style(container, ui_style_card(), 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    /* Logo */
    lv_obj_t *logo = lv_label_create(container);
    lv_label_set_text(logo, LV_SYMBOL_HOME " STEM");
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(logo, COLOR_ACCENT, 0);
    lv_obj_set_style_pad_bottom(logo, SPACING_XL, 0);
    
    /* Title */
    lv_obj_t *title = lv_label_create(container);
    lv_label_set_text(title, "Welcome to STEM");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_pad_bottom(title, SPACING_SM, 0);
    
    /* Subtitle */
    lv_obj_t *subtitle = lv_label_create(container);
    lv_label_set_text(subtitle, "Cloud-first version control system");
    lv_obj_set_style_text_color(subtitle, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_pad_bottom(subtitle, SPACING_2XL, 0);
    
    /* Storage URL input */
    lv_obj_t *input_label = lv_label_create(container);
    lv_label_set_text(input_label, "Cloud Storage URL:");
    lv_obj_set_style_text_color(input_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_pad_bottom(input_label, SPACING_SM, 0);
    lv_obj_set_width(input_label, lv_pct(80));
    
    storage_input = lv_textarea_create(container);
    lv_textarea_set_one_line(storage_input, true);
    lv_textarea_set_placeholder_text(storage_input, "gdrive://MyDrive/Projects/MyRepo");
    lv_obj_set_width(storage_input, lv_pct(80));
    lv_obj_add_style(storage_input, ui_style_input(), 0);
    lv_obj_set_style_pad_bottom(storage_input, SPACING_SM, 0);
    
    /* Help text */
    lv_obj_t *help = lv_label_create(container);
    lv_label_set_text(help, "Supports: gdrive://, dropbox://, onedrive://, s3://, file://");
    lv_obj_set_style_text_color(help, COLOR_TEXT_TERTIARY, 0);
    lv_obj_set_style_text_font(help, &lv_font_montserrat_12, 0);
    lv_obj_set_style_pad_bottom(help, SPACING_XL, 0);
    
    /* Buttons */
    lv_obj_t *btn_container = lv_obj_create(container);
    lv_obj_set_size(btn_container, lv_pct(80), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_set_style_pad_column(btn_container, SPACING_MD, 0);
    
    /* Create button */
    lv_obj_t *btn_create = lv_btn_create(btn_container);
    lv_obj_set_flex_grow(btn_create, 1);
    lv_obj_add_style(btn_create, ui_style_button_primary(), 0);
    lv_obj_add_event_cb(btn_create, on_create_clicked, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *label_create = lv_label_create(btn_create);
    lv_label_set_text(label_create, "Create New");
    lv_obj_center(label_create);
    
    /* Open button */
    lv_obj_t *btn_open = lv_btn_create(btn_container);
    lv_obj_set_flex_grow(btn_open, 1);
    lv_obj_add_style(btn_open, ui_style_button_secondary(), 0);
    lv_obj_add_event_cb(btn_open, on_open_clicked, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *label_open = lv_label_create(btn_open);
    lv_label_set_text(label_open, "Open Existing");
    lv_obj_center(label_open);
    
    /* Load screen */
    lv_screen_load(wizard_screen);
}
