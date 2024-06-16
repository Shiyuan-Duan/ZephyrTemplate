#include "state.h"

static SystemStatus system_status = {0};

void initialize_system_status(void) {
    system_status.current_state = STATE_IDLE;
    system_status.flash_storage_used = 0;
    system_status.running_time = 0;
    system_status.battery_level = 100.0;
}

void fw_set_state(FirmwareState new_state) {
    system_status.current_state = new_state;
}

FirmwareState fw_get_state(void) {
    return system_status.current_state;
}

void fw_update_flash_storage_used(uint32_t bytes) {
    system_status.flash_storage_used += bytes;
}

void fw_update_running_time(uint32_t time) {
    system_status.running_time += time;
}

void fw_update_battery_level(float level) {
    system_status.battery_level = level;
}

SystemStatus fw_get_system_status(void) {
    return system_status;
}
