#ifndef MEMORY_CPP
#define MEMORY_CPP

extern "C" {
    #include "user_interface.h"
}

void memoryPrintMemoryStatus() {
    uint32_t free = system_get_free_heap_size();
    Serial.print(F("Free memory: "));
    Serial.println(free);
}

#endif
