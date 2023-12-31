#include <stdbool.h>
#include <stdlib.h>

#include "SDL.h"

#include "log.h"

#include "alloc.h"
#include "app.h"
#include "video.h"

// --- Window
#define DEFAULT_WINDOW_FLAGS 0

/**
 * Initialize static application.
 *
 * TODO: Return error code.
 */
app_t *app_init(app_config_t *config) {

    app_t *app   = new (app_t);
    app->video   = NULL;
    app->running = false;

    log_set_level(LOG_DEBUG);

    if (!(app->video = video_init(
              &(video_cfg_t){.window_title         = config->window_title,
                             .window_position_x    = config->window_position_x,
                             .window_position_y    = config->window_position_y,
                             .window_width         = config->window_width,
                             .window_height        = config->window_height,
                             .window_is_fullscreen = config->window_is_fullscreen}))) {

        log_error("Cannot initialize video sub-system");
        app_term(app);
        return NULL;
    }

    // --- RNG
    // Seed RNG
    srand(time(NULL));
    // Toss out the first few values.
    for (int count = 0; count < 1000; count++) {
        rand();
    }

    return app;
}

/**
 * Terminate static applicaiton.
 */
void app_term(app_t *app) {
    if (!app) {
        return;
    }
    video_term(app->video);
    SDL_Quit();
    delete (app);
}

void app_run(app_t *app, frame_processor_t process_frame,
             event_processor_t process_event) {

    // --- Validation Checks
    if (!app) {
        log_error("Application instance is NULL");
    }

    if (!process_frame) {
        log_error("Frame Processor is NULL");
    }

    if (!process_event) {
        log_error("Event Processor is NULL");
    }

    // Set running flag.
    app->running = true;

    /** CPU ticks at the start of the last frame. */
    uint64_t prev_frame_ticks = SDL_GetTicks64();

    /** CPU ticks at the start of this frame. */
    uint64_t curr_frame_ticks = 0;

    /** Time at the beginning of the frame. */
    uint64_t frame_start_time = 0;

    /** Time at the end of the frame. */
    uint64_t frame_end_time = 0;

    /** Milliseconds this frame has taken. */
    float elapsed_frame_ms = 0;

    /** Time between frames. Measured in seconds. */
    float delta = 0;

    // --- Application Loop
    while (app->running) {

        // --- Start Frame Timing

        frame_start_time = SDL_GetPerformanceCounter();
        curr_frame_ticks = SDL_GetTicks64();
        delta            = (curr_frame_ticks - prev_frame_ticks) / 1000.0f;

        // --- Poll input events
        /** Input Event Processing */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            process_event(app, &event);
            // --- Check OS-level quit request
            if (event.type == SDL_QUIT) {
                app->running = false;
                break;
            }
        }

        // --- Process Frame
        process_frame(app, delta);

        // --- End Frame Timing
        //
        prev_frame_ticks = curr_frame_ticks;
        frame_end_time   = SDL_GetPerformanceCounter();
        elapsed_frame_ms = (frame_end_time - frame_start_time) /
                           (float)SDL_GetPerformanceFrequency() * 1000.0f;

        // 60 FPS in Milliseconds
        // == 1 (frame) / 60 (seconds) * 1000 (convert to ms)
        static float const FPS60 = 16.666f;

        long frame_delay = floor(FPS60 - elapsed_frame_ms);

        // Delay each frame to get as close to 60FPS as possible.
        if (frame_delay < 0)
            frame_delay = 0;
        SDL_Delay(frame_delay);
    }

    return;
}

void app_stop(app_t *app) { app->running = false; }
