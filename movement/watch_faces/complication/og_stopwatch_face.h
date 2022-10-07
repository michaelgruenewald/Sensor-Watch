#ifndef OG_STOPWATCH_FACE_H_
#define OG_STOPWATCH_FACE_H_

#include "movement.h"

typedef struct {
    bool running;
    uint32_t ticks_counted;
    uint32_t ticks_halted;
    watch_date_time resign_time;
} og_stopwatch_state_t;

void og_stopwatch_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr);
void og_stopwatch_face_activate(movement_settings_t *settings, void *context);
bool og_stopwatch_face_loop(movement_event_t event, movement_settings_t *settings, void *context);
void og_stopwatch_face_resign(movement_settings_t *settings, void *context);

#define og_stopwatch_face ((const watch_face_t){ \
    og_stopwatch_face_setup, \
    og_stopwatch_face_activate, \
    og_stopwatch_face_loop, \
    og_stopwatch_face_resign, \
    NULL, \
})

#endif // OG_STOPWATCH_FACE_H_
