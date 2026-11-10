#include <cassert>
#include <iostream>
#include "QueueManager.hpp"

void testEmptyQueue() {
    QueueManager q;
    Vehicle out{0, Approach::NORTH, 0};

    assert(q.getQueueSize(Approach::NORTH) == 0);
    assert(!q.dequeueVehicle(Approach::NORTH, out));
    assert(q.getTotalWaitingCount() == 0);
}

void testEnqueueAndDequeue() {
    QueueManager q;
    Vehicle v{1, Approach::NORTH, 10};
    Vehicle out{0, Approach::NORTH, 0};

    q.enqueueVehicle(v);

    assert(q.getQueueSize(Approach::NORTH) == 1);
    assert(q.dequeueVehicle(Approach::NORTH, out));
    assert(out.id == 1);
    assert(q.getQueueSize(Approach::NORTH) == 0);
}

void testFIFOOrder() {
    QueueManager q;
    Vehicle out{0, Approach::NORTH, 0};

    q.enqueueVehicle(Vehicle{1, Approach::NORTH, 1});
    q.enqueueVehicle(Vehicle{2, Approach::NORTH, 2});
    q.enqueueVehicle(Vehicle{3, Approach::NORTH, 3});

    assert(q.dequeueVehicle(Approach::NORTH, out) && out.id == 1);
    assert(q.dequeueVehicle(Approach::NORTH, out) && out.id == 2);
    assert(q.dequeueVehicle(Approach::NORTH, out) && out.id == 3);
    assert(!q.dequeueVehicle(Approach::NORTH, out));
}

void testIndependentApproaches() {
    QueueManager q;
    Vehicle out{0, Approach::NORTH, 0};

    q.enqueueVehicle(Vehicle{1, Approach::NORTH, 1});
    q.enqueueVehicle(Vehicle{2, Approach::SOUTH, 2});
    q.enqueueVehicle(Vehicle{3, Approach::EAST, 3});
    q.enqueueVehicle(Vehicle{4, Approach::WEST, 4});

    assert(q.getTotalWaitingCount() == 4);
    assert(q.dequeueVehicle(Approach::EAST, out));
    assert(out.id == 3);
    assert(q.getQueueSize(Approach::NORTH) == 1);
    assert(q.getQueueSize(Approach::SOUTH) == 1);
    assert(q.getQueueSize(Approach::WEST) == 1);
}

void testRepeatedOperationsAndReset() {
    QueueManager q;
    Vehicle out{0, Approach::WEST, 0};

    for (int i = 1; i <= 100; ++i) {
        q.enqueueVehicle(Vehicle{i, Approach::WEST, i});
    }

    for (int i = 1; i <= 100; ++i) {
        assert(q.dequeueVehicle(Approach::WEST, out));
        assert(out.id == i);
    }

    assert(q.getTotalWaitingCount() == 0);
    q.enqueueVehicle(Vehicle{101, Approach::WEST, 101});
    q.reset();

    assert(q.getTotalWaitingCount() == 0);
    assert(!q.dequeueVehicle(Approach::WEST, out));
}

int main() {
    testEmptyQueue();
    testEnqueueAndDequeue();
    testFIFOOrder();
    testIndependentApproaches();
    testRepeatedOperationsAndReset();

    std::cout << "All 5 queue tests passed." << std::endl;
    return 0;
}
