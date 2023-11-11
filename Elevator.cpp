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

void Elevator::moveTofloor(int f)
{
    // check curGoal first then
    if(curGoal == 0)
        curGoal = f;
    else
        moveList.insert(f);
}

void Elevator::addPassengers(int x)
{
    passengers += x;
}

void Elevator::removePassengers(int x)
{
    passengers -= x;
    if(passengers < 0)
        passengers = 0;
}

void Elevator::resetEmergency()
{

    qDebug() << id << ": Resetting from emergency...";
    state = Idle;
}

int Elevator::getId() const
{
    return id;
}
int Elevator::getNumFloorsReserved() const
{
    return moveList.size();
}

int Elevator::currentFloor() const
{
    return curFloor;
}

int Elevator::numPassengers() const
{
    return passengers;
}

bool Elevator::isButtonPressed(int b) const
{
    for(int i : buttonsPressed)
        if(i == b)
            return true;
    return false;
}

const std::set<int>& Elevator::getButtonsPressed() const
{
    return buttonsPressed;
}

const Elevator::ElevatorState& Elevator::currentState() const
{
    return state;
}

// Happens every second - for each floor update
void Elevator::updateElevator()
{
    if(AGGRESSIVE_LOGGING)
    {
        //qDebug() << "Elevator Update " << id << " - TIME: "  << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();

        std::string s = " EV nums: ";
        for(int i : moveList)
            s += std::to_string(i) + " ";
        qDebug() << id << " EV state: " << state << " curr goal: " << curGoal << " cur floor: " << curFloor << s;
    }

    if(callHelp)
    {
        qDebug() << id << ": Attempting to contact front desk... " << helpCounter++;
        if(helpCounter > 5)
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

        if(curGoal == curFloor)
        {
            if(moveList.count(curFloor) > 0)
            {
                state = DoorsOpen;
                return;
            }
            curGoal = 0;
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
            return;
        }

        if(curGoal == curFloor)
        {
            state = DoorsOpen;
            if(moveList.size())
                curGoal = *moveList.rend();
            else
                curGoal = 0;
            return;
        }

        if(curGoal > curFloor)
            state = MovingUp;

        if(curGoal < curFloor)
            state = MovingDown;

        break;
    case MovingDown:

        curFloor -= 1;
        emit floorChanged(curFloor, id - 1, false);

        if(moveList.count(curFloor) > 0)
        {
            state = DoorsOpen;
            return;
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
            return;
        }

        if(curGoal > curFloor)
            state = MovingUp;

        if(curGoal < curFloor)
            state = MovingDown;

        break;
    case DoorsOpen:

        emit doorOpened(curFloor, id);
        moveList.erase(curFloor);
        buttonsPressed.erase(curFloor);

        QThread::msleep(TIME_ELEVATOR_OPEN*1000);

        if(passengers >= ELEVATOR_PEOPLE_LIMIT)
        {
            state = Overload;
            return;
        }
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
            qDebug() << "EV: " << id << " ... A loud beeping noise is played as an overloaded elevator's doors close ... ";
            state = DoorsClosing;
        }

        break;
    case Emergency:

        emit emergencyOnBoard(curFloor, id);


        if(curFloor != 0)
        {
            curFloor -= 1;
            emit floorChanged(curFloor, id - 1, false);
        }


        QThread::msleep(100);

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
    if(state != Idle)
    {
        state = DoorsClosing;
        emit doorClosed(curFloor, id);
    }

    emit emergencyOnBoard(curFloor, id);
    state = Emergency;
}


void Elevator::resetEmergencyInElevator(int ev)
{
    if(ev == -1 || ev == (id - 1))
    {
        state = Idle;
        buttonsPressed.clear();
        moveList.clear();
        curGoal = 0;
        emit emergencyOnBoard(curFloor, id);
    }
}
