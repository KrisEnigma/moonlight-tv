#if defined(TARGET_WEBOS)

#include "hid_passthrough_panel.h"

#include "ctm/ctm_state.h"
#include "hid_passthrough/hid_passthrough_manager.h"
#include "stream/session.h"

#include "util/i18n.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HID_PT_MAX_ROWS 64

typedef struct {
    lv_obj_t *container;
    lv_obj_t *status_label;
    lv_obj_t *list;
    lv_group_t *group;
    session_t *session;
    hid_passthrough_panel_close_cb on_close;
    void *on_close_userdata;
    lv_timer_t *refresh_timer;
    lv_obj_t *row_buttons[HID_PT_MAX_ROWS];
    lv_obj_t *plug_labels[HID_PT_MAX_ROWS];
    int selected_index;
    char selected_key[96];
    int list_width;
    uint64_t last_sig;
    bool have_rendered;
} hid_pt_panel_t;

static void style_row(lv_obj_t *row, bool selected) {
    lv_obj_set_style_radius(row, LV_DPX(8), 0);
    lv_obj_set_style_border_width(row, selected ? 2 : 1, 0);
    lv_obj_set_style_border_color(row, selected ? lv_color_hex(0x38bdf8) : lv_color_hex(0x263540), 0);
    lv_obj_set_style_bg_color(row, selected ? lv_color_hex(0x12384a) : lv_color_hex(0x18232c), 0);
    lv_obj_set_style_bg_color(row, selected ? lv_color_hex(0x16455d) : lv_color_hex(0x202e38), LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
}

static void update_row_styles(hid_pt_panel_t *panel) {
    for (int i = 0; i < g_devices.count && i < HID_PT_MAX_ROWS; ++i) {
        if (panel->row_buttons[i]) {
            style_row(panel->row_buttons[i], i == panel->selected_index);
        }
    }
}

static void panel_close(hid_pt_panel_t *panel) {
    if (!panel) {
        return;
    }
    if (panel->on_close) {
        panel->on_close(panel->on_close_userdata);
    }
}

static void back_btn_cb(lv_event_t *e) {
    panel_close(lv_event_get_user_data(e));
}

static void row_clicked_cb(lv_event_t *event) {
    hid_pt_panel_t *panel = lv_event_get_user_data(event);
    int index = (int) (intptr_t) lv_obj_get_user_data(lv_event_get_current_target(event));
    if (!panel || index < 0 || index >= g_devices.count) {
        return;
    }
    panel->selected_index = index;
    snprintf(panel->selected_key, sizeof(panel->selected_key), "%s", g_devices.items[index].key);
    update_row_styles(panel);
}

static void plug_button_cb(lv_event_t *event) {
    hid_pt_panel_t *panel = lv_event_get_user_data(event);
    int index = (int) (intptr_t) lv_obj_get_user_data(lv_event_get_current_target(event));
    if (!panel || index < 0 || index >= g_devices.count) {
        return;
    }

    logical_device_t *item = &g_devices.items[index];
    bool requested_state = !item->plugged;
    if (requested_state) {
        if (!plug_in_item(item)) {
            hid_passthrough_panel_refresh(panel->container);
            return;
        }
    } else {
        stop_session(item->key);
    }

    item->plugged = requested_state;
    set_plug_key(item->key, item->plugged);
    panel->selected_index = index;
    snprintf(panel->selected_key, sizeof(panel->selected_key), "%s", item->key);

    if (panel->plug_labels[index]) {
        lv_label_set_text(panel->plug_labels[index], item->plugged ? locstr("Plug out") : locstr("Plug in"));
    }
    update_row_styles(panel);
    hid_passthrough_panel_refresh(panel->container);
}

static void render_device_list(hid_pt_panel_t *panel) {
    lv_obj_clean(panel->list);
    memset(panel->row_buttons, 0, sizeof(panel->row_buttons));
    memset(panel->plug_labels, 0, sizeof(panel->plug_labels));

    if (g_devices.count == 0) {
        lv_obj_t *empty = lv_label_create(panel->list);
        lv_label_set_text(empty, locstr("No HID devices visible to the native app"));
        lv_obj_set_style_text_color(empty, lv_color_hex(0xb6c5cf), 0);
        lv_obj_set_style_pad_all(empty, LV_DPX(18), 0);
        return;
    }

    const int row_h = LV_DPX(60);
    const int gap = LV_DPX(8);
    const int row_w = panel->list_width > 0 ? panel->list_width : LV_PCT(100);
    const int button_w = LV_DPX(112);
    int y = 0;

    for (int i = 0; i < g_devices.count && i < HID_PT_MAX_ROWS; ++i) {
        const logical_device_t *item = &g_devices.items[i];
        lv_obj_t *row = lv_obj_create(panel->list);
        panel->row_buttons[i] = row;
        lv_obj_set_pos(row, 0, y);
        lv_obj_set_size(row, row_w, row_h);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(row, row_clicked_cb, LV_EVENT_CLICKED, panel);
        lv_obj_set_user_data(row, (void *) (intptr_t) i);
        style_row(row, i == panel->selected_index);
        lv_group_add_obj(panel->group, row);

        lv_obj_t *name = lv_label_create(row);
        lv_label_set_text(name, item->name);
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
        lv_obj_set_width(name, row_w - button_w - LV_DPX(30));
        lv_obj_set_pos(name, LV_DPX(16), LV_DPX(20));

        lv_obj_t *plug_btn = lv_btn_create(row);
        lv_obj_set_size(plug_btn, button_w, LV_DPX(38));
        lv_obj_align(plug_btn, LV_ALIGN_RIGHT_MID, LV_DPX(-14), 0);
        lv_obj_set_style_radius(plug_btn, LV_DPX(6), 0);
        lv_obj_set_style_bg_color(plug_btn, item->plugged ? lv_color_hex(0x7f1d1d) : lv_color_hex(0x0f766e), 0);
        lv_obj_set_style_bg_color(plug_btn, item->plugged ? lv_color_hex(0x991b1b) : lv_color_hex(0x0d9488),
                                  LV_STATE_PRESSED);
        lv_obj_add_event_cb(plug_btn, plug_button_cb, LV_EVENT_CLICKED, panel);
        lv_obj_set_user_data(plug_btn, (void *) (intptr_t) i);
        lv_group_add_obj(panel->group, plug_btn);

        panel->plug_labels[i] = lv_label_create(plug_btn);
        lv_label_set_text(panel->plug_labels[i], item->plugged ? locstr("Plug out") : locstr("Plug in"));
        lv_obj_center(panel->plug_labels[i]);

        y += row_h + gap;
    }
}

static void update_status_label(hid_pt_panel_t *panel) {
    if (!panel->status_label) {
        return;
    }
    char status[128];
    snprintf(status, sizeof(status), "%d device%s | Windows %s",
             g_devices.count,
             g_devices.count == 1 ? "" : "s",
             g_agent_online ? g_agent_host : "not found");
    lv_label_set_text(panel->status_label, status);
}

static uint64_t device_list_signature(void) {
    uint64_t sig = 1469598103934665603ULL;
#define SIG_MIX(p, n) do {                                              \
        const unsigned char *_b = (const unsigned char *) (p);           \
        for (size_t _i = 0; _i < (size_t) (n); ++_i) {                  \
            sig ^= _b[_i];                                              \
            sig *= 1099511628211ULL;                                    \
        }                                                               \
    } while (0)
    SIG_MIX(&g_devices.count, sizeof(g_devices.count));
    for (int i = 0; i < g_devices.count; ++i) {
        const logical_device_t *item = &g_devices.items[i];
        unsigned char st = (unsigned char) (item->plugged ? 1 : 0);
        SIG_MIX(item->key, strlen(item->key));
        SIG_MIX(&st, 1);
    }
#undef SIG_MIX
    return sig;
}

