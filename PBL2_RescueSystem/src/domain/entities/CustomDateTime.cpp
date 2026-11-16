#include "CustomDateTime.h"

// ============================================================================
// THỰC THI CUSTOM DATETIME
// ============================================================================
long CustomDateTime::toTotalMinutes() const {
    long totalDays = (long)year * 365 + (long)month * 30 + day;
    return totalDays * 1440 + hour * 60 + minute;
}
