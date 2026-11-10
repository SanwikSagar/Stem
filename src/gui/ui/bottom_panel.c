#include "screens.h"
#include "theme.h"

lv_obj_t* ui_create_bottom_panel(lv_obj_t *parent, stem_repository_t *repo) {
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_add_style(panel, ui_style_panel(), 0);
    lv_obj_set_style_border_side(panel, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_pad_all(panel, SPACING_LG, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(panel, SPACING_LG, 0);
    
    /* Left: Commit message */
    lv_obj_t *left = lv_obj_create(panel);
    lv_obj_set_flex_grow(left, 1);
    lv_obj_set_height(left, lv_pct(100));
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_pad_all(left, 0, 0);
    
    lv_obj_t *msg_label = lv_label_create(left);
    lv_label_set_text(msg_label, "Commit Message");
    lv_obj_set_style_text_color(msg_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(msg_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_bottom(msg_label, SPACING_SM, 0);
    
    lv_obj_t *textarea = lv_textarea_create(left);
    lv_obj_set_width(textarea, lv_pct(100));
    lv_obj_set_flex_grow(textarea, 1);
    lv_textarea_set_placeholder_text(textarea, "Enter commit message...");
    lv_obj_add_style(textarea, ui_style_input(), 0);
    
    lv_obj_t *checkbox = lv_checkbox_create(left);
    lv_checkbox_set_text(checkbox, "Amend last commit");
    lv_obj_set_style_text_color(checkbox, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_pad_top(checkbox, SPACING_SM, 0);
    
    /* Right: Staged files */
    lv_obj_t *right = lv_obj_create(panel);
    lv_obj_set_width(right, 320);
    lv_obj_set_height(right, lv_pct(100));
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_pad_all(right, 0, 0);
    
    lv_obj_t *files_label = lv_label_create(right);
    lv_label_set_text(files_label, "Staged Files");
    lv_obj_set_style_text_color(files_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(files_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_bottom(files_label, SPACING_SM, 0);
    
    /* File list */
    const char *files[] = {
        "src/main.c",
        "include/header.h",
        "README.md"
    };
    
    for (int i = 0; i < 3; i++) {
        lv_obj_t *cb = lv_checkbox_create(right);
        lv_checkbox_set_text(cb, files[i]);
        lv_obj_set_style_text_color(cb, COLOR_TEXT_PRIMARY, 0);
        lv_obj_set_style_pad_ver(cb, SPACING_SM, 0);
        lv_obj_add_state(cb, LV_STATE_CHECKED);
    }
    
    /* Commit button */
    lv_obj_t *btn_commit = lv_btn_create(right);
    lv_obj_set_width(btn_commit, lv_pct(100));
    lv_obj_add_style(btn_commit, ui_style_button_primary(), 0);
    lv_obj_set_style_margin_top(btn_commit, SPACING_MD, 0);
    
    lv_obj_t *label_commit = lv_label_create(btn_commit);
    lv_label_set_text(label_commit, LV_SYMBOL_OK " Commit");
    lv_obj_center(label_commit);
    
    return panel;
}
