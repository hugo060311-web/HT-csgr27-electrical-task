/* Web simulation copy, assembled from the existing D project. Original submission files are unchanged. */
#define PICO_BUILD 1

/* ===== include/types.h ===== */
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


/* ===== include/logic.h ===== */
#ifndef LOGIC_H
#define LOGIC_H
SystemOutputs evaluate_system_state(BmsData bms, BatteryTempData batt_temp,
                                     InverterData inverter, PvData pv,
                                     GridData grid, RelayState relay,
                                     DoorSwitchData door);

#endif // LOGIC_H


/* ===== include/mock_hw.h ===== */
/*
 * Mock hardware layer for the home battery system. Provides one function
 * to read each sensor/ECU and one function to write each actuator, so the
 * rest of the code never has to know how the underlying data is stored or
 * where it comes from. The test harness sets what these read functions
 * return by calling load_world_state() with a full snapshot of every
 * component before each scenario.
 */

#ifndef MOCK_HW_H
#define MOCK_HW_H

#include <stdint.h>
typedef struct {
    BmsData bms;
    BatteryTempData batt_temp;
    InverterData inverter;
    PvData pv;
    GridData grid;
    RelayState relay;
    DoorSwitchData door;
    uint32_t sim_time_ms;
} WorldState;

void load_world_state(const WorldState *world);

BmsData read_bms(void);
BatteryTempData read_batt_temp(void);
InverterData read_inverter(void);
PvData read_pv(void);
GridData read_grid(void);
RelayState read_relay(void);
DoorSwitchData read_door_switch(void);
uint32_t read_system_time_ms(void);

void write_relay(bool closed);
void write_buzzer(bool on);
void write_notification(bool on);

#endif // MOCK_HW_H


/* ===== test/fault_scenarios.h ===== */
#ifndef FAULT_SCENARIOS_H
#define FAULT_SCENARIOS_H

#include <stddef.h>
typedef struct {
    const char *name;
    WorldState world;
} FaultScenario;

extern const FaultScenario FAULT_SCENARIOS[];
extern const size_t NUM_FAULT_SCENARIOS;

#endif // FAULT_SCENARIOS_H


/* ===== src/mock_hw.c ===== */
static WorldState g_world;
static bool g_buzzer_on;
static bool g_notification_on;

void load_world_state(const WorldState *world) {
    g_world = *world;
    g_buzzer_on = false;
    g_notification_on = false;
}

BmsData read_bms(void) {
    return g_world.bms;
}

BatteryTempData read_batt_temp(void) {
    return g_world.batt_temp;
}

InverterData read_inverter(void) {
    return g_world.inverter;
}

PvData read_pv(void) {
    return g_world.pv;
}

GridData read_grid(void) {
    return g_world.grid;
}

RelayState read_relay(void) {
    return g_world.relay;
}

DoorSwitchData read_door_switch(void) {
    return g_world.door;
}

uint32_t read_system_time_ms(void) {
    return g_world.sim_time_ms;
}

void write_relay(bool closed) {
    g_world.relay.commanded_closed = closed;
}

void write_buzzer(bool on) {
    g_buzzer_on = on;
}

void write_notification(bool on) {
    g_notification_on = on;
}


/* ===== src/logic.c ===== */
// This is the only file you should edit.
// AI assistance: helped draft and review these fault checks; thresholds and
// behaviour still need to be understood and confirmed by the applicant.

#include <math.h>
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


/* ===== test/fault_scenarios.c ===== */
/*
 * Predefined scenarios covering nominal operation and a range of single and
 * combined fault conditions across the components. Feel free to edit these
 * or add your own for your own testing. Submissions are evaluated against
 * whatever is committed here at the deadline.
 */
const FaultScenario FAULT_SCENARIOS[] = {
    {
        .name = "nominal_operation",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "battery_over_temperature",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 85.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "cell_under_voltage",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 2800, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "cell_over_voltage",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 4300, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "bms_comms_lost",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = false,
                .last_update_ms = 50000,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "inverter_comms_lost",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = false,
                .last_update_ms = 50000,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "bms_stale_despite_comms_ok",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 20000,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "inverter_stale_despite_comms_ok",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 20000,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "door_triggered_alone",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = true, .timestamp_ms = 99900 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "door_triggered_with_bms_comms_lost",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = false,
                .last_update_ms = 50000,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = true, .timestamp_ms = 99900 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "relay_stuck_open",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = false },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "relay_stuck_closed",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = false, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "grid_absent",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 0.0f, .frequency_hz = 0.0f, .present = false },
            .relay = { .commanded_closed = false, .feedback_closed = false },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "grid_out_of_band",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 25.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 260.0f, .frequency_hz = 52.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = false, .timestamp_ms = 0 },
            .sim_time_ms = 100000,
        },
    },
    {
        .name = "multiple_simultaneous_faults",
        .world = {
            .bms = {
                .cell_voltages_mv = {3700, 3700, 3700, 2800, 3700, 3700, 3700, 3700},
                .pack_current_a = -2.5f,
                .pack_temp_c = 25.0f,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .batt_temp = { .temp_c = 85.0f, .valid = true },
            .inverter = {
                .running = true,
                .fault_code = 0,
                .comms_ok = true,
                .last_update_ms = 99950,
            },
            .pv = { .voltage_v = 380.0f, .current_a = 8.0f },
            .grid = { .voltage_v = 230.0f, .frequency_hz = 50.0f, .present = true },
            .relay = { .commanded_closed = true, .feedback_closed = true },
            .door = { .triggered = true, .timestamp_ms = 99900 },
            .sim_time_ms = 100000,
        },
    },
};

