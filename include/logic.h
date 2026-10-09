#ifndef LOGIC_H
#define LOGIC_H

#include "types.h"

SystemOutputs evaluate_system_state(BmsData bms, BatteryTempData batt_temp,
                                     InverterData inverter, PvData pv,
                                     GridData grid, RelayState relay,
                                     DoorSwitchData door);

#endif // LOGIC_H
