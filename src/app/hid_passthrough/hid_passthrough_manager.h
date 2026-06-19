#ifndef HID_PASSTHROUGH_MANAGER_H
#define HID_PASSTHROUGH_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#if defined(TARGET_WEBOS)

#define HID_PT_PATH_LEN 64
#define HID_PT_DEFAULT_PORT 48054

struct hid_passthrough_manager {
    bool running;
    char host[128];
    int port;
};

typedef struct hid_passthrough_manager hid_passthrough_manager_t;

typedef struct {
    char path[HID_PT_PATH_LEN];
    char product[64];
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t usage_page;
    uint16_t usage;
    bool plugged;
    bool connected;
} hid_pt_device_info_t;

void hid_passthrough_manager_init(hid_passthrough_manager_t *manager);

void hid_passthrough_manager_deinit(hid_passthrough_manager_t *manager);

int hid_passthrough_manager_start(hid_passthrough_manager_t *manager, const char *host, int port);

void hid_passthrough_manager_stop(hid_passthrough_manager_t *manager);

bool hid_passthrough_manager_active(const hid_passthrough_manager_t *manager);

int hid_passthrough_manager_device_count(hid_passthrough_manager_t *manager);

int hid_passthrough_manager_get_device(hid_passthrough_manager_t *manager, int index,
                                       hid_pt_device_info_t *info);

int hid_passthrough_manager_plug(hid_passthrough_manager_t *manager, const char *path);

int hid_passthrough_manager_unplug(hid_passthrough_manager_t *manager, const char *path);

void hid_passthrough_manager_rescan(hid_passthrough_manager_t *manager);

#else

typedef struct hid_passthrough_manager {
    int unused;
} hid_passthrough_manager_t;

typedef struct {
    char path[64];
    char product[64];
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t usage_page;
    uint16_t usage;
    bool plugged;
    bool connected;
} hid_pt_device_info_t;

static inline void hid_passthrough_manager_init(hid_passthrough_manager_t *manager) {
    (void) manager;
}

static inline void hid_passthrough_manager_deinit(hid_passthrough_manager_t *manager) {
    (void) manager;
}

static inline int hid_passthrough_manager_start(hid_passthrough_manager_t *manager, const char *host, int port) {
    (void) manager;
    (void) host;
    (void) port;
    return -1;
}

static inline void hid_passthrough_manager_stop(hid_passthrough_manager_t *manager) {
    (void) manager;
}

static inline bool hid_passthrough_manager_active(const hid_passthrough_manager_t *manager) {
    (void) manager;
    return false;
}

#endif

#endif
