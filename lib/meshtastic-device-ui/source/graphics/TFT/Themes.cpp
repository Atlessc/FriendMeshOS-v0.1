#if defined(VIEW_320x240) || defined(VIEW_240x320)

#include "graphics/view/TFT/Themes.h"
#include "stdint.h"

static enum Themes::Theme theme = Themes::eCleanModern;

struct SemanticPalette {
    uint32_t accent;
    uint32_t muted;
    uint32_t focus;
    uint32_t disabled;
    uint32_t warning;
    uint32_t error;
    uint32_t sos;
};

struct ThemeGeometry {
    int32_t radius;
    int32_t borderWidth;
    int32_t shadowWidth;
};

static constexpr SemanticPalette semanticPalette[Themes::eThemeCount] = {
    {0xff22d3ee, 0xff64748b, 0xff67e8f9, 0xff475569, 0xfff59e0b, 0xfff43f5e, 0xffef4444}, // Clean Modern
    {0xff9cff57, 0xff486b3c, 0xffd6ff8a, 0xff3c4a38, 0xffffd166, 0xffff5c5c, 0xffff3b3b}, // Retro Terminal
    {0xffffd60a, 0xff5c5c5c, 0xff0066ff, 0xffa0a0a0, 0xfff59e0b, 0xffdc2626, 0xffdc2626}, // Neobrutalist
    {0xffff5a36, 0xff5b6977, 0xffffb347, 0xff3d4650, 0xffff9f1c, 0xffff3b30, 0xffff2d2d}, // Orbital Mission
    {0xff147d78, 0xff78958c, 0xffe76f2e, 0xffb4c1bb, 0xffdd6b20, 0xffc53030, 0xffc53030}, // Alpine Daylight
    {0xff8b5cf6, 0xff7c83b6, 0xff38bdf8, 0xff52577a, 0xfffb923c, 0xfffb7185, 0xfff43f5e}, // Friendly Mesh
};

static constexpr ThemeGeometry themeGeometry[Themes::eThemeCount] = {
    {10, 2, 2}, // Clean Modern
    {0, 1, 0},  // Retro Terminal
    {0, 3, 5},  // Neobrutalist
    {3, 2, 1},  // Orbital Mission
    {6, 1, 1},  // Alpine Daylight
    {12, 2, 2}, // Friendly Mesh
};

Themes::Theme Themes::get(void)
{
    return theme;
}

Themes::Theme Themes::normalize(uint32_t value)
{
    return value < eThemeCount ? static_cast<Theme>(value) : eCleanModern;
}

const char *Themes::name(enum Theme value)
{
    static constexpr const char *names[eThemeCount] = {
        "Clean Modern Field Tool", "Retro Handheld Terminal", "Bold Neobrutalist Utility",
        "Orbital Mission Control", "Alpine Daylight Navigator", "Friendly Mesh Constellation",
    };
    return names[normalize(static_cast<uint32_t>(value))];
}

enum ThemeColor {
    eMainScreenStyle,
    eTopPanelBg,
    eTopPanelText,
    eTopImageBg,
    eTopImageRecolor,
    eTopImageRecolorOpa,
    ePositiveImageRecolor,
    ePanelBg,
    ePanelPressedBg,
    ePanelText,
    ePanelBorder,
    eNodePanelBg,
    eNodePanelBorder,
    eNodePanelText,
    eNodeButtonBg,
    eNodeButtonBgOpa,
    eButtonPanelBg,
    eMainButtonBg,
    eMainButtonText,
    eMainButtonBorder,
    eMainButtonShadow,
    eMainButtonImageRecolor,
    eMainButtonImageRecolorOpa,
    eHomeContainerBg,
    eHomeContainerBorder,
    eHomeContainerShadow,
    eHomeContainerText,
    eHomeButtonBg,
    eHomeButtonText,
    eHomeButtonBorder,
    eHomeButtonImageRecolor,
    eHomeButtonImageRecolorOpa,
    eChannelButtonBg,
    eChannelButtonBorder,
    eChannelButtonText,
    eSettingsPanelBg,
    eSettingsPanelText,
    eSettingsPanelBorder,
    eSettingsPanelShadow,
    eSettingsPanelBgOpa,
    eSettingsButtonBg,
    eSettingsButtonText,
    eSettingsButtonBorder,
    eSettingsButtonImageRecolor,
    eSettingsButtonImageRecolorOpa,
    eSettingsLabelBg,
    eSettingsLabelBorder,
    eTabViewBg,
    eTabViewText,
    eTabButtonDefaultBg,
    eTabButtonActiveBg,
    eTabButtonPressedBg,
    eTabButtonDefaultText,
    eTabButtonActiveText,
    eTabButtonPressedText,
    eTabButtonDefaultBorder,
    eChatMessageBg,
    eChatMessageBgOpa,
    eChatMessageText,
    eChatMessageBorder,
    eNewMessageBg,
    eNewMessageBgOpa,
    eNewMessageText,
    eNewMessageBorder,
    eAlertPanelBg,
    eBtnMatrixBorderMain,
    eBtnMatrixBorderItems,
    eBtnMatrixBgItems,
    eBtnMatrixTextItems,
    eBatteryPercentageText,
    eColorTextLabel,
    eSpinnerMainArc,
    eSpinnerIndicatorArc,
    eTableHeadingText,
    eTableHeadingBg,
    eTableItemText,
    eTableItemBg,
    eTableItemDarkBg,
    eTableBorder,
    eTableCellBorder
};

