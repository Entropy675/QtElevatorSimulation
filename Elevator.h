#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QObject>

#include "defs.h"
#include <vector>

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
        DoorsOpening,
        DoorsClosing,
        Emergency // Add other states as needed
    };
	
    Elevator(int nfloors = NUM_FLOORS);
    ~Elevator();

	int getId() const;
	int currentFloor() const;
	int numPassengers() const;
	const std::vector<int>& getButtonsPressed() const;
	const ElevatorState& currentState() const;
	
signals:
    void floorChanged(int floor, int ev);
    void doorOpened(int ev); // sends its own id so that controller knows
    void doorClosed(int ev);

public slots:
    void updateElevator();
    void pressButton(int ev, int flr);
    void helpButtonPressed(int ev);
    void emergency();

private:
    static int ElevatorId;
    const int id;
    const int numFloors;
    float openForSecs = TIME_ELEVATOR_OPEN;

    std::vector<int> buttonsPressed;

    int passengers = 0;
    int curFloor = 0;
    ElevatorState state = Idle;

	// Private internals, whatever funcs you have to do to implement the interface
	// next 2 add or remove delay that the door is open for:

    void moveTofloor(int);
	void closeDoor(); 
	void openDoor();
};


#endif