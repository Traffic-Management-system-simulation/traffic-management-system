#include "../include/Vehicle.hpp"
#include <cassert>
#include <iostream>

void testProbabilityZero() {
    VehicleManager manager;
    Approach approach = Approach::NORTH;

    bool generated = manager.generateVehicle(approach, 0.0);

    assert(!generated);
    std::cout << "PASS: Probability 0.0 generates no vehicle\n";
}

void testProbabilityOne() {
    VehicleManager manager;
    Approach approach = Approach::NORTH;

    bool generated = manager.generateVehicle(approach, 1.0);

    assert(generated);
    std::cout << "PASS: Probability 1.0 always generates a vehicle\n";
}

void testProbabilityPointOne() {
    VehicleManager manager;
    int generatedCount = 0;
    const int trials = 1000;

    for (int i = 0; i < trials; ++i) {
        Approach approach = Approach::NORTH;
        if (manager.generateVehicle(approach, 0.1)) {
            ++generatedCount;
        }
    }

    double rate = static_cast<double>(generatedCount) / trials;
    assert(rate > 0.05 && rate < 0.15);

    std::cout << "PASS: Probability 0.1 generated rate = "
              << rate << "\n";
}

void testInvalidProbability() {
    VehicleManager manager;
    Approach approach = Approach::NORTH;

    assert(!manager.generateVehicle(approach, -0.1));
    assert(!manager.generateVehicle(approach, 1.1));

    std::cout << "PASS: Invalid probabilities are rejected\n";
}

void testUniqueVehicleIds() {
    VehicleManager manager;

    Vehicle first = manager.createVehicle(Approach::NORTH, 1);
    Vehicle second = manager.createVehicle(Approach::SOUTH, 2);
    Vehicle third = manager.createVehicle(Approach::EAST, 3);

    assert(first.id != second.id);
    assert(second.id != third.id);
    assert(first.id < second.id);
    assert(second.id < third.id);

    std::cout << "PASS: Vehicle IDs are unique and increasing\n";
}

void testVehicleCreation() {
    VehicleManager manager;

    Vehicle vehicle = manager.createVehicle(Approach::WEST, 10);

    assert(vehicle.approach == Approach::WEST);
    assert(vehicle.entryTick == 10);
    assert(vehicle.id > 0);

    std::cout << "PASS: Vehicle is created with correct approach and entry tick\n";
}

int main() {
    std::cout << "Testing Vehicle Generation\n\n";

    testProbabilityZero();
    testProbabilityOne();
    testProbabilityPointOne();
    testInvalidProbability();
    testUniqueVehicleIds();
    testVehicleCreation();

    std::cout << "\nAll vehicle generation tests passed.\n";

    return 0;
}