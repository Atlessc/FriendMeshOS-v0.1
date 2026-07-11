#pragma once

#include "lvgl.h"
class Themes
{
  public:
    enum Theme {
        eCleanModern = 0,
        eRetroTerminal = 1,
        eNeobrutalist = 2,
        eOrbitalMission = 3,
        eAlpineDaylight = 4,
        eFriendlyMesh = 5,
        eThemeCount = 6,

        // Compatibility aliases for the upstream persisted values.
        eDark = eCleanModern,
        eLight = eRetroTerminal,
        eRed = eNeobrutalist
    };

    static void initStyles(void);
    static enum Theme get(void);
    static enum Theme normalize(uint32_t value);
    static const char *name(enum Theme theme);
    static lv_color_t accentColor(void);
    static lv_color_t mutedColor(void);
    static lv_color_t focusColor(void);
    static lv_color_t disabledColor(void);
    static lv_color_t warningColor(void);
    static lv_color_t errorColor(void);
    static lv_color_t sosColor(void);
    static void set(enum Theme th);
    static void recolorButton(lv_obj_t *obj, bool enabled, lv_opa_t opa = 255);
    static void recolorImage(lv_obj_t *obj, bool enabled);
    static void recolorText(lv_obj_t *obj, bool enabled);
    static void recolorTopLabel(lv_obj_t *obj, bool alert);
    static void recolorTableRow(lv_draw_fill_dsc_t *fill_draw_dsc, bool odd);

  private:
    Themes(void) = default;
};
