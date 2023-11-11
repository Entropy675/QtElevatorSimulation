#include "Elevator.h"
#include "QDebug.h"
#include <chrono>
#include <string>
#include <QThread>

int Elevator::ElevatorId = 1;

Elevator::Elevator(int n)
    : numFloors(n), id(ElevatorId++)
{
    if(DEBUG_GENVALUES)
    {
        passengers = id;
        for(int i = 1; i <= id; i++)
        {
            buttonsPressed.insert(i);
            moveList.insert(i);
        }
    }


    elevatorUpdateTimer = new QTimer(this);
    connect(elevatorUpdateTimer, &QTimer::timeout, this, &Elevator::updateElevator);
    elevatorUpdateTimer->start(1000); // Emit the signal every 1000 milliseconds (1 second)

}

Elevator::~Elevator()
{
    elevatorUpdateTimer->stop();
    delete elevatorUpdateTimer;
}

// Happens every second - for each floor update
void Elevator::updateElevator()
{
    if(AGGRESSIVE_LOGGING)
        qDebug() << "Elevator Update " << id << " - TIME: "  << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();

    qDebug() << id << " EV state: " << state << " curr goal: " << curGoal << " cur floor: " << curFloor;

    if(callHelp)
    {
        qDebug() << id << ": Attempting to contact front desk... " << helpCounter++;
        if(helpCounter == 6)
        {
            helpCounter = 1;
            qDebug() << id << ": CONNECTING 911 FOR EMERGENCY !!!";
            callHelp = false;
            state = Emergency;
        }
    }

    switch(state)
    {
    case Idle:

        if(moveList.size() != 0)
        {
            curGoal = *moveList.rbegin();
            qDebug() << "movelistsize trigger ev: " << id << " size: " << moveList.size();
        }

        if(curGoal != curFloor)
        {
            if(curGoal > curFloor)
                state = MovingUp;

            if(curGoal < curFloor)
                state = MovingDown;

            return;
        }

        break;
    case MovingUp:
        // Move up, check if we've highest move point/goal. If we have, set it to lowest point change to moving down.

        curFloor += 1;
        emit floorChanged(curFloor, id - 1, true);

        if(moveList.count(curFloor) > 0)
        {
            state = DoorsOpen;
        }

        if(curGoal == curFloor)
        {
            state = DoorsOpen;
            if(moveList.size())
                curGoal = *moveList.rend();
            else
                curGoal = 0;
        }

        break;
    case MovingDown:

        curFloor -= 1;
        emit floorChanged(curFloor, id - 1, false);

        if(moveList.count(curFloor) > 0)
        {
            state = DoorsOpen;
        }

        if(curGoal == curFloor)
        {
            if(curGoal == 0 && moveList.count(0) == 0)
            {
                state = Idle;
                return;
            }

            state = DoorsOpen;
            if(moveList.size())
                curGoal = *moveList.rbegin();
            else
                curGoal = 0;
        }

        break;
    case DoorsOpen:

        emit doorOpened(curFloor, id);
        //moveList.erase(curFloor);
        //buttonsPressed.erase(curFloor);

        QThread::msleep(TIME_ELEVATOR_OPEN*1000);

        state = DoorsClosing;

        break;
    case DoorsClosing:

        emit doorClosed(curFloor, id);

        QThread::msleep(1000);

        state = Idle;

        break;
    case Overload:

        emit overloaded(curFloor, id);

        if(passengers < ELEVATOR_PEOPLE_LIMIT)
        {
            state = DoorsClosing;
        }

        break;
    case Emergency:

        if(!emergencyStart)
        {
            emergencyStart = true;
            emit emergency(curFloor, id);
        }

        if(curFloor != 0)
        {
            curFloor -= 1;
            emit floorChanged(curFloor, id - 1, false);
        }

        break;
    }

}

void Elevator::pressButton(int f) // make sure only legal floors are accepted
{
    if(!isButtonPressed(f))
    {
        buttonsPressed.insert(f);
        moveList.insert(f);
    }
}

void Elevator::helpButtonPressed()
{
    // contact emergency lines (front desk, 911)
    for(int i = 0; i < 3; i++)
        qDebug() << "HELP BUTTON PRESSED: ELEVATOR " << id;
    callHelp = true;
}

void Elevator::emergency()
{
    // go to floor 0 after doors close
    // Emergency request from external sources reached
    qDebug() << "!!! Emergency signal recieved: Elevator " << id;

    state = Emergency;
}

void Elevator::resetEmergency()
{

    qDebug() << id << ": Resetting from emergency...";
    emergencyStart = false;
    state = Idle;
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
    // check curGoal first then
    if(curGoal == 0)
        curGoal = f;
    else
        moveList.insert(f);
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

const std::set<int>& Elevator::getButtonsPressed() const
{
    return buttonsPressed;
}

const Elevator::ElevatorState& Elevator::currentState() const
{
    return state;
}
