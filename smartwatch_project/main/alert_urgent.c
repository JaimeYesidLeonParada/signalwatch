#include "alert_urgent.h"
#include "alert_catalog.h"

bool alert_is_urgent(const char *label, float confidence)
{
    if (label == NULL) return false;
    return alert_catalog_index_of(label) >= 0 && confidence > ALERT_URGENT_CONFIDENCE;
}
