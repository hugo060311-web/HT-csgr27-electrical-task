#include "mock_hw.h"

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