static constexpr uint32_t themeColor[][Themes::eThemeCount] = {
    // clean modern, retro, neobrutalist, orbital, alpine, friendly
    {0xff07111f, 0xff071008, 0xfff5f1e8, 0xff020b16, 0xfff6f1df, 0xff11143b}, // eMainScreenStyle
    {0xff0b2239, 0xff0b1d0d, 0xffffd60a, 0xff071525, 0xffe8e2cf, 0xff1b1f4d}, // eTopPanelBg
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eTopPanelText
    {0xff0b2239, 0xff0b1d0d, 0xffffd60a, 0xff071525, 0xffe8e2cf, 0xff1b1f4d}, // eTopImageBg
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff38bdf8}, // eTopImageRecolor
    {255, 255, 255, 255, 255, 255}, // eTopImageRecolorOpa
    {0xff22c55e, 0xff9cff57, 0xff16a34a, 0xff22c55e, 0xff2f855a, 0xff2dd4bf}, // ePositiveImageRecolor
    {0xff0f172a, 0xff0d1b10, 0xffffffff, 0xff071525, 0xfffffaf0, 0xff1b1f4d}, // ePanelBg
    {0xff172554, 0xff18351c, 0xffffd60a, 0xff1a3045, 0xffdce9df, 0xff312e81}, // ePanelPressedBg
    {0xffe2e8f0, 0xff91cf61, 0xff202020, 0xffc3b79f, 0xff426b64, 0xffd8d1ff}, // ePanelText
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff8b5cf6}, // ePanelBorder
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eNodePanelBg
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eNodePanelBorder
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eNodePanelText
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eNodeButtonBg
    {0, 0, 0, 0, 0, 0}, // eNodeButtonBgOpa
    {0xff07111f, 0xff071008, 0xfff5f1e8, 0xff020b16, 0xfff6f1df, 0xff11143b}, // eButtonPanelBg
    {0xff111827, 0xff0d1b10, 0xffffd60a, 0xff0b1d2e, 0xfffffaf0, 0xff312e81}, // eMainButtonBg
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eMainButtonText
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff8b5cf6}, // eMainButtonBorder
    {0xff020617, 0xff020a04, 0xff0a0a0a, 0xff000000, 0xffc5bfae, 0xff080a25}, // eMainButtonShadow
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff38bdf8}, // eMainButtonImageRecolor
    {255, 255, 255, 255, 255, 255}, // eMainButtonImageRecolorOpa
    {0xff07111f, 0xff071008, 0xfff5f1e8, 0xff020b16, 0xfff6f1df, 0xff11143b}, // eHomeContainerBg
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff8b5cf6}, // eHomeContainerBorder
    {0xff020617, 0xff020a04, 0xff0a0a0a, 0xff000000, 0xffc5bfae, 0xff080a25}, // eHomeContainerShadow
    {0xffe2e8f0, 0xff91cf61, 0xff202020, 0xffc3b79f, 0xff426b64, 0xffd8d1ff}, // eHomeContainerText
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eHomeButtonBg
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eHomeButtonText
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eHomeButtonBorder
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff38bdf8}, // eHomeButtonImageRecolor
    {255, 255, 255, 255, 255, 255}, // eHomeButtonImageRecolorOpa
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eChannelButtonBg
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eChannelButtonBorder
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eChannelButtonText
    {0xff0f172a, 0xff0d1b10, 0xffffffff, 0xff071525, 0xfffffaf0, 0xff1b1f4d}, // eSettingsPanelBg
    {0xffe2e8f0, 0xff91cf61, 0xff202020, 0xffc3b79f, 0xff426b64, 0xffd8d1ff}, // eSettingsPanelText
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff8b5cf6}, // eSettingsPanelBorder
    {0xff020617, 0xff020a04, 0xff0a0a0a, 0xff000000, 0xffc5bfae, 0xff080a25}, // eSettingsPanelShadow
    {250, 250, 250, 250, 250, 250}, // eSettingsPanelBgOpa
    {0xff111827, 0xff0d1b10, 0xffffd60a, 0xff0b1d2e, 0xfffffaf0, 0xff312e81}, // eSettingsButtonBg
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eSettingsButtonText
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eSettingsButtonBorder
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff38bdf8}, // eSettingsButtonImageRecolor
    {255, 255, 255, 255, 255, 255}, // eSettingsButtonImageRecolorOpa
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eSettingsLabelBg
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eSettingsLabelBorder
    {0xff07111f, 0xff071008, 0xfff5f1e8, 0xff020b16, 0xfff6f1df, 0xff11143b}, // eTabViewBg
    {0xffe2e8f0, 0xff91cf61, 0xff202020, 0xffc3b79f, 0xff426b64, 0xffd8d1ff}, // eTabViewText
    {0xff0f172a, 0xff0d1b10, 0xffffffff, 0xff071525, 0xfffffaf0, 0xff1b1f4d}, // eTabButtonDefaultBg
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eTabButtonActiveBg
    {0xff164e63, 0xff1f3d18, 0xff0066ff, 0xff3a1d18, 0xffdce9df, 0xff3730a3}, // eTabButtonPressedBg
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eTabButtonDefaultText
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eTabButtonActiveText
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eTabButtonPressedText
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eTabButtonDefaultBorder
    {0xff0f172a, 0xff0d1b10, 0xffffffff, 0xff071525, 0xfffffaf0, 0xff1b1f4d}, // eChatMessageBg
    {255, 255, 255, 255, 255, 255}, // eChatMessageBgOpa
    {0xffe2e8f0, 0xff91cf61, 0xff202020, 0xffc3b79f, 0xff426b64, 0xffd8d1ff}, // eChatMessageText
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eChatMessageBorder
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eNewMessageBg
    {255, 255, 255, 255, 255, 255}, // eNewMessageBgOpa
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eNewMessageText
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff8b5cf6}, // eNewMessageBorder
    {0xff450a0a, 0xff3a1010, 0xfffee2e2, 0xff3a0d0d, 0xfffde8e8, 0xff4c1630}, // eAlertPanelBg
    {0xff07111f, 0xff071008, 0xfff5f1e8, 0xff020b16, 0xfff6f1df, 0xff11143b}, // eBtnMatrixBorderMain
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff8b5cf6}, // eBtnMatrixBorderItems
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eBtnMatrixBgItems
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eBtnMatrixTextItems
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eBatteryPercentageText
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff38bdf8}, // eColorTextLabel
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eSpinnerMainArc
    {0xff22d3ee, 0xff9cff57, 0xff0a0a0a, 0xffff5a36, 0xff147d78, 0xff38bdf8}, // eSpinnerIndicatorArc
    {0xfff8fafc, 0xffb6ff63, 0xff0a0a0a, 0xfff4e6c8, 0xff123b3a, 0xfffff7ed}, // eTableHeadingText
    {0xff0b2239, 0xff0b1d0d, 0xffffd60a, 0xff071525, 0xffe8e2cf, 0xff1b1f4d}, // eTableHeadingBg
    {0xffe2e8f0, 0xff91cf61, 0xff202020, 0xffc3b79f, 0xff426b64, 0xffd8d1ff}, // eTableItemText
    {0xff111827, 0xff0a150c, 0xffffffff, 0xff0b1d2e, 0xfff4efdf, 0xff24285e}, // eTableItemBg
    {0xff0f172a, 0xff08120a, 0xffe8e3d8, 0xff06111e, 0xffebe5d4, 0xff171a46}, // eTableItemDarkBg
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eTableBorder
    {0xff64748b, 0xff486b3c, 0xff5c5c5c, 0xff5b6977, 0xff78958c, 0xff7c83b6}, // eTableCellBorder
};

