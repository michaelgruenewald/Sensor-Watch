#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "og_stopwatch_face.h"
#include "watch.h"
#include "watch_utility.h"

#define PRECISION 32

static void update_display(og_stopwatch_state_t *stopwatch_state) {
    uint32_t ticks;

    if (stopwatch_state->ticks_halted) {
        watch_set_indicator(WATCH_INDICATOR_LAP);
        ticks = stopwatch_state->ticks_halted;
    } else {
        watch_clear_indicator(WATCH_INDICATOR_LAP);
        ticks = stopwatch_state->ticks_counted;
    }

    uint32_t centiseconds = ticks * 100 / PRECISION;
    uint32_t seconds = ticks / PRECISION;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    uint32_t days = hours / 24;

    char buf[11];
    if (days > 39) {
        snprintf(buf, sizeof buf, "ST   >40d ");
    } else if (days) {
        snprintf(buf, sizeof buf, "ST%2"PRIu32"%02"PRIu32"%02"PRIu32"%02"PRIu32, days % 40, hours % 24, minutes % 60, seconds % 60);
    } else if (hours) {
        snprintf(buf, sizeof buf, "ST  %02"PRIu32"%02"PRIu32"%02"PRIu32, hours % 24, minutes % 60, seconds % 60);
    } else {
        snprintf(buf, sizeof buf, "ST  %02"PRIu32"%02"PRIu32"%02"PRIu32, minutes % 60, seconds % 60, centiseconds % 100);
    }
    watch_display_string(buf, 0);
}

void og_stopwatch_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(og_stopwatch_state_t));
        memset(*context_ptr, 0, sizeof(og_stopwatch_state_t));
    }
}

void og_stopwatch_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;
    if (watch_tick_animation_is_running()) {
        watch_stop_tick_animation();
    }
}

bool og_stopwatch_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    (void) settings;
    og_stopwatch_state_t *stopwatch_state = (og_stopwatch_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            watch_set_colon();
            if (stopwatch_state->running) {
                movement_request_tick_frequency(PRECISION);
                uint32_t gone_for = watch_utility_date_time_to_unix_time(watch_rtc_get_date_time(), 0) - watch_utility_date_time_to_unix_time(stopwatch_state->resign_time, 0);
                stopwatch_state->ticks_counted += gone_for * PRECISION;
            }
            update_display(stopwatch_state);
            break;
        case EVENT_TICK:
            if (stopwatch_state->running) {
                stopwatch_state->ticks_counted++;
            }
            update_display(stopwatch_state);
            break;
        case EVENT_MODE_BUTTON_UP:
            movement_move_to_next_face();
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            movement_illuminate_led();
            if (stopwatch_state->running) {
                if (stopwatch_state->ticks_halted) {
                    stopwatch_state->ticks_halted = 0;
                } else {
                    stopwatch_state->ticks_halted = stopwatch_state->ticks_counted;
                }
            } else {
                stopwatch_state->ticks_counted = 0;
            }
            update_display(stopwatch_state);
            break;
        case EVENT_ALARM_BUTTON_DOWN:
            stopwatch_state->running = !stopwatch_state->running;
            if (stopwatch_state->running) {
                movement_request_tick_frequency(PRECISION);
            } else {
                movement_request_tick_frequency(1);
            }
            break;
        case EVENT_TIMEOUT:
            // do not time out, keep the stopwatch on screen
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            break;
        default:
            break;
    }

    return true;
}

void og_stopwatch_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    og_stopwatch_state_t *stopwatch_state = (og_stopwatch_state_t *)context;

    stopwatch_state->resign_time = watch_rtc_get_date_time();
}
