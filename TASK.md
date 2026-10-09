# Embedded Systems Task: Home Battery Monitoring

This task is set in a different domain from the rest of this assessment. We're not testing Formula Student knowledge here. We're testing how you think about an unfamiliar system, spot edge cases, and justify decisions under ambiguity.

## Scenario

This task is set around a **home battery storage system** that runs 24/7. A central ECU continuously monitors a number of sensors and other ECUs, and must decide whether the system stays **ON** (operating normally) or switches **OFF** (safe shutdown). There is no partial or degraded state, only ON or OFF.

The components below are involved, but how they connect to the central ECU, and to each other, is deliberately left undefined. Some are safety-critical, some are not. Design how these components communicate with the central ECU, decide which are safety-critical and why, and implement the logic that determines the system's state based on that design. You don't need to model a real bus protocol or arbitration scheme. A simplified or abstracted communication approach is fine, but you should be able to justify the approach you chose.

- **BMS**: cell voltages, temperatures, pack current
- **Battery temperature sensor**: enclosure/pack temperature
- **Inverter**: DC-AC conversion status
- **PV array sensor**: solar panel voltage and current
- **Grid sensor**: grid voltage and frequency
- **Grid-tie relay/contactor**: connects/disconnects the system from the utility grid
- **Buzzer**: audible alert output
- **Door/tamper switch**: enclosure access sensor
- **Notification flag**: user-facing app alert

Some of these report continuously, some are event-driven, and some come from a networked component rather than a raw reading. Part of the task is deciding how each of these differences should affect your design.

## What to implement

Struct definitions and mock read/write functions for every component above are already provided (see `include/` and `src/mock_hw.c`), along with a set of predefined fault scenarios (`test/fault_scenarios.c`) you can test against.

`src/logic.c` contains a single empty function, `evaluate_system_state()`. **This is the only file you should edit.** Implement it, plus any helper functions you need, in that file.

## What to submit

- Your implementation of `evaluate_system_state()`, including any helper functions
- Comments explaining your intended approach for any part you weren't able to fully implement. This will still be read and considered
- A written summary (400 words max) listing the cases in which your system switches OFF, and why

There's no single correct architecture, but stronger submissions will demonstrate broader and more rigorous fault coverage, clear reasoning, and code that's easy for someone else to follow.

See `README.md` for setup instructions, what's provided, and how to submit your work.
