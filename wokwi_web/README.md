# Wokwi web simulation

Project: https://wokwi.com/projects/477397690398701569

Use a Raspberry Pi Pico Arduino project. Paste sketch.ino and diagram.json into the matching tabs and add main.c. Save and run. The simulation cycles through the 15 supplied scenarios at about two seconds each. Hold the door/tamper button through a normal scenario to check shutdown.

main.c combines the existing C headers, mock layer, fault logic and scenarios. sketch.ino supplies the Arduino serial/startup adapter. AI assisted with this simulation packaging and adapter. Original src/logic.c and SUMMARY.md are not modified.

The screenshots show automatic playback and the normal-to-shutdown button transition. They do not separately document release recovery or audible buzzer performance.