#include "fonts.h"
#include "images.h"
#include "styles.h"

#define THEME(COLOR) (themeColor[COLOR][theme])
#define SEMANTIC(COLOR) (semanticPalette[theme].COLOR)
#define GEOMETRY(VALUE) (themeGeometry[theme].VALUE)

lv_color_t Themes::accentColor(void)
{
    return lv_color_hex(SEMANTIC(accent));
}

lv_color_t Themes::mutedColor(void)
{
    return lv_color_hex(SEMANTIC(muted));
}

lv_color_t Themes::focusColor(void)
{
    return lv_color_hex(SEMANTIC(focus));
}

lv_color_t Themes::disabledColor(void)
{
    return lv_color_hex(SEMANTIC(disabled));
}

lv_color_t Themes::warningColor(void)
{
    return lv_color_hex(SEMANTIC(warning));
}

lv_color_t Themes::errorColor(void)
{
    return lv_color_hex(SEMANTIC(error));
}

lv_color_t Themes::sosColor(void)
{
    return lv_color_hex(SEMANTIC(sos));
}

// the following styles are copied from eez-studio generated styles and parametrized
extern "C" {
void apply_style_top_panel_style(void)
{
    lv_style_t *style = get_style_top_panel_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eTopPanelBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eTopPanelText)));
    // lv_style_set_text_font(style, &ui_font_montserrat_16);
};
void apply_style_panel_style_MAIN_DEFAULT(void)
{
    lv_style_t *style = get_style_panel_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(ePanelBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(ePanelText)));
    lv_style_set_border_color(style, lv_color_hex(THEME(ePanelBorder)));
    lv_style_set_border_width(style, GEOMETRY(borderWidth));
    lv_style_set_radius(style, GEOMETRY(radius));
    // lv_style_set_shadow_color(style, lv_color_hex(0xffe0e0e0));
};
void apply_style_panel_style_MAIN_PRESSED(void)
{
    lv_style_t *style = get_style_panel_style_MAIN_PRESSED();
    lv_style_set_bg_color(style, lv_color_hex(THEME(ePanelPressedBg)));
};
void apply_style_home_container_style(void)
{
    lv_style_t *style = get_style_home_container_style_MAIN_DEFAULT();
    lv_style_set_border_color(style, lv_color_hex(THEME(eHomeContainerBorder)));
    lv_style_set_border_width(style, GEOMETRY(borderWidth));
    lv_style_set_border_side(style, LV_BORDER_SIDE_FULL);
    lv_style_set_bg_color(style, lv_color_hex(THEME(eHomeContainerBg)));
    lv_style_set_shadow_color(style, lv_color_hex(THEME(eHomeContainerShadow)));
    lv_style_set_text_font(style, &ui_font_montserrat_16);
    lv_style_set_radius(style, GEOMETRY(radius));
    lv_style_set_text_color(style, lv_color_hex(THEME(eHomeContainerText)));
};
void apply_style_settings_panel_style(void)
{
    lv_style_t *style = get_style_settings_panel_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eSettingsPanelBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eSettingsPanelText)));
    lv_style_set_shadow_color(style, lv_color_hex(THEME(eSettingsPanelShadow)));
    lv_style_set_border_color(style, lv_color_hex(THEME(eSettingsPanelBorder)));
    lv_style_set_border_width(style, GEOMETRY(borderWidth));
    lv_style_set_radius(style, GEOMETRY(radius));
    lv_style_set_bg_opa(style, THEME(eSettingsPanelBgOpa));
};
void apply_style_node_panel_style(void)
{
    lv_style_t *style = get_style_node_panel_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eNodePanelBg)));
    lv_style_set_border_color(style, lv_color_hex(THEME(eNodePanelBorder)));
    lv_style_set_text_font(style, &ui_font_montserrat_12);
    lv_style_set_text_color(style, lv_color_hex(THEME(eNodePanelText)));
    lv_style_set_border_width(style, GEOMETRY(borderWidth));
    lv_style_set_radius(style, GEOMETRY(radius));
};
void apply_style_node_button_style(void)
{
    lv_style_t *style = get_style_node_button_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eNodeButtonBg)));
    lv_style_set_bg_opa(style, THEME(eNodeButtonBgOpa));
};
void apply_style_button_panel_style(void)
{
    lv_style_t *style = get_style_button_panel_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eButtonPanelBg)));
};
void apply_style_home_button_style(void)
{
    lv_style_t *style = get_style_home_button_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eHomeButtonBg)));
    lv_style_set_bg_image_recolor_opa(style, THEME(eHomeButtonImageRecolorOpa));
    lv_style_set_bg_image_recolor(style, lv_color_hex(THEME(eHomeButtonImageRecolor)));
    lv_style_set_border_color(style, lv_color_hex(THEME(eHomeButtonBorder)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eHomeButtonText)));
    lv_style_set_border_width(style, GEOMETRY(borderWidth));
    lv_style_set_radius(style, GEOMETRY(radius));
};
void apply_style_settings_button_style(void)
{
    lv_style_t *style = get_style_settings_button_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eSettingsButtonBg)));
    lv_style_set_bg_image_recolor_opa(style, THEME(eSettingsButtonImageRecolorOpa));
    lv_style_set_bg_image_recolor(style, lv_color_hex(THEME(eSettingsButtonImageRecolor)));
    lv_style_set_border_color(style, lv_color_hex(THEME(eSettingsButtonBorder)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eSettingsButtonText)));
    lv_style_set_border_width(style, GEOMETRY(borderWidth));
    lv_style_set_radius(style, GEOMETRY(radius));
};
void apply_style_main_button_style(void)
{
    lv_style_t *style = get_style_main_button_style_MAIN_DEFAULT();
    lv_style_set_bg_image_recolor_opa(style, THEME(eMainButtonImageRecolorOpa));
    lv_style_set_bg_image_recolor(style, lv_color_hex(THEME(eMainButtonImageRecolor)));
    lv_style_set_border_color(style, lv_color_hex(THEME(eMainButtonBorder)));
    lv_style_set_bg_color(style, lv_color_hex(THEME(eMainButtonBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eMainButtonText)));
    lv_style_set_shadow_color(style, lv_color_hex(THEME(eMainButtonShadow)));
    lv_style_set_shadow_width(style, GEOMETRY(shadowWidth));
    lv_style_set_border_width(style, GEOMETRY(borderWidth));
    lv_style_set_radius(style, GEOMETRY(radius));
};
void apply_style_new_message_style(void)
{
    lv_style_t *style = get_style_new_message_style_MAIN_DEFAULT();
    lv_style_set_border_color(style, lv_color_hex(THEME(eNewMessageBorder)));
    lv_style_set_bg_color(style, lv_color_hex(THEME(eNewMessageBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eNewMessageText)));
    lv_style_set_bg_opa(style, THEME(eNewMessageBgOpa));
};
void apply_style_chat_message_style(void)
{
    lv_style_t *style = get_style_chat_message_style_MAIN_DEFAULT();
    lv_style_set_border_color(style, lv_color_hex(THEME(eChatMessageBorder)));
    lv_style_set_bg_color(style, lv_color_hex(THEME(eChatMessageBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eChatMessageText)));
    lv_style_set_bg_opa(style, THEME(eChatMessageBgOpa));
};
void apply_style_tab_view_style(void)
{
    lv_style_t *style = get_style_tab_view_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eTabViewBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eTabViewText)));
};
void apply_style_drop_down_style(void){};
void apply_style_bw_label_style(void)
{
    lv_style_t *style = get_style_bw_label_style_MAIN_DEFAULT();
    lv_style_set_text_color(style, lv_color_hex(THEME(eBatteryPercentageText)));
};
void apply_style_color_label_style(void)
{
    lv_style_t *style = get_style_color_label_style_MAIN_DEFAULT();
    lv_style_set_text_color(style, lv_color_hex(THEME(eColorTextLabel)));
};
void apply_style_top_image_style(void)
{
    lv_style_t *style = get_style_top_image_style_MAIN_DEFAULT();
    lv_style_set_bg_image_recolor(style, lv_color_hex(THEME(eTopImageRecolor)));
    lv_style_set_bg_image_recolor_opa(style, THEME(eTopImageRecolorOpa));
    lv_style_set_image_recolor(style, lv_color_hex(THEME(eTopImageRecolor)));
    lv_style_set_image_recolor_opa(style, THEME(eTopImageRecolorOpa));
    lv_style_set_bg_color(style, lv_color_hex(THEME(eTopImageBg)));
};
void apply_style_alert_panel_style(void)
{
    lv_style_t *style = get_style_alert_panel_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eAlertPanelBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(ePanelText)));
};
void apply_style_main_screen_style(void)
{
    lv_style_t *style = get_style_main_screen_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eMainScreenStyle)));
};
void apply_style_channel_button_style(void)
{
    lv_style_t *style = get_style_channel_button_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eChannelButtonBg)));
    lv_style_set_border_color(style, lv_color_hex(THEME(eChannelButtonBorder)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eChannelButtonText)));
    lv_style_set_border_width(style, GEOMETRY(borderWidth));
    lv_style_set_radius(style, GEOMETRY(radius));
};
void apply_style_button_matrix_style_ITEMS_DEFAULT(void)
{
    lv_style_t *style = get_style_button_matrix_style_ITEMS_DEFAULT();
    lv_style_set_border_color(style, lv_color_hex(THEME(eBtnMatrixBorderItems)));
    lv_style_set_bg_color(style, lv_color_hex(THEME(eBtnMatrixBgItems)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eBtnMatrixTextItems)));
};
void apply_style_button_matrix_style_MAIN_DEFAULT(void)
{
    lv_style_t *style = get_style_button_matrix_style_MAIN_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eBtnMatrixBorderMain)));
};
void apply_style_spinner_style_MAIN_DEFAULT(void)
{
    lv_style_t *style = get_style_spinner_style_MAIN_DEFAULT();
    lv_style_set_arc_color(style, lv_color_hex(THEME(eSpinnerMainArc)));
};
void apply_style_spinner_style_INDICATOR_DEFAULT(void)
{
    lv_style_t *style = get_style_spinner_style_INDICATOR_DEFAULT();
    lv_style_set_arc_color(style, lv_color_hex(THEME(eSpinnerIndicatorArc)));
};
void apply_style_settings_label_style(void)
{
    lv_style_t *style = get_style_settings_label_style_MAIN_DEFAULT();
    lv_style_set_border_color(style, lv_color_hex(THEME(eSettingsLabelBorder)));
    // lv_style_set_bg_opa(style, 255);
    lv_style_set_bg_color(style, lv_color_hex(THEME(eSettingsLabelBg)));
};
void apply_style_positive_image_style(void)
{
    lv_style_t *style = get_style_positive_image_style_MAIN_DEFAULT();
    lv_style_set_image_recolor(style, lv_color_hex(THEME(ePositiveImageRecolor)));
};
void apply_style_statistics_table_style_MAIN_DEFAULT(void)
{
    lv_style_t *style = get_style_statistics_table_style_MAIN_DEFAULT();
    lv_style_set_border_color(style, lv_color_hex(THEME(eTableBorder)));
};
void apply_style_statistics_table_style_ITEMS_DEFAULT(void)
{
    lv_style_t *style = get_style_statistics_table_style_ITEMS_DEFAULT();
    lv_style_set_bg_color(style, lv_color_hex(THEME(eTableItemBg)));
    lv_style_set_text_color(style, lv_color_hex(THEME(eTableItemText)));
    lv_style_set_border_color(style, lv_color_hex(THEME(eTableCellBorder)));
};
}

