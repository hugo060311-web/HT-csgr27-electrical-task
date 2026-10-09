# CSGR27 Electrical: embedded systems task

See [`TASK.md`](TASK.md) for the full task description. This README covers what's in this repository, how to build/test it, and how to submit your work.

> **`src/logic.c` is the only file you should edit**, though you're also free to edit `test/fault_scenarios.c` to add or change scenarios for your own testing. Everything else is test harness and scaffolding, provided as-is.

## What's in this repository

- `include/types.h`: struct definitions for every sensor/ECU listed in the task
- `include/mock_hw.h`, `src/mock_hw.c`: mock read/write functions for each component, backed by an internal mock hardware state
- `test/fault_scenarios.h`, `test/fault_scenarios.c`: a set of predefined, named fault scenarios you can test your logic against (nominal operation, single faults, comms loss, stale data, door/tamper events, relay mismatches, multiple simultaneous faults, and more)
- `include/logic.h`, `src/logic.c`: `logic.c` contains an empty `evaluate_system_state()` function. **This is the only file you should edit.**
- `src/main.c`: replays every fault scenario in a loop, prints the resulting state/buzzer/notification to the console, and drives GPIO/LEDs/buzzer when built for Pico/Wokwi
- `diagram.json`, `wokwi.toml`: Wokwi simulation files so you can visualise your logic against a simple wired circuit
- `CMakeLists.txt`, `pico_sdk_import.cmake`: build system (see below)

You're free to edit `test/fault_scenarios.c` or add your own scenarios for your own testing, but your submission will be evaluated against the fault scenarios **committed in this repository** at the deadline, so make sure anything you want considered is committed here.

## GPIO pin mapping (Pico/Wokwi builds)

| Pin | Component | Direction |
| --- | --- | --- |
| GP2 | System ON LED | output |
| GP3 | System OFF LED | output |
| GP4 | Buzzer | output |
| GP5 | Notification LED | output |
| GP6 | Door/tamper switch | input (active low, internal pull-up) |

## Testing your work

You can work however suits you:

- **Wokwi web editor**: no local setup required. Paste or import this repository and run it in the browser.
- **Wokwi CLI or VS Code extension**: manages the Pico SDK and toolchain for you locally.
- **Plain Pico SDK build**: install the `arm-none-eabi` toolchain and CMake, then either set `PICO_SDK_PATH` to a local SDK checkout, or configure with `-DPICO_SDK_FETCH_FROM_GIT=ON` to have CMake fetch the SDK automatically:
  ```
  cmake -S . -B build -DPICO_SDK_FETCH_FROM_GIT=ON
  cmake --build build
  ```
- **Native build (fastest iteration)**: no Pico SDK or ARM toolchain needed at all. Compiles with your system's plain `gcc`/`cmake` and prints scenario results straight to your terminal. Useful while you're iterating on `logic.c`, before doing a final check on real hardware or in Wokwi:
  ```
  cmake -S . -B build -DNATIVE_BUILD=ON
  cmake --build build
  ./build/embedded_task
  ```

If you test in the Wokwi web editor, remember to copy any changes back into this repository before the deadline. The web editor doesn't sync to GitHub automatically.

## Submitting

1. Create your own individual repository from this template, using your **personal GitHub account**.
2. Name it `[first initial][last initial]-[repo name]` (e.g. `bo-csgr27-electrical-task`).
3. **Keep your repository public.** We can't mark your work if we can't access it.
4. Send us the repository URL as your submission.

Note that a public repository means other applicants may be able to see your work. You're welcome to discuss the task with friends, but the code and write-up should be your own. You may use AI tools to help write your code, but you must explicitly disclose where you used them in your submission.

Code structure, clarity, and general SWE practice (including how your work evolved through your commit history) will be part of the review, alongside fault coverage and reasoning, and the write-up. Commits made after the deadline will not be taken into account.
