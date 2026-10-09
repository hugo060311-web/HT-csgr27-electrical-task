#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

extern "C" void run_task_scenarios(void);

extern "C" void web_serial_init(void) {
  Serial.begin(115200);
  unsigned long started = millis();
  while (!Serial && millis() - started < 5000) {
    delay(10);
  }
}

extern "C" int web_printf(const char *format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  int count = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  if (count > 0) Serial.print(buffer);
  return count;
}

void setup() {
  run_task_scenarios();
}

void loop() {
}