void Themes::set(enum Theme th)
{
    theme = normalize(static_cast<uint32_t>(th));
    apply_style_top_panel_style();
    apply_style_panel_style_MAIN_DEFAULT();
    apply_style_panel_style_MAIN_PRESSED();
    apply_style_home_container_style();
    apply_style_settings_panel_style();
    apply_style_node_panel_style();
    apply_style_node_button_style();
    apply_style_button_panel_style();
    apply_style_home_button_style();
    apply_style_settings_button_style();
    apply_style_main_button_style();
    apply_style_new_message_style();
    apply_style_chat_message_style();
    apply_style_tab_view_style();
    apply_style_drop_down_style();
    apply_style_bw_label_style();
    apply_style_color_label_style();
    apply_style_top_image_style();
    apply_style_alert_panel_style();
    apply_style_main_screen_style();
    apply_style_channel_button_style();
    apply_style_button_matrix_style_ITEMS_DEFAULT();
    apply_style_button_matrix_style_MAIN_DEFAULT();
    apply_style_spinner_style_MAIN_DEFAULT();
    apply_style_spinner_style_INDICATOR_DEFAULT();
    apply_style_settings_label_style();
    apply_style_positive_image_style();
    apply_style_statistics_table_style_MAIN_DEFAULT();
    apply_style_statistics_table_style_ITEMS_DEFAULT();
}

