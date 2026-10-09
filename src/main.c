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

#include "fault_scenarios.h"
#include "logic.h"
#include "mock_hw.h"

#ifdef PICO_BUILD
#include "pico/stdlib.h"

#define PIN_LED_SYSTEM_ON 2
#define PIN_LED_SYSTEM_OFF 3
#define PIN_BUZZER 4
#define PIN_NOTIFICATION 5
#define PIN_DOOR_SWITCH 6

static void hal_init(void) {
    stdio_init_all();
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

int main(void) {
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

    return 0;
}
