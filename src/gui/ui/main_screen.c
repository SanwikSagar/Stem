#include "screens.h"
#include "theme.h"
#include "stem/stem.h"

static lv_obj_t *main_screen = NULL;
static stem_repository_t *current_repo = NULL;

void ui_show_main_screen(void) {
    main_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(main_screen, COLOR_BG, 0);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
    
    /* Create main layout */
    lv_obj_set_flex_flow(main_screen, LV_FLEX_FLOW_COLUMN);
    
    /* Top Bar */
    lv_obj_t *topbar = ui_create_topbar(main_screen, current_repo);
    lv_obj_set_height(topbar, 56);
    
    /* Content area */
    lv_obj_t *content = lv_obj_create(main_screen);
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_width(content, lv_pct(100));
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    
    /* Left Sidebar */
    lv_obj_t *left_sidebar = ui_create_left_sidebar(content, current_repo);
    lv_obj_set_width(left_sidebar, 240);
    
    /* Main Workspace */
    lv_obj_t *workspace = ui_create_main_workspace(content, current_repo);
    lv_obj_set_flex_grow(workspace, 1);
    
    /* Right Sidebar */
    lv_obj_t *right_sidebar = ui_create_right_sidebar(content, current_repo);
    lv_obj_set_width(right_sidebar, 280);
    
    /* Bottom Panel */
    lv_obj_t *bottom_panel = ui_create_bottom_panel(main_screen, current_repo);
    lv_obj_set_height(bottom_panel, 280);
    
    /* Status Bar */
    lv_obj_t *status_bar = ui_create_status_bar(main_screen, current_repo);
    lv_obj_set_height(status_bar, 28);
    
    /* Load screen */
    lv_screen_load(main_screen);
}
