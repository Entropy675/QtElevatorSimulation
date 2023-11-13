#ifndef ELEVATORCONTROLLER_H
#define ELEVATORCONTROLLER_H

#include <QObject>
#include <QScrollArea>
#include <QComboBox>
#include <QTimer>
#include <QMainWindow>
#include <QThread>

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
    void pressButton(int ev, int fb);
    void unpressButton(int ev, int fb);
    void addElevatorPassengers(int ev, int flr, int num);
    void removeElevatorPassengers(int ev, int flr, int num);
    void moveElevatorToFloor(int ev, int flr);
    void sendRequestToElevator(int ev, int flr);
    void helpButton(int elevatorId);
    void buildingEmergency(int ev);
    void resetEmergency(int ev);

public slots:
    // recieve signals from floors, relay to EVs
	void buttonPressedUp(int floor);
    void buttonPressedDown(int floor);

    // recieves from every elevator, handles UI
    void elevatorFloorChanged(int floor, int ev, bool up);
    void doorOpened(int flr, int ev);
    void doorClosed(int flr, int ev);
    void doorBlocked(int flr, int ev);
    void overloaded(int flr, int ev);
    void emergency(int flr, int ev);

    // for rescaling the elevator box...
    void handleScreenResized(int w, int h);

private slots:
    // buttons
    void buttonElevatorSubmit();
    void buttonElevatorHelp();
    void buttonPlaceOnFloor();
    void buttonMoveToElevator();
    void buttonLeaveElevator();
    void add10ToEachFloor();
    void triggerBuildingEmergency();
    void resetAllElevatorsEmergency();

    // select boxes
    void elevatorSelected(int index);
    void floorSelected(int index);
    void moveComboBoxChange(int index);
    void moveLeaveElevatorBoxChange(int index);

    // constant scan on timer
    void scanRequestTree();

    // update displays for ev changes...
    void updateDisplays();


private:
    std::vector<Floor*> floors;
    std::vector<Elevator*> elevators;
    std::vector<QThread*> threads;

	// if there is no immediate available elevator, put request in this list as a FloorDirection
	std::priority_queue<FloorDirection> earliestRequestTree;
    // this list should be periodically checked and if there are any values
    // and available elevators, pop from the root and work all the values out

    void controlMoveButtonActivated(Elevator* availableEv = nullptr);
    void handleFlrPressed(FloorDirection);

    bool initialResize = true;
    int widthOfInputConst;

    // timers
    QTimer* requestScanTimer;

    // ui elements
    Ui::MainWindow* ui;
    QGridLayout* elevatorGridLayout; // we generate this
};


#endif
