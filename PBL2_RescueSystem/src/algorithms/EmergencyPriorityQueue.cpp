#include "EmergencyPriorityQueue.h"
#include "EmergencyCase.h"

// ============================================================================
// THỰC THI HÀNG ĐỢI ƯU TIÊN MAX-HEAP & AGING
// ============================================================================
EmergencyPriorityQueue::EmergencyPriorityQueue()
    : currentSize(0), referenceTime(10, 11, 2026, 8, 30, 0) {
    for (int i = 0; i < MAX_HEAP_CAPACITY; ++i) {
        heapArray[i] = nullptr;
    }
}

EmergencyPriorityQueue::~EmergencyPriorityQueue() {
    clearAll();
}

void EmergencyPriorityQueue::clearAll() {
    for (int i = 0; i < currentSize; ++i) {
        if (heapArray[i] != nullptr) {
            delete heapArray[i];
            heapArray[i] = nullptr;
        }
    }
    currentSize = 0;
}

void EmergencyPriorityQueue::swapNodes(int i, int j) {
    EmergencyCase* temp = heapArray[i];
    heapArray[i] = heapArray[j];
    heapArray[j] = temp;
}

void EmergencyPriorityQueue::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        double currScore = heapArray[index]->calculateDynamicPriority(referenceTime);
        double parentScore = heapArray[parent]->calculateDynamicPriority(referenceTime);
        if (currScore > parentScore) {
            swapNodes(index, parent);
            index = parent;
        } else {
            break;
        }
    }
}

void EmergencyPriorityQueue::heapifyDown(int index) {
    while (true) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        double largestScore = heapArray[largest]->calculateDynamicPriority(referenceTime);

        if (left < currentSize) {
            double leftScore = heapArray[left]->calculateDynamicPriority(referenceTime);
            if (leftScore > largestScore) {
                largest = left;
                largestScore = leftScore;
            }
        }
        if (right < currentSize) {
            double rightScore = heapArray[right]->calculateDynamicPriority(referenceTime);
            if (rightScore > largestScore) {
                largest = right;
            }
        }

        if (largest != index) {
            swapNodes(index, largest);
            index = largest;
        } else {
            break;
        }
    }
}

void EmergencyPriorityQueue::updateReferenceTime(const CustomDateTime& now) {
    referenceTime = now;
    rebuildHeapForAging();
}

void EmergencyPriorityQueue::rebuildHeapForAging() {
    for (int i = (currentSize / 2) - 1; i >= 0; --i) {
        heapifyDown(i);
    }
}

bool EmergencyPriorityQueue::push(EmergencyCase* newCase) {
    if (!newCase || currentSize >= MAX_HEAP_CAPACITY) return false;
    heapArray[currentSize] = newCase;
    heapifyUp(currentSize);
    currentSize++;
    return true;
}

EmergencyCase* EmergencyPriorityQueue::popMax() {
    if (currentSize <= 0) return nullptr;
    EmergencyCase* topCase = heapArray[0];
    currentSize--;
    if (currentSize > 0) {
        heapArray[0] = heapArray[currentSize];
        heapArray[currentSize] = nullptr;
        heapifyDown(0);
    } else {
        heapArray[0] = nullptr;
    }
    return topCase;
}

bool EmergencyPriorityQueue::removeCaseByPointer(EmergencyCase* target) {
    if (!target || currentSize <= 0) return false;
    int targetIdx = -1;
    for (int i = 0; i < currentSize; ++i) {
        if (heapArray[i] == target) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx == -1) return false;

    currentSize--;
    if (targetIdx < currentSize) {
        heapArray[targetIdx] = heapArray[currentSize];
        heapArray[currentSize] = nullptr;
        heapifyUp(targetIdx);
        heapifyDown(targetIdx);
    } else {
        heapArray[targetIdx] = nullptr;
    }
    return true;
}

EmergencyCase* EmergencyPriorityQueue::peekMax() const {
    return (currentSize > 0) ? heapArray[0] : nullptr;
}
