#ifndef ELEVATORCONTROLLER_H
#define ELEVATORCONTROLLER_H

#include <QObject>
#include <QScrollArea>
#include <QComboBox>
#include <QTimer>
#include <QMainWindow>

#include "./ui_mainwindow.h"

#include "defs.h"
#include "Floor.h"
#include "Elevator.h"

#include <vector>
#include <tuple>
#include <queue>
#include <chrono>

class ElevatorController : public QObject
{
    Q_OBJECT

	struct  FloorDirection
	{
		int time;
		int num = 0;
		bool up = true;
		
		FloorDirection(int n, bool u) : num(n), up(u) 
		{
			time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
		}
		
		// comparison operator for the priority queue, prioritize earliest request
		bool operator<(const FloorDirection& other) const
		{
			return time < other.time;
        }
    };
	
public:
    ElevatorController(Ui::MainWindow* ui, int numElevators = NUM_ELEVATORS, int numFloors = NUM_FLOORS);
	~ElevatorController(); // clean up floors

signals:
    void sendRequestToElevator(int ev, int flr);
    void updateElevators(); // constant signal on timer
    void helpButton(int elevatorId);
    void emergency();

public slots:
    // recieve signals from floors
	void buttonPressedUp(int floor);
    void buttonPressedDown(int floor);

    // recieves from every elevator
    void elevatorFloorChanged(int floor, int ev);
    void doorOpened(int ev);
    void doorClosed(int ev);

private slots:
    // buttons
    void buttonElevatorSubmit();
    void buttonPlaceOnFloor();
    void buttonMoveToElevator();

    // spin boxes
    void spinBoxPlaceChange();
    void spinBoxMoveChange();

    // select boxes
    void elevatorSelected(int index);
    void floorSelected(int index);

    // constant scan on timer
    void scanRequestTree();

private:
    std::vector<Floor*> floors;
    std::vector<Elevator*> elevators;
	
	// if there is no immediate available elevator, put request in this list as a FloorDirection
	std::priority_queue<FloorDirection> earliestRequestTree;
    // this list should be periodically checked and if there are any values
    // and available elevators, pop from the root and work all the values out

    void controlMoveButtonActivated();

    // timers
    QTimer* requestScanTimer;
    QTimer* elevatorUpdateTimer;

    // ui elements
    Ui::MainWindow* ui;
};


#endif
