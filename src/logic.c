// This is the only file you should edit.
// AI assistance: helped draft and review these fault checks; thresholds and
// behaviour still need to be understood and confirmed by the applicant.

#include <math.h>

#include "logic.h"
#include "mock_hw.h"

// These are exercise assumptions. A real battery must use limits from its BMS
// and battery manufacturer.
#define MIN_CELL_MV 3000.0f
#define MAX_CELL_MV 4200.0f
#define MAX_BATTERY_TEMP_C 60.0f
#define MAX_DATA_AGE_MS 1000u

static bool data_is_stale(uint32_t now_ms, uint32_t update_ms) {
    return (uint32_t)(now_ms - update_ms) > MAX_DATA_AGE_MS;
}

SystemOutputs evaluate_system_state(BmsData bms, BatteryTempData batt_temp,
                                     InverterData inverter, PvData pv,
                                     GridData grid, RelayState relay,
                                     DoorSwitchData door) {
    SystemOutputs out = {0};
    bool fault = false;
    uint32_t now_ms = read_system_time_ms();

    (void)pv; // Zero solar output can be normal, for example at night.

    // Missing or old data from either safety-related controller is unsafe.
    if (!bms.comms_ok || data_is_stale(now_ms, bms.last_update_ms) ||
        !inverter.comms_ok || data_is_stale(now_ms, inverter.last_update_ms)) {
        fault = true;
    }

    if (!batt_temp.valid || !isfinite(batt_temp.temp_c) ||
        batt_temp.temp_c >= MAX_BATTERY_TEMP_C ||
        !isfinite(bms.pack_temp_c) || bms.pack_temp_c >= MAX_BATTERY_TEMP_C ||
        !isfinite(bms.pack_current_a)) {
        fault = true;
    }

    for (int i = 0; i < BMS_NUM_CELLS; i++) {
        if (!isfinite(bms.cell_voltages_mv[i]) ||
            bms.cell_voltages_mv[i] < MIN_CELL_MV ||
            bms.cell_voltages_mv[i] > MAX_CELL_MV) {
            fault = true;
        }
    }

    if (!inverter.running || inverter.fault_code != 0) {
        fault = true;
    }

    // The grid limits are illustrative for this task, not certified settings.
    if (grid.present &&
        (!isfinite(grid.voltage_v) || !isfinite(grid.frequency_hz) ||
         grid.voltage_v < 207.0f || grid.voltage_v > 253.0f ||
         grid.frequency_hz < 49.0f || grid.frequency_hz > 51.0f)) {
        fault = true;
    }

    // Do not accept a closed grid connection when the grid is absent.
    if ((!grid.present && (relay.commanded_closed || relay.feedback_closed)) ||
        relay.commanded_closed != relay.feedback_closed) {
        fault = true;
    }

    // Treat a tamper event as a shutdown request.
    if (door.triggered) {
        fault = true;
    }

    if (fault) {
        out.state = SYSTEM_OFF;
        out.buzzer_on = true;
        out.notification_flag = true;
        write_relay(false);
    } else {
        out.state = SYSTEM_ON;
    }

    return out;
}
