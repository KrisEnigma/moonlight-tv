/* Flydigi Apex 4 (and compatible dongles): composite HID passthrough via hidraw. */

#define _GNU_SOURCE

#include "ctm_controller.h"
#include "ctm_state.h"

#include <stdio.h>
#include <string.h>

static bool flydigi_matches(const ctm_controller_dev_t *dev)
{
    if (!dev) return false;
    if (strcmp(dev->vid, "04b4") == 0 && strcmp(dev->pid, "2412") == 0) return true;
    if (dev->usb_busid[0] && is_flydigi_usb_busid(dev->usb_busid)) return true;
    if (is_xpad_compatible_pid(dev->vid, dev->pid) && dev->usb_busid[0] &&
        is_flydigi_usb_busid(dev->usb_busid)) {
        return true;
    }
    if (!dev->name[0]) return false;
#ifdef _GNU_SOURCE
    return strcasestr(dev->name, "flydigi") != NULL ||
           strcasestr(dev->name, "vader") != NULL ||
           strcasestr(dev->name, "apex") != NULL;
#else
    return false;
#endif
}

static int flydigi_select_node(const ctm_controller_dev_t *dev, char *out, size_t out_len)
{
    if (!dev || !out || out_len == 0) return -1;
    if (dev->usb_busid[0] &&
        flydigi_handshake_hidraw_path_for_busid(dev->usb_busid, out, out_len) == 0) {
        return 0;
    }
    if (dev->path[0] && strstr(dev->path, "/dev/hidraw") != NULL) {
        snprintf(out, out_len, "%s", dev->path);
        return 0;
    }
    return -1;
}

const ctm_controller_ops_t ctm_controller_flydigi_ops = {
    .kind = "flydigi",
    .matches = flydigi_matches,
    .select_node = flydigi_select_node,
    .on_plug_init = NULL,
    .patch_output = NULL,
    .set_settings = NULL,
    .composite = true,
    .composite_evdev_gamepad = true,
};
