#ifndef STATE_H
#define STATE_H

#include <stdint.h>

// Define the possible states
typedef enum {
    STATE_IDLE,
    STATE_STREAMING,
    STATE_SAVING,
    STATE_ERROR
} FirmwareState;

// Define a structure to hold system information
typedef struct {
    FirmwareState current_state;
    uint32_t flash_storage_used; // in bytes
    uint32_t running_time;       // in seconds
    float battery_level;         // percentage
} SystemStatus;

// Function declarations
void fw_initialize_system_status(void);
void fw_set_state(FirmwareState new_state);
FirmwareState fw_get_state(void);
void fw_update_flash_storage_used(uint32_t bytes);
void fw_update_running_time(uint32_t time);
void fw_update_battery_level(float level);
SystemStatus fw_get_system_status(void);

#endif // STATE_H