static void refresh_devices(hid_pt_panel_t *panel) {
    if (!panel || !panel->session) {
        return;
    }
    session_ensure_hid_passthrough(panel->session);
    hid_passthrough_manager_t *mgr = session_get_hid_passthrough(panel->session);
    if (mgr) {
        hid_passthrough_manager_rescan(mgr);
    }

    int selected = -1;
    if (panel->selected_key[0]) {
        for (int i = 0; i < g_devices.count; ++i) {
            if (strcmp(g_devices.items[i].key, panel->selected_key) == 0) {
                selected = i;
                break;
            }
        }
    }
    if (selected < 0 && g_devices.count > 0) {
        selected = 0;
        snprintf(panel->selected_key, sizeof(panel->selected_key), "%s", g_devices.items[0].key);
    }
    panel->selected_index = selected;
    update_status_label(panel);

    uint64_t sig = device_list_signature();
    if (!panel->have_rendered || sig != panel->last_sig) {
        panel->last_sig = sig;
        panel->have_rendered = true;
        render_device_list(panel);
    } else {
        update_row_styles(panel);
    }
}

static void refresh_timer_cb(lv_timer_t *timer) {
    hid_pt_panel_t *panel = timer->user_data;
    if (panel && panel->container && lv_obj_is_valid(panel->container)) {
        refresh_devices(panel);
    }
}

static void refresh_button_cb(lv_event_t *event) {
    hid_pt_panel_t *panel = lv_event_get_user_data(event);
    refresh_devices(panel);
}

static void panel_delete_cb(lv_event_t *e) {
    hid_pt_panel_t *panel = lv_event_get_user_data(e);
    if (panel->refresh_timer) {
        lv_timer_del(panel->refresh_timer);
        panel->refresh_timer = NULL;
    }
    if (panel->group) {
        lv_group_del(panel->group);
    }
    free(panel);
}

