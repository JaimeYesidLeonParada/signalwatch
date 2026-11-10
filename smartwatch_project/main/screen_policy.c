#include "screen_policy.h"

void screen_policy_init(screen_policy_t *p, uint32_t timeout_ms, uint32_t now_ms)
{
    p->on = true;
    p->timeout_ms = timeout_ms;
    p->last_activity_ms = now_ms;
}

bool screen_policy_activity(screen_policy_t *p, uint32_t now_ms)
{
    bool woke = !p->on;
    p->on = true;
    p->last_activity_ms = now_ms;
    return woke;
}

bool screen_policy_tick(screen_policy_t *p, uint32_t now_ms)
{
    if (!p->on || p->timeout_ms == 0) {
        return false;
    }
    // La resta sin signo es correcta aunque now_ms haya dado la vuelta.
    if ((uint32_t)(now_ms - p->last_activity_ms) >= p->timeout_ms) {
        p->on = false;
        return true;
    }
    return false;
}

bool screen_policy_is_on(const screen_policy_t *p)
{
    return p->on;
}
