/*
 * Change the BLE device name to "<prefix> BT<n>" whenever the active
 * profile changes, so each host shows which profile it is paired with.
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static char current_name[CONFIG_BT_DEVICE_NAME_MAX + 1];

static void apply_profile_name(struct k_work *work) {
    char name[CONFIG_BT_DEVICE_NAME_MAX + 1];

    snprintf(name, sizeof(name), "%s BT%d", CONFIG_MONA2_BLE_PROFILE_NAME_PREFIX,
             zmk_ble_active_profile_index());

    if (strcmp(name, current_name) == 0) {
        return; /* profile-changed is also raised on (dis)connect; skip if unchanged */
    }

    int err = zmk_ble_set_device_name(name);
    if (err) {
        LOG_ERR("Failed to set profile device name (err %d)", err);
        return;
    }
    strcpy(current_name, name);
}

K_WORK_DEFINE(apply_profile_name_work, apply_profile_name);

static int ble_profile_name_listener(const zmk_event_t *eh) {
    if (as_zmk_ble_active_profile_changed(eh) != NULL) {
        k_work_submit(&apply_profile_name_work);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(mona2_ble_profile_name, ble_profile_name_listener);
ZMK_SUBSCRIPTION(mona2_ble_profile_name, zmk_ble_active_profile_changed);
