#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QObject>
#include <QTimer>

#include "defs.h"
#include <vector>
#include <set>

// connect with something like:
// QObject::connect(&elevator, SIGNAL(doorOpened()), &passenger, SLOT(openDoor()));

class Elevator : public QObject
{
	Q_OBJECT
	
public:
    enum ElevatorState 
	{
        Idle,
        MovingUp,
        MovingDown,
        DoorsOpen,
        DoorsClosing,
        Overload,
        Emergency // Add other states as needed
    };
	
    Elevator(int nfloors = NUM_FLOORS);
    ~Elevator();

    void moveTofloor(int);
    void addPassengers(int);
    void resetEmergency();

	int getId() const;
    int getNumFloorsReserved() const;
	int currentFloor() const;
    int numPassengers() const;
    bool isButtonPressed(int) const;
    const std::set<int>& getButtonsPressed() const;
	const ElevatorState& currentState() const;
	
signals:
    void floorChanged(int floor, int ev, bool up);
    void doorOpened(int floor, int ev); // sends its own id so that controller knows
    void doorClosed(int floor, int ev);
    void overloaded(int floor, int ev);
    void emergencyOnBoard(int floor, int ev);

public slots:
    void updateElevator();
    void pressButton(int flr);
    void helpButtonPressed();
    void emergency();

private:
    static int ElevatorId;
    const int id;
    const int numFloors;
    float openForSecs = TIME_ELEVATOR_OPEN;

    bool emergencyStart = false;
    bool callHelp = false;
    int helpCounter = 1;

    int passengers = 0;
    int curFloor = 0;
    int curGoal = 0; // will go to

    std::set<int> buttonsPressed;
    std::set<int> moveList;

    QTimer* elevatorUpdateTimer;
    ElevatorState state = Idle;
};


#endif