void Themes::initStyles(void)
{
    // set(get());
    //  lvgl v9 tabview buttons are not btn-matrix anymore but array of buttons
    //  see https://forum.lvgl.io/t/style-a-tabview-widget-in-v9-0-0/14747
    lv_style_init(&style_btn_default);
    lv_style_set_text_color(&style_btn_default, lv_color_hex(THEME(eTabButtonDefaultText)));
    lv_style_set_bg_color(&style_btn_default, lv_color_hex(THEME(eTabButtonDefaultBg)));
    lv_style_set_bg_opa(&style_btn_default, LV_OPA_COVER);
    lv_style_set_border_color(&style_btn_default, lv_color_hex(THEME(eTabButtonDefaultBorder)));
    lv_style_set_border_opa(&style_btn_default, LV_OPA_COVER);
    lv_style_set_border_width(&style_btn_default, 1);
    lv_style_set_border_side(&style_btn_default, LV_BORDER_SIDE_FULL);

    lv_style_init(&style_btn_active);
    lv_style_set_text_color(&style_btn_active, lv_color_hex(THEME(eTabButtonActiveText)));
    lv_style_set_bg_color(&style_btn_active, lv_color_hex(THEME(eTabButtonActiveBg)));
    lv_style_set_bg_opa(&style_btn_active, LV_OPA_COVER);
    lv_style_set_border_color(
    &style_btn_active,
    lv_color_hex(THEME(ePanelBorder))
);
    lv_style_set_border_opa(&style_btn_active, LV_OPA_COVER);
    lv_style_set_border_width(&style_btn_active, 3);
    lv_style_set_border_side(&style_btn_active, LV_BORDER_SIDE_BOTTOM);

    lv_style_init(&style_btn_pressed);
    lv_style_set_text_color(&style_btn_pressed, lv_color_hex(THEME(eTabButtonPressedText)));
    lv_style_set_bg_color(&style_btn_pressed, lv_color_hex(THEME(eTabButtonPressedBg)));
    lv_style_set_bg_opa(&style_btn_pressed, LV_OPA_COVER);
    lv_style_set_border_color(
    &style_btn_pressed,
    lv_color_hex(THEME(ePanelBorder))
);
    lv_style_set_border_opa(&style_btn_pressed, LV_OPA_COVER);
    lv_style_set_border_width(&style_btn_pressed, 3);
    lv_style_set_border_side(&style_btn_pressed, LV_BORDER_SIDE_BOTTOM);
}

