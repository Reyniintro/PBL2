#include "Victim.h"
#include <cstring>

static void safeCopyStr(char* dest, const char* src, int maxLen) {
    if (!dest || maxLen <= 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, maxLen - 1);
    dest[maxLen - 1] = '\0';
}

// ============================================================================
// THỰC THI LỚP NẠN NHÂN CƠ SỞ
// ============================================================================
Victim::Victim(const char* id, Consciousness state, bool breathing, bool trapped, int count)
    : consciousState(state), isBreathing(breathing), isTrapped(trapped),
    victimCount(count > 0 ? count : 1), finalOutcome(OUTCOME_NONE) {
    safeCopyStr(victimID, id, MAX_ID_LEN);
}

void Victim::updateVitalSigns(Consciousness newState, bool breathing, bool trapped) {
    consciousState = newState;
    isBreathing = breathing;
    isTrapped = trapped;
}
