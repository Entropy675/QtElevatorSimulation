#include "ElevatorController.h"
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>
#include <QChar>
#include <queue>

ElevatorController::ElevatorController(Ui::MainWindow* u, int numElevators, int numFloors)
{
    // create/populate ui ComboBox elements
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

    // buttons connected
    connect(ui->pushElevatorButton, &QPushButton::clicked, this, &ElevatorController::buttonElevatorSubmit);
    connect(ui->pushPlaceButton, &QPushButton::clicked, this, &ElevatorController::buttonPlaceOnFloor);
    connect(ui->pushMoveButton, &QPushButton::clicked, this, &ElevatorController::buttonMoveToElevator);
    connect(ui->spinBoxMove, qOverload<int>(&QSpinBox::valueChanged), this, &ElevatorController::moveComboBoxChange);

    // keep in mind: "setWidget" and "setLayout" etc add to the ui tree, memory managed by Qt not Me :-)

    // widget to hold the grid layout
    QWidget* elevatorWidget = new QWidget();
    ui->elevatorScrollArea->setWidget(elevatorWidget); // Set the content widget of the ElevatorController

    QGridLayout* elevatorGridLayout = new QGridLayout(elevatorWidget);
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
        updateButtonsPressedText(elevator - 1);
        // should be connecting slot in elevator to signals in elevator controller
        connect(this, &ElevatorController::sendRequestToElevator, ev, &Elevator::pressButton);
        connect(this, &ElevatorController::helpButton, ev, &Elevator::helpButtonPressed);
        connect(this, &ElevatorController::emergency, ev, &Elevator::emergency);
        connect(ev, &Elevator::floorChanged, this, &ElevatorController::elevatorFloorChanged);
        connect(ev, &Elevator::doorOpened, this, &ElevatorController::doorOpened);
        connect(ev, &Elevator::doorClosed, this, &ElevatorController::doorClosed);
        connect(ev, &Elevator::overloaded, this, &ElevatorController::overloaded);

        QThread* evThread = new QThread;
        ev->moveToThread(evThread);
        evThread->start();
        threads.push_back(evThread);
    }

    requestScanTimer = new QTimer(this);
    connect(requestScanTimer, &QTimer::timeout, this, &ElevatorController::scanRequestTree);
    requestScanTimer->start(SCAN_REQUEST_TREE_SECS * 1000);  // Scan backup request tree every 15 seconds, in case overflow

    qDebug() << "Elevator Controller Initialized";
}

ElevatorController::~ElevatorController() // clean up floors
{
    for(int i = 0; i < floors.size(); i++)
        delete floors[i];

    requestScanTimer->stop();
    delete requestScanTimer;

    for(int i = 0; i < threads.size(); i++)
    {
        threads[i]->quit();
        threads[i]->wait();
        delete threads[i];
    }
}

// --- UI INPUT & CALLBACK FUNCS ---

void ElevatorController::updateButtonsPressedText(int evIndex)
{
    QString buttonList = "";
    const std::set<int>& blist = elevators[evIndex]->getButtonsPressed();
    for(const int& a : blist)
    {
        buttonList += QString::number(a) + " ";
    }
    ui->textBrowserButtonsPressed->setPlainText(buttonList);
}

void ElevatorController::buttonElevatorSubmit()
{
    // get the int values from the combo boxes
    int ev = ui->comboElevatorBox->currentText().remove(0, 10).toInt();
    int fb = ui->comboFloorBox->currentText().remove(0, 7).toInt();

    qDebug() << "BUTTON - elevator submit pressed elev: " << ev << " floor button: " << fb;

    elevators[ev - 1]->pressButton(fb);

    updateButtonsPressedText(ev - 1);
}

void ElevatorController::buttonPlaceOnFloor()
{
    qDebug() << "BUTTON buttonPlaceOnFloor.... spawning ppl on floor";

    const int flr = ui->comboFloorBox->currentText().remove(0, 7).toInt() - 1;
    floors[flr]->addPeople(ui->spinBoxPlace->value());
    ui->passangerOnFloorNumber->display(floors[flr]->peopleOnFloor());

    ui->spinBoxPlace->setValue(0);
}

