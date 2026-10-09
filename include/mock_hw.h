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

#include "types.h"

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
