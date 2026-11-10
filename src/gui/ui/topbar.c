#include "screens.h"
#include "theme.h"

lv_obj_t* ui_create_topbar(lv_obj_t *parent, stem_repository_t *repo) {
    lv_obj_t *topbar = lv_obj_create(parent);
    lv_obj_set_width(topbar, lv_pct(100));
    lv_obj_add_style(topbar, ui_style_panel(), 0);
    lv_obj_set_flex_flow(topbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(topbar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(topbar, SPACING_XL, 0);
    lv_obj_set_style_pad_ver(topbar, 0, 0);
    lv_obj_set_style_border_side(topbar, LV_BORDER_SIDE_BOTTOM, 0);
    
    /* Left: Logo and Repo Name */
    lv_obj_t *left = lv_obj_create(topbar);
    lv_obj_set_size(left, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_pad_column(left, SPACING_MD, 0);
    
    lv_obj_t *logo = lv_label_create(left);
    lv_label_set_text(logo, LV_SYMBOL_HOME);
    lv_obj_set_style_text_color(logo, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_20, 0);
    
    lv_obj_t *repo_name = lv_label_create(left);
    lv_label_set_text(repo_name, repo ? "My Project" : "STEM");
    lv_obj_set_style_text_color(repo_name, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(repo_name, &lv_font_montserrat_16, 0);
    
    /* Center: Actions */
    lv_obj_t *center = lv_obj_create(topbar);
    lv_obj_set_size(center, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(center, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(center, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(center, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(center, 0, 0);
    lv_obj_set_style_pad_column(center, SPACING_SM, 0);
    
    /* Branch button */
    lv_obj_t *btn_branch = lv_btn_create(center);
    lv_obj_add_style(btn_branch, ui_style_button_secondary(), 0);
    lv_obj_set_size(btn_branch, LV_SIZE_CONTENT, 32);
    
    lv_obj_t *label_branch = lv_label_create(btn_branch);
    lv_label_set_text(label_branch, LV_SYMBOL_SHUFFLE " main");
    lv_obj_center(label_branch);
    
    /* Action buttons */
    const char *actions[] = {
        LV_SYMBOL_DOWNLOAD " Pull",
        LV_SYMBOL_UPLOAD " Push", 
        LV_SYMBOL_REFRESH " Sync",
        LV_SYMBOL_CALL " Merge"
    };
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t *btn = lv_btn_create(center);
        lv_obj_add_style(btn, ui_style_button_ghost(), 0);
        lv_obj_set_size(btn, LV_SIZE_CONTENT, 32);
        
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, actions[i]);
        lv_obj_center(label);
    }
    
    /* Right: Settings */
    lv_obj_t *right = lv_obj_create(topbar);
    lv_obj_set_size(right, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_pad_column(right, SPACING_XS, 0);
    
    const char *icons[] = {
        LV_SYMBOL_SEARCH,
        LV_SYMBOL_SETTINGS
    };
    
    for (int i = 0; i < 2; i++) {
        lv_obj_t *btn = lv_btn_create(right);
        lv_obj_add_style(btn, ui_style_button_ghost(), 0);
        lv_obj_set_size(btn, 32, 32);
        
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, icons[i]);
        lv_obj_center(label);
    }
    
    return topbar;
}
