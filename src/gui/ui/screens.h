#ifndef STEM_UI_SCREENS_H
#define STEM_UI_SCREENS_H

#include "lvgl/lvgl.h"
#include "stem/repository.h"

/* Repository wizard screen */
void ui_show_wizard_screen(void);

/* Main application screen */
void ui_show_main_screen(void);

/* Screen components */
lv_obj_t* ui_create_topbar(lv_obj_t *parent, stem_repository_t *repo);
lv_obj_t* ui_create_left_sidebar(lv_obj_t *parent, stem_repository_t *repo);
lv_obj_t* ui_create_main_workspace(lv_obj_t *parent, stem_repository_t *repo);
lv_obj_t* ui_create_right_sidebar(lv_obj_t *parent, stem_repository_t *repo);
lv_obj_t* ui_create_bottom_panel(lv_obj_t *parent, stem_repository_t *repo);
lv_obj_t* ui_create_status_bar(lv_obj_t *parent, stem_repository_t *repo);

#endif /* STEM_UI_SCREENS_H */
