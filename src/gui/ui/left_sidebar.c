#include "screens.h"
#include "theme.h"

static void on_nav_clicked(lv_event_t *e) {
    /* TODO: Handle navigation */
}

lv_obj_t* ui_create_left_sidebar(lv_obj_t *parent, stem_repository_t *repo) {
    lv_obj_t *sidebar = lv_obj_create(parent);
    lv_obj_add_style(sidebar, ui_style_panel(), 0);
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_border_side(sidebar, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_pad_all(sidebar, SPACING_LG, 0);
    
    /* Navigation */
    const char *nav_items[] = {
        LV_SYMBOL_HOME " Overview",
        LV_SYMBOL_FILE " Changes",
        LV_SYMBOL_LIST " History",
        LV_SYMBOL_SHUFFLE " Branches",
        LV_SYMBOL_DOWNLOAD " Remotes",
        LV_SYMBOL_SETTINGS " Settings"
    };
    
    for (int i = 0; i < 6; i++) {
        lv_obj_t *btn = lv_btn_create(sidebar);
        lv_obj_set_width(btn, lv_pct(100));
        lv_obj_set_height(btn, 40);
        lv_obj_add_style(btn, ui_style_button_ghost(), 0);
        lv_obj_add_event_cb(btn, on_nav_clicked, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_LEFT, 0);
        
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, nav_items[i]);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, SPACING_MD, 0);
    }
    
    /* Separator */
    lv_obj_t *sep = lv_obj_create(sidebar);
    lv_obj_set_height(sep, 1);
    lv_obj_set_width(sep, lv_pct(100));
    lv_obj_set_style_bg_color(sep, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(sep, 0, 0);
    lv_obj_set_style_margin_ver(sep, SPACING_MD, 0);
    
    /* File tree */
    lv_obj_t *tree_label = lv_label_create(sidebar);
    lv_label_set_text(tree_label, "Repository Explorer");
    lv_obj_set_style_text_color(tree_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(tree_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_pad_bottom(tree_label, SPACING_SM, 0);
    
    /* TODO: Add file tree */
    
    return sidebar;
}
