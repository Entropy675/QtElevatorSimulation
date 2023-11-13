#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QObject>
#include <QTimer>

#include "defs.h"
#include <vector>
#include <mutex>
#include <set>

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

	int getId() const;
    int getNumFloorsReserved() const;
	int currentFloor() const;
    int numPassengers() const;
    bool lastDirMovingUp() const;
    bool isButtonPressed(int) const;
    const std::set<int>& getButtonsPressed() const;
    const ElevatorState& currentState() const;
	
signals:
    void floorChanged(int floor, int ev, bool up);
    void doorOpened(int floor, int ev); // sends its own id so that controller knows
    void doorClosed(int floor, int ev);
    void doorBlocked(int floor, int ev);
    void overloaded(int floor, int ev);
    void emergencyOnBoard(int floor, int ev);
    void updateDisplays();

public slots:
    void updateElevator();
    void pressButton(int ev, int fb);
    void unpressButton(int ev, int fb);
    void moveTofloor(int ev, int fb);
    void addPassengers(int ev, int flr, int num);
    void removePassengers(int ev, int flr, int num);
    void helpButtonPressed(int elevatorId);
    void emergency(int ev);
    void resetEmergencyInElevator(int ev);
    void restartTimer();


private:
    static int ElevatorId;
    const int id;
    const int numFloors;
    float openForSecs = TIME_ELEVATOR_OPEN;

    bool emergencyFloorReached = false;
    bool callHelp = false;
    bool movingUp = true;
    int helpCounter = 1;
    int doorsBlockedRecently = 0;

    int passengers = 0;
    int curFloor = 0;
    int curGoal = SAFE_FLOOR; // will go to

    // two seperate ones, one for the displayed ones other for commands from ec
    std::set<int> buttonsPressed;
    std::set<int> moveList;

    QTimer* delayTimer;
    QTimer* elevatorUpdateTimer;
    ElevatorState state = MovingUp;
};


#endif
