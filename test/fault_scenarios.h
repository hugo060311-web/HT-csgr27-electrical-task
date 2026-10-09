#ifndef FAULT_SCENARIOS_H
#define FAULT_SCENARIOS_H

#include <stddef.h>

#include "mock_hw.h"

typedef struct {
    const char *name;
    WorldState world;
} FaultScenario;

extern const FaultScenario FAULT_SCENARIOS[];
extern const size_t NUM_FAULT_SCENARIOS;

#endif // FAULT_SCENARIOS_H
