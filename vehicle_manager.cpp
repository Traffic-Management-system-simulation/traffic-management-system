#include <iostream>
#include <string>
#include <random>

using namespace std;

enum class Lane {
    NORTH,
    SOUTH,
    EAST,
    WEST
};

struct Vehicle {
    int id;
    string type;
    Lane lane;
    bool emergency;
};

int nextVehicleId = 1;

Vehicle createVehicle(const string& type, Lane lane, bool emergency) {
    Vehicle vehicle;

    vehicle.id = nextVehicleId++;
    vehicle.type = type;
    vehicle.lane = lane;
    vehicle.emergency = emergency;

    return vehicle;
}

bool shouldGenerateVehicle(double probability) {
    if (probability < 0.0 || probability > 1.0) {
        return false;
    }

    static random_device rd;
    static mt19937 generator(rd());
    uniform_real_distribution<double> distribution(0.0, 1.0);

    return distribution(generator) < probability;
}

Vehicle generateVehicle(Lane lane, double generationProbability) {
    if (!shouldGenerateVehicle(generationProbability)) {
        return {-1, "", Lane::NORTH, false};
    }

    static const string vehicleTypes[] = {
        "Car",
        "Bus",
        "Truck"
    };

    static random_device rd;
    static mt19937 generator(rd());
    uniform_int_distribution<int> typeDistribution(0, 2);

    string type = vehicleTypes[typeDistribution(generator)];

    return createVehicle(type, lane, false);
}