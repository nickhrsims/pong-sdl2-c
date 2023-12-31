#include "paddle.h"
#include "aabb.h"
#include "entity.h"
#include "log.h"

// --- Field Section Horizontal Ratio
// (How small should the subsection of field be in which the paddle is centered)
#define PADDLE_FIELD_WIDTH_RATIO 6

// --- Minimum Size
#define PADDLE_MIN_WIDTH  8
#define PADDLE_MIN_HEIGHT 128

static void update(entity_t *paddle, float delta) {
    paddle->transform.x += (int)(paddle->vx * delta);
    paddle->transform.y += (int)(paddle->vy * delta);
}

static void out_of_bounds(entity_t *self, aabb_edge_t edge) {
    switch (edge) {
    case AABB_TOP_EDGE:
        entity_set_direction(self, DIR_DOWN);
        break;
    case AABB_BOTTOM_EDGE:
        entity_set_direction(self, DIR_UP);
        break;
    default:
        break;
    }
}

/**
 * Configure `paddle` based on playing `field` of type `identifier`.
 */
void paddle_configure(entity_t *paddle, aabb_t *field, paddle_identifier_t identifier) {
    // A vertical slice of the field of common dimensions between
    //  both the left and right paddle.
    //
    // The paddle identifier (left or right) determines where the
    // field section begins horizontally.
    //
    //   In the left case:
    //    field section occupies this space on the left end of the field.
    //    ----------------
    //   |XXX|            |
    //   |XXX|            |
    //   |XXX|            |
    //    ----------------
    //
    // In the right case:
    //   field section occupies this space on the right end of the field.
    //    ----------------
    //   |            |XXX|
    //   |            |XXX|
    //   |            |XXX|
    //    ----------------
    //
    // The specific width of the field section is determined
    // by the field width ratio.
    //
    // This field section is used to simplify positioning logic.
    aabb_t field_section;

    // A "unit width" conforming to a common ratio.
    int unit_scaled_field_width = (field->x + field->w) / PADDLE_FIELD_WIDTH_RATIO;

    // --- Common properties of field section
    field_section.x = 0;
    field_section.w = unit_scaled_field_width;
    field_section.y = field->y;
    field_section.h = field->h;

    // --- Left or Right Paddle?
    // (where does the section begin horizontally?)
    switch (identifier) {
    case LEFT_PADDLE:
        // the field section begins where the field begins.
        // (the left end)
        field_section.x = field->x;
        break;
    case RIGHT_PADDLE:
        // the field section begins at the right-most edge
        // minus one unit-scaled width.
        // (the right end)
        field_section.x = (field->x + field->w) - unit_scaled_field_width;
        break;
    }

    // --- Position
    int field_section_center_x = field_section.x + field_section.w / 2;
    int field_section_center_y = field_section.y + field_section.h / 2;
    paddle->transform.x        = field_section_center_x;
    paddle->transform.y        = field_section_center_y;

    // --- Size
    // TODO: Scale with field.
    paddle->transform.w = PADDLE_MIN_WIDTH;
    paddle->transform.h = PADDLE_MIN_HEIGHT;

    // --- Velocity
    entity_set_velocity(paddle, 0, 0);

    paddle->update        = update;
    paddle->out_of_bounds = out_of_bounds;
}

entity_t *paddle_init(aabb_t *field, paddle_identifier_t identifier) {
    entity_t *paddle = entity_init();
    paddle_configure(paddle, field, identifier);
    return paddle;
}