void hid_passthrough_panel_refresh(lv_obj_t *panel_root) {
    hid_pt_panel_t *panel = lv_obj_get_user_data(panel_root);
    if (panel) {
        refresh_devices(panel);
    }
}

lv_group_t *hid_passthrough_panel_get_group(lv_obj_t *panel_root) {
    hid_pt_panel_t *panel = lv_obj_get_user_data(panel_root);
    return panel ? panel->group : NULL;
}

lv_obj_t *hid_passthrough_panel_create(lv_obj_t *parent, session_t *session,
                                       hid_passthrough_panel_close_cb on_close, void *userdata) {
    hid_pt_panel_t *panel = calloc(1, sizeof(*panel));
    if (!panel) {
        return NULL;
    }
    panel->session = session;
    panel->on_close = on_close;
    panel->on_close_userdata = userdata;
    panel->selected_index = -1;
    panel->group = lv_group_create();

    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x11161a), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_user_data(cont, panel);

    lv_obj_t *sheet = lv_obj_create(cont);
    lv_obj_set_width(sheet, LV_PCT(94));
    lv_obj_set_height(sheet, LV_PCT(82));
    lv_obj_center(sheet);
    lv_obj_set_style_bg_color(sheet, lv_color_hex(0x121a20), 0);
    lv_obj_set_style_bg_opa(sheet, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(sheet, LV_DPX(8), 0);
    lv_obj_set_style_border_width(sheet, 1, 0);
    lv_obj_set_style_border_color(sheet, lv_color_hex(0x2c3d49), 0);
    lv_obj_set_style_pad_all(sheet, 0, 0);
    lv_obj_set_flex_flow(sheet, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(sheet, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *header = lv_obj_create(sheet);
    lv_obj_remove_style_all(header);
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_height(header, LV_DPX(72));
    lv_obj_set_style_bg_color(header, lv_color_hex(0x18232c), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, locstr("CTM Device Bridge"));
    lv_obj_set_style_text_color(title, lv_color_hex(0xf5f8fa), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, LV_DPX(18), 0);

    panel->status_label = lv_label_create(header);
    lv_label_set_text(panel->status_label, locstr("starting"));
    lv_obj_set_style_text_color(panel->status_label, lv_color_hex(0xaab6bf), 0);
    lv_obj_align(panel->status_label, LV_ALIGN_RIGHT_MID, LV_DPX(-280), 0);

    lv_obj_t *refresh_btn = lv_btn_create(header);
    lv_obj_set_size(refresh_btn, LV_DPX(120), LV_DPX(44));
    lv_obj_align(refresh_btn, LV_ALIGN_RIGHT_MID, LV_DPX(-150), 0);
    lv_obj_add_event_cb(refresh_btn, refresh_button_cb, LV_EVENT_CLICKED, panel);
    lv_group_add_obj(panel->group, refresh_btn);
    lv_obj_t *refresh_lbl = lv_label_create(refresh_btn);
    lv_label_set_text(refresh_lbl, locstr("Refresh"));
    lv_obj_center(refresh_lbl);

    lv_obj_t *back_btn = lv_btn_create(header);
    lv_obj_set_size(back_btn, LV_DPX(120), LV_DPX(44));
    lv_obj_align(back_btn, LV_ALIGN_RIGHT_MID, LV_DPX(-18), 0);
    lv_obj_add_event_cb(back_btn, back_btn_cb, LV_EVENT_CLICKED, panel);
    lv_group_add_obj(panel->group, back_btn);
    lv_obj_t *back_lbl = lv_label_create(back_btn);
    lv_label_set_text(back_lbl, locstr("Back"));
    lv_obj_center(back_lbl);

    lv_obj_t *devices_title = lv_label_create(sheet);
    lv_label_set_text(devices_title, locstr("Devices"));
    lv_obj_set_style_text_color(devices_title, lv_color_hex(0xf5f8fa), 0);
    lv_obj_set_style_pad_left(devices_title, LV_DPX(18), 0);
    lv_obj_set_style_pad_top(devices_title, LV_DPX(14), 0);

    panel->list = lv_obj_create(sheet);
    lv_obj_set_width(panel->list, LV_PCT(100));
    lv_obj_set_flex_grow(panel->list, 1);
    lv_obj_set_style_bg_opa(panel->list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(panel->list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(panel->list, LV_DPX(12), 0);
    lv_obj_add_flag(panel->list, LV_OBJ_FLAG_SCROLLABLE);
    panel->list_width = lv_obj_get_width(panel->list) - LV_DPX(24);
    if (panel->list_width < LV_DPX(200)) {
        panel->list_width = LV_DPX(520);
    }

    panel->container = cont;
    lv_obj_add_event_cb(cont, panel_delete_cb, LV_EVENT_DELETE, panel);
    panel->refresh_timer = lv_timer_create(refresh_timer_cb, 2000, panel);

    hid_passthrough_panel_refresh(cont);
    lv_group_focus_obj(back_btn);
    return cont;
}

#endif
