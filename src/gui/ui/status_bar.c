#include "screens.h"
#include "theme.h"

lv_obj_t* ui_create_status_bar(lv_obj_t *parent, stem_repository_t *repo) {
    lv_obj_t *status_bar = lv_obj_create(parent);
    lv_obj_set_width(status_bar, lv_pct(100));
    lv_obj_add_style(status_bar, ui_style_panel(), 0);
    lv_obj_set_style_border_side(status_bar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_pad_hor(status_bar, SPACING_LG, 0);
    lv_obj_set_style_pad_ver(status_bar, 4, 0);
    lv_obj_set_flex_flow(status_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    /* Left: Branch and changes */
    lv_obj_t *left = lv_obj_create(status_bar);
    lv_obj_set_size(left, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_pad_column(left, SPACING_LG, 0);
    
    lv_obj_t *branch = lv_label_create(left);
    lv_label_set_text(branch, LV_SYMBOL_SHUFFLE " main");
    lv_obj_set_style_text_color(branch, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(branch, &lv_font_montserrat_12, 0);
    
    lv_obj_t *changes = lv_label_create(left);
    lv_label_set_text(changes, LV_SYMBOL_UP " 2  " LV_SYMBOL_DOWN " 0");
    lv_obj_set_style_text_color(changes, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(changes, &lv_font_montserrat_12, 0);
    
    /* Right: Sync status */
    lv_obj_t *right = lv_obj_create(status_bar);
    lv_obj_set_size(right, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_pad_column(right, SPACING_LG, 0);
    
    lv_obj_t *sync_status = lv_label_create(right);
    lv_label_set_text(sync_status, LV_SYMBOL_OK " Synced");
    lv_obj_set_style_text_color(sync_status, COLOR_SUCCESS, 0);
    lv_obj_set_style_text_font(sync_status, &lv_font_montserrat_12, 0);
    
    lv_obj_t *storage = lv_label_create(right);
    lv_label_set_text(storage, "4.2 GB / 15 GB");
    lv_obj_set_style_text_color(storage, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(storage, &lv_font_montserrat_12, 0);
    
    return status_bar;
}