void ElevatorController::buttonMoveToElevator()
{
    qDebug() << "BUTTON: buttonMoveToElevator.... moving ppl ";
    // So we need to call some func, that finds the

}

void ElevatorController::controlMoveButtonActivated()
{
    // we want to check if there is an elevator on the floor in door open state
    // & set the control button to active or not based on it
    qDebug() << "BUTTON ACTIVATE: Activating/Deactivating the Move Button to allow moving ppl ";

    const int flr = ui->comboFloorBox->currentText().remove(0, 7).toInt() - 1;
    Elevator* availableEv = nullptr;

    for(Elevator* ev : elevators)
    {
        if(ev->currentFloor() == flr && ev->currentState() == Elevator::DoorsOpen)
        {
            availableEv = ev;
            break;
        }
    }

    if(availableEv != nullptr)
        ui->pushMoveButton->setEnabled(true);
    else if(ui->pushMoveButton->isEnabled())
        ui->pushMoveButton->setEnabled(false);
}

void ElevatorController::elevatorSelected(int index)
{
    ui->passengerNumber->display(elevators[index]->numPassengers());

    updateButtonsPressedText(index);

    qDebug()  << "COMBO BOX: Elevator selected. Elevator: " << index;
}

void ElevatorController::floorSelected(int index)
{
    // update the segment display for passangers
    ui->passangerOnFloorNumber->display(floors[index]->peopleOnFloor());
    qDebug()  << "COMBO BOX: Floor selected. Floor: " << index;

    controlMoveButtonActivated(); // potentially changes move buttons state
}

void ElevatorController::moveComboBoxChange(int index)
{
    if(index > ui->passangerOnFloorNumber->value())
    {
        ui->spinBoxMove->setValue(ui->passangerOnFloorNumber->value());
    }
}

// --- UI INPUT & CALLBACK FUNCS ---

// --- UI UPDATE / EV SOCKET FUNCS ---

void ElevatorController::doorOpened(int flr, int ev)
{
    // a door has opened
    qDebug()  << "EV signal: Door opened! Elevator: " << ev;
}

void ElevatorController::doorClosed(int flr, int ev)
{
    // a door has closed
    qDebug()  << "EV signal: Door closed!! Elevator: " << ev;
}

void ElevatorController::overloaded(int flr, int ev)
{
    // a door has closed
    qDebug()  << "EV signal: Elevator overloaded!! Elevator: " << ev;
}

void ElevatorController::elevatorFloorChanged(int floor, int ev, bool up)
{
    // each elevator emits this when the moved to new floor
    qDebug()  << "EV signal: Elevator floor changed, floor: " << floor << " elevator: " << ev << " up dir: " << up;
}

// --- UI UPDATE / EV SOCKET FUNCS ---

// --- EV REQUEST FUNCS ---

void ElevatorController::buttonPressedUp(int floor)
{
    // an up button on a floor has been pressed
    qDebug()  << "Floor signal: Floor up button pressed: " << floor;
    handleFlrPressed(floor, true);
}

void ElevatorController::buttonPressedDown(int floor)
{
    // a down button on a floor has been pressed
    qDebug() << "Floor signal: Floor down button pressed: " << floor;
    handleFlrPressed(floor, false);
}

void ElevatorController::handleFlrPressed(int floor, bool up)
{
    Elevator* found = nullptr;
    for(Elevator* pEv : elevators)
    {
        if(pEv->currentState() == Elevator::Idle)
        {
            found = pEv;
            break;
        }

        if(pEv->currentState() == Elevator::MovingUp && (up ? pEv->currentFloor() < floor :  pEv->currentFloor() > floor))
        {
            found = pEv;
            break;
        }

        if(pEv->currentState() == Elevator::MovingDown)
        {
            found = pEv;
            break;
        }
    }

    if(found)
        found->moveTofloor(floor);
    else
        earliestRequestTree.push(FloorDirection(floor, up));
}

void ElevatorController::scanRequestTree()
{
    // happen on a timer, scan request tree realloc elevators if free
    //if(AGGRESSIVE_LOGGING)
    qDebug() << "scanRequestTree pings "; // << i++;
    FloorDirection fd = earliestRequestTree.top();
    earliestRequestTree.pop();
    handleFlrPressed(fd.num, fd.up);
}

// --- EV REQUEST FUNCS ---
