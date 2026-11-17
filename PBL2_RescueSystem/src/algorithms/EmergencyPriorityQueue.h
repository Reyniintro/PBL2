#ifndef EMERGENCY_PRIORITY_QUEUE_H
#define EMERGENCY_PRIORITY_QUEUE_H

#include "CustomDateTime.h"

class EmergencyCase;

#define MAX_HEAP_CAPACITY 200

// ============================================================================
// HÀNG ĐỢI ƯU TIÊN MAX-HEAP (LƯU CÁC CA CHỜ KHI ĐỘI ĐANG BẬN HẾT)
// ============================================================================
class EmergencyPriorityQueue {
private:
    EmergencyCase* heapArray[MAX_HEAP_CAPACITY];
    int currentSize;
    CustomDateTime referenceTime;

    void swapNodes(int i, int j);
    void heapifyUp(int index);
    void heapifyDown(int index);

    EmergencyPriorityQueue(const EmergencyPriorityQueue&) = delete;
    EmergencyPriorityQueue& operator=(const EmergencyPriorityQueue&) = delete;

public:
    EmergencyPriorityQueue();
    ~EmergencyPriorityQueue();

    void updateReferenceTime(const CustomDateTime& now);
    bool push(EmergencyCase* newCase);
    EmergencyCase* popMax();
    bool removeCaseByPointer(EmergencyCase* target);
    EmergencyCase* peekMax() const;
    void rebuildHeapForAging();

    int size() const { return currentSize; }
    bool isEmpty() const { return currentSize == 0; }
    EmergencyCase* getCaseAt(int idx) const {
        return (idx >= 0 && idx < currentSize) ? heapArray[idx] : nullptr;
    }
    void clearAll();
};

#endif // EMERGENCY_PRIORITY_QUEUE_H