void Themes::recolorButton(lv_obj_t *obj, bool enabled, lv_opa_t opa)
{
    const lv_color_t color = enabled ? accentColor() : disabledColor();

    lv_obj_set_style_bg_image_recolor(
        obj,
        color,
        LV_PART_MAIN | LV_STATE_DEFAULT
    );

    lv_obj_set_style_bg_image_recolor_opa(
        obj,
        opa,
        LV_PART_MAIN | LV_STATE_DEFAULT
    );
}

void Themes::recolorImage(lv_obj_t *obj, bool enabled)
{
    const lv_color_t color = enabled ? accentColor() : disabledColor();

    lv_obj_set_style_image_recolor(
        obj,
        color,
        LV_PART_MAIN | LV_STATE_DEFAULT
    );
}

void Themes::recolorText(lv_obj_t *obj, bool enabled)
{
    const lv_color_t color = enabled ? accentColor() : mutedColor();

    lv_obj_set_style_text_color(
        obj,
        color,
        LV_PART_MAIN | LV_STATE_DEFAULT
    );
}

void Themes::recolorTopLabel(lv_obj_t *obj, bool alert)
{
    const lv_color_t color = alert ? errorColor() : lv_color_hex(THEME(eTopPanelText));

    lv_obj_set_style_text_color(
        obj,
        color,
        LV_PART_MAIN | LV_STATE_DEFAULT
    );
}

void Themes::recolorTableRow(lv_draw_fill_dsc_t *fill_draw_dsc, bool odd)
{
    if (odd) {
        fill_draw_dsc->color = lv_color_hex(THEME(eTableItemBg));
    } else {
        fill_draw_dsc->color = lv_color_hex(THEME(eTableItemDarkBg));
    }
}

#endif // VIEW_320x240
