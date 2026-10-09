#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define BMS_NUM_CELLS 8

// Battery management system: per-cell voltages, pack current, and the pack
// temperature as reported by the BMS itself.
typedef struct {
    float cell_voltages_mv[BMS_NUM_CELLS];
    float pack_current_a;
    float pack_temp_c;
    bool comms_ok;
    uint32_t last_update_ms;
} BmsData;

// Standalone battery enclosure/pack temperature sensor.
typedef struct {
    float temp_c;
    bool valid;
} BatteryTempData;

// DC-AC inverter status.
typedef struct {
    bool running;
    uint8_t fault_code;
    bool comms_ok;
    uint32_t last_update_ms;
} InverterData;

// Solar PV array voltage and current.
typedef struct {
    float voltage_v;
    float current_a;
} PvData;

// Utility grid voltage and frequency at the point of connection.
typedef struct {
    float voltage_v;
    float frequency_hz;
    bool present;
} GridData;

// Grid-tie relay/contactor: the last commanded position vs. the position
// the relay itself reports.
typedef struct {
    bool commanded_closed;
    bool feedback_closed;
} RelayState;

// Enclosure door/tamper switch.
typedef struct {
    bool triggered;
    uint32_t timestamp_ms;
} DoorSwitchData;

typedef enum {
    SYSTEM_OFF = 0,
    SYSTEM_ON = 1,
} SystemState;

// Outputs produced by evaluate_system_state().
typedef struct {
    SystemState state;
    bool buzzer_on;
    bool notification_flag;
} SystemOutputs;

#endif // TYPES_H
