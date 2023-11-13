#include "Elevator.h"
#include "QDebug.h"
#include <chrono>
#include <string>

int Elevator::ElevatorId = 1;

Elevator::Elevator(int n)
    : numFloors(n), id(ElevatorId++)
{
    elevatorUpdateTimer = new QTimer(this);
    connect(elevatorUpdateTimer, &QTimer::timeout, this, &Elevator::updateElevator);
    elevatorUpdateTimer->start(ELEVATOR_SPEED_MS);

    delayTimer = new QTimer(this);
    delayTimer->setSingleShot(true); // Run only once
    connect(delayTimer, &QTimer::timeout, this, &Elevator::restartTimer);
}

Elevator::~Elevator()
{
    elevatorUpdateTimer->stop();
    delete elevatorUpdateTimer;

    delayTimer->stop();
    delete delayTimer;
}

void Elevator::restartTimer()
{
    elevatorUpdateTimer->start(ELEVATOR_SPEED_MS);
}

void Elevator::moveTofloor(int e, int f)
{
    if(e != id && e != -1)
        return;
    moveList.insert(f);

    emit updateDisplays();
}

void Elevator::addPassengers(int e, int f, int x)
{
    if(e != id && e != -1)
        return;
    passengers += x;
    doorsBlockedRecently = ELEVATOR_BLOCK_DOOR_TIME_SENSITIVITY;

    emit updateDisplays();
}

void Elevator::removePassengers(int e, int f, int x)
{
    if(e != id && e != -1)
        return;
    passengers -= x;
    doorsBlockedRecently = ELEVATOR_BLOCK_DOOR_TIME_SENSITIVITY;
    if(passengers < 0)
        passengers = 0;

    emit updateDisplays();
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

bool Elevator::lastDirMovingUp() const
{
    return movingUp;
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
    std::set<int> combineList(moveList.begin(), moveList.end());
    combineList.insert(buttonsPressed.begin(), buttonsPressed.end());

    if(AGGRESSIVE_LOGGING)
    {
        std::string s = " EV nums: ";
        for(int i : combineList)
            s += std::to_string(i) + " ";
        if(state != Idle)
            qDebug() << id << " EV state: " << state << " curr goal: " << curGoal << " cur floor: " << curFloor << s;
    }

    if(callHelp)
    {
        qDebug() << "HELP CALL [Elevator: " << id << "]: Attempting to contact front desk... " << helpCounter++;
        if(helpCounter > HELP_COUNTER)
        {
            helpCounter = 1;
            qDebug() << id << ": CONNECTING 911 FOR EMERGENCY !!!";
            callHelp = false;
            state = Emergency;
        }
    }

    if(doorsBlockedRecently)
        doorsBlockedRecently--;

    switch(state)
    {
    case Idle:

        if(combineList.size() != 0)
        {
            curGoal = *combineList.rbegin();
            qDebug() << "combineList trigger ev: " << id << " size: " << combineList.size();
        }

        if(curGoal == curFloor)
        {
            if(combineList.count(curFloor) > 0)
            {
                state = DoorsOpen;
                return;
            }
            curGoal = SAFE_FLOOR;
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
        movingUp = true;
        curFloor += 1;
        emit floorChanged(curFloor, id, true);

        if(combineList.empty()) // go to the largest place in list
            curGoal = SAFE_FLOOR;
        else
        {
            std::set<int>::reverse_iterator rit = combineList.rbegin();
            curGoal = *rit;
        }

        if(combineList.count(curFloor) > 0)
        {
            state = DoorsOpen;
            return;
        }

        if(curGoal == curFloor)
        {
            if(curGoal == SAFE_FLOOR && combineList.count(SAFE_FLOOR) == 0)
            {
                state = Idle;
                return;
            }

            if(combineList.size())
                curGoal = *combineList.rend();
            else
                curGoal = SAFE_FLOOR;
            return;
        }

        if(curGoal > curFloor)
            state = MovingUp;

        if(curGoal < curFloor)
            state = MovingDown;

        break;
    case MovingDown:
        movingUp = false;
        curFloor -= 1;
        emit floorChanged(curFloor, id, false);

        if(combineList.empty()) // go to the smallest place in list
            curGoal = SAFE_FLOOR;
        else
            curGoal = *combineList.rbegin();

        if(combineList.count(curFloor) > 0)
        {
            state = DoorsOpen;
            return;
        }

        if(curGoal == curFloor)
        {
            if(curGoal == SAFE_FLOOR && combineList.count(SAFE_FLOOR) == 0)
            {
                state = Idle;
                return;
            }

            if(combineList.size())
                curGoal = *combineList.rbegin();
            else
                curGoal = SAFE_FLOOR;
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

        elevatorUpdateTimer->stop();
        delayTimer->start(TIME_ELEVATOR_OPEN*1000);

        if(passengers >= ELEVATOR_PEOPLE_LIMIT)
        {
            state = Overload;
            return;
        }
        state = DoorsClosing;

        break;
    case DoorsClosing:
        if(doorsBlockedRecently)
        {
            state = DoorsOpen;
            emit doorBlocked(curFloor, id);
            elevatorUpdateTimer->stop();
            delayTimer->start(1000);
            doorsBlockedRecently = 0;
            return;
        }
        emit doorClosed(curFloor, id);

        elevatorUpdateTimer->stop();
        delayTimer->start(1000);

        emit floorChanged(curFloor, id, false);
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

        if(!emergencyFloorReached)
        {
            emit floorChanged(curFloor, id, false);
            emit emergencyOnBoard(curFloor, id);
            emergencyFloorReached = true;
        }


        if(curFloor != SAFE_FLOOR)
        {
            if(SAFE_FLOOR > curFloor)
                curFloor += 1;
            else
                curFloor -= 1;
            emit floorChanged(curFloor, id, false);
        }

        break;
    }

    emit updateDisplays();
}

void Elevator::pressButton(int ev, int fb) // make sure only legal floors are accepted
{
    if(ev != id)
        return;
    buttonsPressed.insert(fb);
    emit updateDisplays();
}

void Elevator::unpressButton(int ev, int fb)
{
    if(ev != id)
        return;
    buttonsPressed.erase(fb);
    emit updateDisplays();
}

void Elevator::helpButtonPressed(int ev)
{
    qDebug() << "HELP BUTTON: ELEVATOR " << ev << " id: " << id;
    if(!(ev == -1 || ev == id))
        return;
    // contact emergency lines (front desk, 911)
    for(int i = 0; i < 3; i++)
        qDebug() << "HELP BUTTON PRESSED: ELEVATOR " << id;
    callHelp = true;
}

void Elevator::emergency(int ev)
{
    if(ev != id && ev != -1)
        return;

    // go to floor 0 after doors close
    // Emergency request from external sources reached
    qDebug() << "!!! Emergency signal recieved: Elevator " << id;
    if(state == Emergency)
        return;

    //qDebug() << "!!! Pass 2 (Success): Emergency signal recieved: Elevator " << id;
    if(state != Idle && state != MovingUp && state != MovingDown)
    {
        state = DoorsClosing;
        emit doorClosed(curFloor, id);

        elevatorUpdateTimer->stop();
        delayTimer->start(TIME_ELEVATOR_OPEN*1000);
    }

    emit emergencyOnBoard(curFloor, id);
    state = Emergency;
}


void Elevator::resetEmergencyInElevator(int ev)
{
    if(ev == -1 || ev == id)
    {
        state = Idle;
        buttonsPressed.clear();
        moveList.clear();
        curGoal = SAFE_FLOOR;
        emergencyFloorReached = false;
        emit emergencyOnBoard(curFloor, id);
    }
}
