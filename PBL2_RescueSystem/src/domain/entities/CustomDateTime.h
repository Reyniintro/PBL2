#ifndef CUSTOM_DATE_TIME_H
#define CUSTOM_DATE_TIME_H

#include <cstdio>

// ============================================================================
// CẤU TRÚC THỜI GIAN TỰ CÀI ĐẶT
// ============================================================================
struct CustomDateTime {
    int day, month, year;
    int hour, minute, second;

    CustomDateTime(int d = 10, int m = 11, int y = 2026, int h = 8, int min = 0, int s = 0)
        : day(d), month(m), year(y), hour(h), minute(min), second(s) {}

    long toTotalMinutes() const;

    long operator-(const CustomDateTime& other) const {
        return this->toTotalMinutes() - other.toTotalMinutes();
    }

    void formatDate(char* buffer, int maxLen) const {
        snprintf(buffer, maxLen, "%02d/%02d/%04d", day, month, year);
    }

    void formatDateTime(char* buffer, int maxLen) const {
        snprintf(buffer, maxLen, "%02d/%02d/%04d %02d:%02d:%02d", day, month, year, hour, minute, second);
    }
};

#endif // CUSTOM_DATE_TIME_H
