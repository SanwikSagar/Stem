#include "screens.h"
#include "theme.h"

lv_obj_t* ui_create_right_sidebar(lv_obj_t *parent, stem_repository_t *repo) {
    lv_obj_t *sidebar = lv_obj_create(parent);
    lv_obj_add_style(sidebar, ui_style_panel(), 0);
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_border_side(sidebar, LV_BORDER_SIDE_LEFT, 0);
    lv_obj_set_style_pad_all(sidebar, SPACING_LG, 0);
    lv_obj_set_style_pad_row(sidebar, SPACING_LG, 0);
    
    /* Cloud Storage Card */
    lv_obj_t *cloud_card = lv_obj_create(sidebar);
    lv_obj_add_style(cloud_card, ui_style_card(), 0);
    lv_obj_set_width(cloud_card, lv_pct(100));
    lv_obj_set_height(cloud_card, LV_SIZE_CONTENT);
    
    lv_obj_t *cloud_title = lv_label_create(cloud_card);
    lv_label_set_text(cloud_title, "Cloud Storage");
    lv_obj_set_style_text_font(cloud_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(cloud_title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_pad_bottom(cloud_title, SPACING_MD, 0);
    
    lv_obj_t *cloud_info = lv_label_create(cloud_card);
    lv_label_set_text(cloud_info, 
        LV_SYMBOL_WIFI " Google Drive\n"
        "Storage: 4.2 GB / 15 GB\n"
        LV_SYMBOL_OK " Connected");
    lv_obj_set_style_text_color(cloud_info, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(cloud_info, &lv_font_montserrat_12, 0);
    
    /* Repository Status Card */
    lv_obj_t *status_card = lv_obj_create(sidebar);
    lv_obj_add_style(status_card, ui_style_card(), 0);
    lv_obj_set_width(status_card, lv_pct(100));
    lv_obj_set_height(status_card, LV_SIZE_CONTENT);
    
    lv_obj_t *status_title = lv_label_create(status_card);
    lv_label_set_text(status_title, "Repository Status");
    lv_obj_set_style_text_font(status_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(status_title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_pad_bottom(status_title, SPACING_MD, 0);
    
    lv_obj_t *status_info = lv_label_create(status_card);
    lv_label_set_text(status_info,
        "Branch: main\n"
        "Staged: 3 files\n"
        "Untracked: 5 files\n"
        "Clean: Yes");
    lv_obj_set_style_text_color(status_info, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(status_info, &lv_font_montserrat_12, 0);
    
    /* Quick Actions Card */
    lv_obj_t *actions_card = lv_obj_create(sidebar);
    lv_obj_add_style(actions_card, ui_style_card(), 0);
    lv_obj_set_width(actions_card, lv_pct(100));
    lv_obj_set_height(actions_card, LV_SIZE_CONTENT);
    
    lv_obj_t *actions_title = lv_label_create(actions_card);
    lv_label_set_text(actions_title, "Quick Actions");
    lv_obj_set_style_text_font(actions_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(actions_title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_pad_bottom(actions_title, SPACING_MD, 0);
    
    const char *actions[] = {
        "Stage All",
        "Commit",
        "Discard",
        "Sync"
    };
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t *btn = lv_btn_create(actions_card);
        lv_obj_set_width(btn, lv_pct(100));
        lv_obj_add_style(btn, ui_style_button_primary(), 0);
        lv_obj_set_style_margin_bottom(btn, SPACING_SM, 0);
        
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, actions[i]);
        lv_obj_center(label);
    }
    
    return sidebar;
}