const size_t NUM_FAULT_SCENARIOS = sizeof(FAULT_SCENARIOS) / sizeof(FAULT_SCENARIOS[0]);


/* ===== src/main.c ===== */
/*
 * Test harness: replays the fixed fault scenarios, prints the result of
 * each, and drives the outputs described below. You should not need to
 * edit this file. Implement your logic in src/logic.c instead (the only
 * file you should edit).
 *
 * GPIO pin mapping (Pico/Wokwi builds), mirrored in README.md:
 *   GP2 - System ON LED         (output)
 *   GP3 - System OFF LED        (output)
 *   GP4 - Buzzer                (output)
 *   GP5 - Notification LED      (output)
 *   GP6 - Door/tamper switch    (input, active low, internal pull-up)
 */

#include <stdio.h>
void web_serial_init(void);
int web_printf(const char *format, ...);
#define printf web_printf
#ifdef PICO_BUILD
#include "pico/stdlib.h"

#define PIN_LED_SYSTEM_ON 2
#define PIN_LED_SYSTEM_OFF 3
#define PIN_BUZZER 4
#define PIN_NOTIFICATION 5
#define PIN_DOOR_SWITCH 6

static void hal_init(void) {
    web_serial_init();
    sleep_ms(2000); // Allow the simulated USB serial connection to settle.
    gpio_init(PIN_LED_SYSTEM_ON);
    gpio_set_dir(PIN_LED_SYSTEM_ON, GPIO_OUT);
    gpio_init(PIN_LED_SYSTEM_OFF);
    gpio_set_dir(PIN_LED_SYSTEM_OFF, GPIO_OUT);
    gpio_init(PIN_BUZZER);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);
    gpio_init(PIN_NOTIFICATION);
    gpio_set_dir(PIN_NOTIFICATION, GPIO_OUT);
    gpio_init(PIN_DOOR_SWITCH);
    gpio_set_dir(PIN_DOOR_SWITCH, GPIO_IN);
    gpio_pull_up(PIN_DOOR_SWITCH);
}

static void hal_apply_outputs(SystemOutputs out) {
    gpio_put(PIN_LED_SYSTEM_ON, out.state == SYSTEM_ON);
    gpio_put(PIN_LED_SYSTEM_OFF, out.state == SYSTEM_OFF);
    gpio_put(PIN_BUZZER, out.buzzer_on);
    gpio_put(PIN_NOTIFICATION, out.notification_flag);
}

static bool hal_door_switch_pressed(void) {
    return !gpio_get(PIN_DOOR_SWITCH);
}

static void hal_delay(void) {
    sleep_ms(2000);
}
#else
static void hal_init(void) {
}

static void hal_apply_outputs(SystemOutputs out) {
    (void)out;
}

static bool hal_door_switch_pressed(void) {
    return false;
}

static void hal_delay(void) {
}
#endif

static void run_scenario(const FaultScenario *scenario) {
    load_world_state(&scenario->world);

    BmsData bms = read_bms();
    BatteryTempData batt_temp = read_batt_temp();
    InverterData inverter = read_inverter();
    PvData pv = read_pv();
    GridData grid = read_grid();
    RelayState relay = read_relay();
    DoorSwitchData door = read_door_switch();

    if (hal_door_switch_pressed()) {
        door.triggered = true;
        door.timestamp_ms = read_system_time_ms();
    }

    SystemOutputs out = evaluate_system_state(bms, batt_temp, inverter, pv, grid, relay, door);

    write_buzzer(out.buzzer_on);
    write_notification(out.notification_flag);

    printf("[%s] state=%s buzzer=%s notification=%s\n", scenario->name,
           out.state == SYSTEM_ON ? "ON" : "OFF", out.buzzer_on ? "ON" : "OFF",
           out.notification_flag ? "ON" : "OFF");

    hal_apply_outputs(out);
}

void run_task_scenarios(void) {
    hal_init();

#ifdef PICO_BUILD
    for (;;) {
#endif
        for (size_t i = 0; i < NUM_FAULT_SCENARIOS; i++) {
            run_scenario(&FAULT_SCENARIOS[i]);
            hal_delay();
        }
#ifdef PICO_BUILD
    }
#endif

    return;
}
