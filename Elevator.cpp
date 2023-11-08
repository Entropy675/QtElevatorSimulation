#include "Elevator.h"
#include "QDebug.h"
#include <chrono>
#include <string>

int Elevator::ElevatorId = 1;

Elevator::Elevator(int n)
    : numFloors(n), id(ElevatorId++)
{
    if(DEBUG)
    {
        passengers = id;
        for(int i = 0; i < id; i++)
            buttonsPressed.push_back(i);
    }
}

Elevator::~Elevator()
{

}

void Elevator::updateElevator()
{
    qDebug() << "Update Elevator " << id << " : "  << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
}

void Elevator::moveTofloor(int f)
{
    //if(f > curFloor)
    // emit floorChanged
}

void Elevator::pressButton(int e, int f) // make sure only legal floors are accepted
{
    // now use:
    // void passengerButtonPress(int floor, int elevatorId);
}

void Elevator::helpButtonPressed(int ev)
{
    // contact emergency lines (front desk, 911)
}

void Elevator::emergency()
{
    // go to floor 0 after doors close
}


int Elevator::getId() const
{
    return id;
}

int Elevator::currentFloor() const
{
    return curFloor;
}

int Elevator::numPassengers() const
{
    return passengers;
}

const std::vector<int>& Elevator::getButtonsPressed() const
{
    return buttonsPressed;
}

const Elevator::ElevatorState& Elevator::currentState() const
{
    return state;
}
