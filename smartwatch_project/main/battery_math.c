#include "battery_math.h"

int battery_percent_from_mv(int mv)
{
    if (mv >= BATTERY_MAX_VOLTAGE) return 100;
    if (mv <= BATTERY_MIN_VOLTAGE) return 0;
    return ((mv - BATTERY_MIN_VOLTAGE) * 100) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE);
}

int battery_filter_update(battery_filter_t *filter, int pct)
{
    if (!filter->has_value) {
        filter->value = (float)pct;
        filter->has_value = true;
    } else {
        filter->value = filter->value * 0.9f + (float)pct * 0.1f;
    }

    return (int)(filter->value + 0.5f);
}

bool battery_is_charging(int pct)
{
    return pct >= CHARGING_PCT_THRESHOLD;
}
