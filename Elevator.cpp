#include "Elevator.h"
#include "QDebug.h"
#include <chrono>
#include <string>

int Elevator::ElevatorId = 1;

Elevator::Elevator(int n)
    : numFloors(n), id(ElevatorId++)
{
    if(DEBUG_GENVALUES)
    {
        passengers = id;
        for(int i = 1; i <= id; i++)
            buttonsPressed.push_back(i);
    }
}

Elevator::~Elevator()
{
    //
}

void Elevator::updateElevator()
{
    if(AGGRESSIVE_LOGGING)
        qDebug() << "Update Elevator " << id << " : "  << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();


    switch(state)
    {
    case Idle:

        break;
    case MovingUp:

        break;
    case MovingDown:

        break;
    case DoorsOpen:

        break;
    case DoorsClosing:

        break;
    case Overload:

        break;
    case Help:

        break;
    case Emergency:

        break;
    }

}

void Elevator::pressButton(int f) // make sure only legal floors are accepted
{
    if(!isButtonPressed(f))
        buttonsPressed.push_back(f);
}

void Elevator::helpButtonPressed()
{
    // contact emergency lines (front desk, 911)
    for(int i = 0; i < 5; i++)
        qDebug() << "!!! HELP BUTTON PRESSED: ELEVATOR " << id;

    state = Help;
}

void Elevator::emergency()
{
    // go to floor 0 after doors close
    // Emergency request from external sources reached
    qDebug() << "!!! Emergency signal recieved: Elevator " << id;

    state = Help;
}

bool Elevator::isButtonPressed(int b) const
{
    for(int i : buttonsPressed)
        if(i == b)
            return true;
    return false;
}

void Elevator::moveTofloor(int f)
{
    //if(f > curFloor)
    // emit floorChanged
}

void Elevator::closeDoor()
{
    //
}

void Elevator::openDoor()
{
    //
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
