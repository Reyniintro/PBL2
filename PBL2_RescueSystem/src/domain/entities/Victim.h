#ifndef VICTIM_H
#define VICTIM_H

#include "DomainConstants.h"
#include "RescueEnums.h"

// ============================================================================
// LỚP NẠN NHÂN CƠ SỞ (CÂY PHÂN CẤP NẠN NHÂN / KHÁCH HÀNG)
// ============================================================================
class Victim {
protected:
    char victimID[MAX_ID_LEN];
    Consciousness consciousState;
    bool isBreathing;
    bool isTrapped;
    int victimCount;
    RescueOutcome finalOutcome;

public:
    Victim(const char* id, Consciousness state, bool breathing, bool trapped, int count = 1);
    virtual ~Victim() {}

    virtual bool isHuman() const = 0;
    virtual void formatDetails(char* outBuffer, int maxLen) const = 0;
    virtual void serializeToFile(char* outBuffer, int maxLen) const = 0;

    const char* getVictimID() const { return victimID; }
    Consciousness getConsciousState() const { return consciousState; }
    bool checkBreathing() const { return isBreathing; }
    bool checkTrapped() const { return isTrapped; }
    int getVictimCount() const { return victimCount; }
    RescueOutcome getOutcome() const { return finalOutcome; }

    void updateVitalSigns(Consciousness newState, bool breathing, bool trapped);
    void setFinalOutcome(RescueOutcome outcome) { finalOutcome = outcome; }
};

#endif // VICTIM_H
