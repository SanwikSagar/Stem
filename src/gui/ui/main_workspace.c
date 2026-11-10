#include "screens.h"
#include "theme.h"

lv_obj_t* ui_create_main_workspace(lv_obj_t *parent, stem_repository_t *repo) {
    lv_obj_t *workspace = lv_obj_create(parent);
    lv_obj_set_style_bg_color(workspace, COLOR_BG, 0);
    lv_obj_set_style_border_width(workspace, 0, 0);
    lv_obj_set_style_pad_all(workspace, SPACING_XL, 0);
    
    /* Title */
    lv_obj_t *title = lv_label_create(workspace);
    lv_label_set_text(title, "Commit History");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_pad_bottom(title, SPACING_LG, 0);
    
    /* Commit list */
    lv_obj_t *commit_list = lv_obj_create(workspace);
    lv_obj_set_width(commit_list, lv_pct(100));
    lv_obj_set_flex_grow(commit_list, 1);
    lv_obj_set_flex_flow(commit_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(commit_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(commit_list, 0, 0);
    lv_obj_set_style_pad_row(commit_list, SPACING_MD, 0);
    
    /* Sample commits */
    for (int i = 0; i < 10; i++) {
        lv_obj_t *commit_card = lv_obj_create(commit_list);
        lv_obj_add_style(commit_card, ui_style_card(), 0);
        lv_obj_set_width(commit_card, lv_pct(100));
        lv_obj_set_height(commit_card, LV_SIZE_CONTENT);
        
        lv_obj_t *commit_title = lv_label_create(commit_card);
        lv_label_set_text(commit_title, "Add new feature");
        lv_obj_set_style_text_font(commit_title, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(commit_title, COLOR_TEXT_PRIMARY, 0);
        
        lv_obj_t *commit_meta = lv_label_create(commit_card);
        lv_label_set_text(commit_meta, "John Doe • 2 hours ago • abc1234");
        lv_obj_set_style_text_color(commit_meta, COLOR_TEXT_SECONDARY, 0);
        lv_obj_set_style_text_font(commit_meta, &lv_font_montserrat_12, 0);
    }
    
    return workspace;
}
