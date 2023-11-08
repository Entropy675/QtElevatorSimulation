#include "ElevatorController.h"
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>
#include <QChar>

ElevatorController::ElevatorController(Ui::MainWindow* u, int numElevators, int numFloors)
{
    // create/populate ui elements
    ui = u;
    QStringList evs;
    for (int elevator = 1; elevator <= numElevators; ++elevator)
    {
        evs << QString("Elevator: %1").arg(elevator);
    }
    ui->comboElevatorBox->addItems(evs);
    connect(ui->comboElevatorBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ElevatorController::elevatorSelected);

    evs.clear();

    for (int floorc = 1; floorc <= numFloors; ++floorc)
    {
        evs << QString("Floor: %1").arg(floorc);
    }
    ui->comboFloorBox->addItems(evs);
    connect(ui->comboFloorBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ElevatorController::floorSelected);

    // connect buttons
    connect(ui->pushElevatorButton, &QPushButton::clicked, this, &ElevatorController::buttonElevatorSubmit);
    //connect(ui->)

    // widget to hold the grid layout
    elevatorWidget = new QWidget;
    ui->elevatorScrollArea->setWidget(elevatorWidget); // Set the content widget of the ElevatorController

    elevatorGridLayout = new QGridLayout(elevatorWidget);
    elevatorWidget->setLayout(elevatorGridLayout);

    // Add buttons or labels to the grid layout based on dimensions
    for (int floor = numFloors - 1; floor >= 0; floor--)
    {
        for (int elevator = 1; elevator <= numElevators; ++elevator)
        {
            // Create a QLabel for each position
            QLabel* cube = new QLabel;
            cube->setText(" ");
            cube->setFixedSize(64, 64);
            QString color = "gray";

            if(numFloors - floor - 1 == 0) // all ev's start at 0
                color = "red";

            cube->setStyleSheet(QString("background-color: %1").arg(color));

            // Add the widget to the grid layout at the specified position
            elevatorGridLayout->addWidget(cube, floor, elevator);
        }

        Floor* flr = new Floor(floor + 1); // floor class is a part of the UI

        // Connect the signals from the Floor to the slots in the ElevatorController
        connect(flr, &Floor::upButtonPressed, this, &ElevatorController::buttonPressedUp);
        connect(flr, &Floor::downButtonPressed, this, &ElevatorController::buttonPressedDown);

        // adds the floor at the start (far left)
        elevatorGridLayout->addWidget(flr, numFloors - floor - 1, 0);
        floors.push_back(flr);
    }

    /*
    some helpful stuff
    int rowCount = elevatorGridLayout->rowCount();
    int columnCount = elevatorGridLayout->columnCount();
    QLayoutItem* item = elevatorGridLayout->itemAtPosition(targetRow, targetColumn);
    QLabel* square = qobject_cast<QLabel*>(elevatorGridLayout->itemAtPosition(targetRow, targetCol));
    square->setStyleSheet(QString("background-color: %1".arg(color));
    */

    // create elevators
    for (int elevator = 1; elevator <= numElevators; ++elevator)
    {
        Elevator* ev = new Elevator(elevator);
        elevators.push_back(ev);

        // should be connecting slot in elevator to signals in elevator controller
        connect(this, &ElevatorController::updateElevators, ev, &Elevator::updateElevator);
        connect(this, &ElevatorController::sendRequestToElevator, ev, &Elevator::pressButton);
        connect(this, &ElevatorController::helpButton, ev, &Elevator::helpButtonPressed);
        connect(this, &ElevatorController::emergency, ev, &Elevator::emergency);
        connect(ev, &Elevator::floorChanged, this, &ElevatorController::elevatorFloorChanged);
        connect(ev, &Elevator::doorOpened, this, &ElevatorController::doorOpened);
        connect(ev, &Elevator::doorClosed, this, &ElevatorController::doorClosed);
    }

    requestScanTimer = new QTimer(this);
    connect(requestScanTimer, &QTimer::timeout, this, &ElevatorController::scanRequestTree);
    requestScanTimer->start(500);  // Execute scanRequestTree every 10 milliseconds (0.01 seconds)

    elevatorUpdateTimer = new QTimer(this);
    connect(elevatorUpdateTimer, &QTimer::timeout, this, &ElevatorController::updateElevators);
    elevatorUpdateTimer->start(1000); // Emit the signal every 1000 milliseconds (1 second)

    qDebug() << "Elevator Controller Initialized";
}

ElevatorController::~ElevatorController() // clean up floors
{
    for(int i = 0; i < floors.size(); i++)
        delete floors[i];

    for(int i = 0; i < elevators.size(); i++)
        delete elevators[i];

    elevatorUpdateTimer->stop();
    requestScanTimer->stop();
    delete requestScanTimer;
    delete elevatorUpdateTimer;
}

void ElevatorController::buttonElevatorSubmit()
{
    // get the int values from the combo boxes
    int eb = ui->comboElevatorBox->currentText().remove(0, 10).toInt();
    int fb = ui->comboFloorBox->currentText().remove(0, 7).toInt();

    qDebug() << "UI Button - elevator submit pressed eb: " << eb << " fb: " << fb;
    //ui->passengerNumber->display(eb * fb);
}

void ElevatorController::buttonPlaceOnFloor()
{

}

void ElevatorController::elevatorSelected(int index)
{
    ui->passengerNumber->display(elevators[index]->numPassengers());
    QString buttonList = "";
    const std::vector<int>& blist = elevators[index]->getButtonsPressed();
    for(const int& a : blist)
    {
        buttonList += QString::number(a) + " ";
    }
    ui->textBrowserButtonsPressed->setPlainText(buttonList);
}


void ElevatorController::floorSelected(int index)
{
    ui->passangerOnFloorNumber->display(floors[index]->peopleOnFloor());
}

void ElevatorController::doorOpened(int ev)
{
    // a door has opened
}

void ElevatorController::doorClosed(int ev)
{
    // a door has closed
}

void ElevatorController::elevatorFloorChanged(int floor, int ev)
{
    // each elevator emits this when the moved to new floor
    qDebug()  << "Elevator floor changed, floor: " << floor << " elevator: " << ev;
}

void ElevatorController::buttonPressedUp(int floor)
{
    // an up button on a floor has been pressed
    qDebug()  << "Floor up button pressed: " << floor;
}

void ElevatorController::buttonPressedDown(int floor)
{
    // a down button on a floor has been pressed
    qDebug() << "Floor down button pressed: " << floor;
}

void ElevatorController::scanRequestTree()
{
    // happen on a timer, scan request tree realloc elevators if free
    qDebug() << "scanRequestTree pings "; // << i++;
}
