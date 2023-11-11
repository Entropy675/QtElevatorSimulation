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
    connect(ui->pushAdd10FloorButton, &QPushButton::clicked, this, &ElevatorController::add10ToEachFloor);
    connect(ui->pushBuildingEmergencyButton, &QPushButton::clicked, this, &ElevatorController::triggerBuildingEmergency);
    connect(ui->spinBoxLeaveElevator, qOverload<int>(&QSpinBox::valueChanged), this, &ElevatorController::moveLeaveElevatorBoxChange);
    connect(ui->pushEmergencyResetButton, &QPushButton::clicked, this, &ElevatorController::resetAllElevatorsEmergency);
    connect(ui->pushLeaveButton, &QPushButton::clicked, this, &ElevatorController::buttonLeaveElevator);

    // keep in mind: "setWidget" and "setLayout" etc add to the ui tree, memory managed by Qt not Me :-)

    // widget to hold the grid layout
    QWidget* elevatorWidget = new QWidget();
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

    // create elevators
    for (int elevator = 1; elevator <= numElevators; ++elevator)
    {
        Elevator* ev = new Elevator(elevator);
        elevators.push_back(ev);
        updateSelectedElevatorDisplays();
        // should be connecting slot in elevator to signals in elevator controller
        connect(this, &ElevatorController::resetEmergency, ev, &Elevator::resetEmergencyInElevator);
        connect(this, &ElevatorController::sendRequestToElevator, ev, &Elevator::pressButton);
        connect(this, &ElevatorController::helpButton, ev, &Elevator::helpButtonPressed);
        connect(this, &ElevatorController::buildingEmergency, ev, &Elevator::emergency);
        connect(ev, &Elevator::floorChanged, this, &ElevatorController::elevatorFloorChanged);
        connect(ev, &Elevator::doorOpened, this, &ElevatorController::doorOpened);
        connect(ev, &Elevator::doorClosed, this, &ElevatorController::doorClosed);
        connect(ev, &Elevator::overloaded, this, &ElevatorController::overloaded);
        connect(ev, &Elevator::emergencyOnBoard, this, &ElevatorController::emergency);

        QThread* evThread = new QThread;
        ev->moveToThread(evThread);
        evThread->start();
        threads.push_back(evThread);
    }

    requestScanTimer = new QTimer(this);
    connect(requestScanTimer, &QTimer::timeout, this, &ElevatorController::scanRequestTree);
    requestScanTimer->start(SCAN_REQUEST_TREE_SECS);  // Scan backup request tree every 15 seconds, in case overflow

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


void ElevatorController::updateSelectedElevatorDisplays()
{
    int ev = ui->comboElevatorBox->currentText().remove(0, 10).toInt() - 1; // stored from 0

    ui->passengerNumber->display(elevators[ev]->numPassengers());

    QString buttonList = "";
    const std::set<int>& blist = elevators[ev]->getButtonsPressed();
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

    updateSelectedElevatorDisplays();
}

void ElevatorController::buttonPlaceOnFloor()
{
    qDebug() << "BUTTON buttonPlaceOnFloor.... spawning ppl on floor";

    const int flr = ui->comboFloorBox->currentText().remove(0, 7).toInt() - 1;

    floors[flr]->addPeople(ui->spinBoxPlace->value());
    ui->passengerOnFloorNumber->display(floors[flr]->peopleOnFloor());

    ui->spinBoxPlace->setValue(0);
}

void ElevatorController::buttonMoveToElevator()
{
    qDebug() << "BUTTON: buttonMoveToElevator.... moving ppl ";
    // So we need to call some func, that finds the

    const int flr = ui->comboFloorBox->currentText().remove(0, 7).toInt();
    int evweak = ui->comboElevatorBox->currentText().remove(0, 10).toInt() - 1; // if this is on the same floor its used

    Elevator* availableEv = nullptr;
    Elevator* weakEv = nullptr;
    bool potentialEvPassed = false;

    if(elevators[evweak]->currentFloor() == flr)
        weakEv = elevators[evweak];

    for(Elevator* ev : elevators)
    {
        if(ev->currentFloor() == flr && ev->currentState() == Elevator::DoorsOpen)
        {
            availableEv = ev;
            if(weakEv == ev && weakEv != nullptr)
            {
                availableEv = weakEv;
                break;
            }
        }
    }

    if(!availableEv)
        return;

    const int val = ui->spinBoxMove->value(); //usr input
    ui->spinBoxMove->setValue(0);

    floors[flr - 1]->removePeople(val);
    availableEv->addPassengers(val);

    // just set the combo box option to the one that people were put into automatically... for visibility
    ui->comboElevatorBox->setCurrentIndex(availableEv->getId() - 1);

    // update the floor on people display
    ui->passengerOnFloorNumber->display(floors[flr - 1]->peopleOnFloor());
    updateSelectedElevatorDisplays();
}

void ElevatorController::buttonLeaveElevator()
{

    int ev = ui->comboElevatorBox->currentText().remove(0, 10).toInt();
    const int flr = elevators[ev - 1]->currentFloor() - 1;

    qDebug() << "BUTTON: buttonLeaveElevator.... moving ppl EV: " << ev << " FLR: " << flr;


    const int val = ui->spinBoxLeaveElevator->value(); //usr input
    ui->spinBoxLeaveElevator->setValue(0);

    elevators[ev - 1]->removePassengers(val);
    floors[flr]->addPeople(val);

    // just set the combo box option to the one that people were put into automatically... for visibility
    ui->comboFloorBox->setCurrentIndex(flr);

    // update the floor on people display
    ui->passengerNumber->display(elevators[ev-1]->numPassengers());
    ui->passengerOnFloorNumber->display(floors[flr]->peopleOnFloor());
    updateSelectedElevatorDisplays();
}


void ElevatorController::add10ToEachFloor()
{
    qDebug() << "BUTTON add 10 To Each Floor.... spawning 10 ppl on each floor!";

    for(Floor* f : floors)
    {
        f->addPeople(10);
    }

    const int flr = ui->comboFloorBox->currentText().remove(0, 7).toInt() - 1;
    ui->passengerOnFloorNumber->display(floors[flr]->peopleOnFloor());
    ui->spinBoxPlace->setValue(0);
}

void ElevatorController::triggerBuildingEmergency()
{
    emit buildingEmergency();
}

void ElevatorController::resetAllElevatorsEmergency()
{
    emit resetEmergency(-1);
    updateSelectedElevatorDisplays();
}

void ElevatorController::controlMoveButtonActivated(Elevator* availableEv)
{
    // we want to check if there is an elevator on the floor in door open state
    // & set the control button to active or not based on it
    qDebug() << "BUTTON ACTIVATE: Activating/Deactivating the Move Button to allow moving ppl ";

    const int flr = ui->comboFloorBox->currentText().remove(0, 7).toInt();
    const int evNum = ui->comboElevatorBox->currentText().remove(0, 10).toInt();

    for(Elevator* ev : elevators)
    {
        if(availableEv != nullptr)
            break;
        if(ev->currentFloor() == flr && ev->currentState() == Elevator::DoorsOpen)
            availableEv = ev;
    }

    if(elevators[evNum - 1]->currentFloor() == flr)
        ui->pushLeaveButton->setEnabled(true);
    else if(ui->pushLeaveButton->isEnabled())
        ui->pushLeaveButton->setEnabled(false);

    if(availableEv != nullptr && flr == availableEv->currentFloor())
        ui->pushMoveButton->setEnabled(true);
    else if(ui->pushMoveButton->isEnabled())
        ui->pushMoveButton->setEnabled(false);
}

void ElevatorController::elevatorSelected(int index)
{
    ui->passengerNumber->display(elevators[index]->numPassengers());

    updateSelectedElevatorDisplays();

    controlMoveButtonActivated(elevators[index]);


    qDebug()  << "COMBO BOX: Elevator selected. Elevator: " << index;
}

void ElevatorController::floorSelected(int index)
{
    // update the segment display for passangers
    ui->passengerOnFloorNumber->display(floors[index]->peopleOnFloor());
    qDebug()  << "COMBO BOX: Floor selected. Floor: " << index;

    ui->spinBoxMove->setValue(0); // wipe this since its different # ppl
    controlMoveButtonActivated(); // potentially changes move buttons state
}

void ElevatorController::moveComboBoxChange(int index)
{
    if(index > ui->passengerOnFloorNumber->value())
    {
        ui->spinBoxMove->setValue(ui->passengerOnFloorNumber->value());
    }
}

void ElevatorController::moveLeaveElevatorBoxChange(int index)
{
    if(index > ui->passengerNumber->value())
    {
        ui->spinBoxLeaveElevator->setValue(ui->passengerNumber->value());
    }
}

// --- UI UPDATE / EV SOCKET FUNCS ---

void ElevatorController::elevatorFloorChanged(int floor, int ev, bool up)
{
    // each elevator emits this when the moved to new floor
    qDebug() << "EV signal: Elevator floor changed, floor: " << floor << " elevator: " << ev << " up dir: " << up;
    qDebug() << "(X, Y) : " << floor << ", " << (ev + 1);

    const int x = floors.size() - floor; // as the floors decrease, x increases (flr increase, x decrease)
    const int y = ev + 1;
    QLayoutItem* layoutItem = elevatorGridLayout->itemAtPosition(x, y); // check if we are looking at a valid ev

    if (layoutItem)
    {
        QWidget* widget = layoutItem->widget();

        if (widget)
        {
            const QMetaObject* metaObject = widget->metaObject();
            QString widgetType = QString::fromUtf8(metaObject->className());

            if (widgetType == "QLabel")
            {

                QLabel* square = qobject_cast<QLabel*>(widget);
                QLabel* squarePrev;

                if(!up)
                    squarePrev = qobject_cast<QLabel*>(elevatorGridLayout->itemAtPosition((x - 1 + floors.size()) % floors.size(), y)->widget());
                else
                    squarePrev = qobject_cast<QLabel*>(elevatorGridLayout->itemAtPosition((x + 1)%floors.size(), y)->widget());

                squarePrev->setStyleSheet(QString("background-color: gray;"));

                if(elevators[ev]->currentState() == Elevator::Emergency)
                    square->setStyleSheet("background-color: yellow;");
                else
                    square->setStyleSheet("background-color: red;");
            }
            else
                qDebug() << "Widget type: " << widgetType;
        }
        else
            qDebug() << "No widget at this position.";
    } else
        qDebug() << "No layout item at this position.";

}

void ElevatorController::doorOpened(int flr, int ev)
{
    // a door has opened
    qDebug()  << "EV signal: Door opened! Elevator: " << ev;
    const int x = floors.size() - flr; // as the floors decrease, x increases (flr increase, x decrease)
    const int y = ev;
    if (!elevatorGridLayout->itemAtPosition(x, y))
    {
        qDebug()  << "doorClosed(): No Item at position:  (" << x << ", " << y <<  ")";
        return;
    }
    QWidget* wdg = elevatorGridLayout->itemAtPosition(x, y)->widget();
    if(QString::fromUtf8(wdg->metaObject()->className()) != "QLabel")
    {
        qDebug()  << "doorClosed(): Item at position:  (" << x << ", " << y <<  ") " << "is a: " << QString::fromUtf8(wdg->metaObject()->className());
        return;
    }
    QLabel* squarePrev = qobject_cast<QLabel*>(wdg);
    squarePrev->setStyleSheet(QString("background-color: green;"));
    controlMoveButtonActivated(elevators[ev - 1]);
    updateSelectedElevatorDisplays();
}

void ElevatorController::doorClosed(int flr, int ev)
{
    // a door has closed
    qDebug()  << "EV signal: Door closed!! Elevator: " << ev;
    const int x = floors.size() - flr; // as the floors decrease, x increases (flr increase, x decrease)
    const int y = ev;

    if (!elevatorGridLayout->itemAtPosition(x, y))
    {
        qDebug()  << "doorClosed(): No Item at position:  (" << x << ", " << y <<  ")";
        return;
    }
    QWidget* wdg = elevatorGridLayout->itemAtPosition(x, y)->widget();
    if(QString::fromUtf8(wdg->metaObject()->className()) != "QLabel")
    {
        qDebug()  << "doorClosed(): Item at position:  (" << x << ", " << y <<  ") " << "is a: " << QString::fromUtf8(wdg->metaObject()->className());
        return;
    }
    QLabel* squarePrev = qobject_cast<QLabel*>(wdg);
    squarePrev->setStyleSheet(QString("background-color: purple;"));
}

void ElevatorController::overloaded(int flr, int ev)
{
    // a door has closed
    qDebug()  << "EV signal: Elevator overloaded!! Elevator: " << ev;
    const int x = floors.size() - flr; // as the floors decrease, x increases (flr increase, x decrease)
    const int y = ev;
    if (!elevatorGridLayout->itemAtPosition(x, y))
    {
        qDebug()  << "doorClosed(): No Item at position:  (" << x << ", " << y <<  ")";
        return;
    }
    QWidget* wdg = elevatorGridLayout->itemAtPosition(x, y)->widget();
    if(QString::fromUtf8(wdg->metaObject()->className()) != "QLabel")
    {
        qDebug()  << "doorClosed(): Item at position:  (" << x << ", " << y <<  ") " << "is a: " << QString::fromUtf8(wdg->metaObject()->className());
        return;
    }
    QLabel* squarePrev = qobject_cast<QLabel*>(wdg);
    squarePrev->setStyleSheet(QString("background-color: orange;"));
}

void ElevatorController::emergency(int flr, int ev)
{
    // a door has closed
    qDebug()  << "EV signal: Elevator emergency!! Elevator: " << ev;

    if(flr == 0)
    {
        QLayoutItem* layoutItem = elevatorGridLayout->itemAtPosition(floors.size() - 1, ev); // check if we are looking at a valid ev

        if (layoutItem)
        {
            QWidget* widget = layoutItem->widget();

            if (widget)
            {
                const QMetaObject* metaObject = widget->metaObject();
                QString widgetType = QString::fromUtf8(metaObject->className());
                if (widgetType == "QLabel")
                {

                    QLabel* square = qobject_cast<QLabel*>(widget);

                    if(elevators[ev - 1]->currentState() == Elevator::Idle)
                        square->setStyleSheet(QString("background-color: red;"));
                    else
                        square->setStyleSheet(QString("background-color: yellow;"));
                }
                else
                    qDebug() << "Widget type: " << widgetType;
            }
            else
                qDebug() << "No widget at this position.";
        } else
            qDebug() << "No layout item at this position.";
    }
}

// --- EV REQUEST FUNCS ---

void ElevatorController::buttonPressedUp(int floor)
{
    // an up button on a floor has been pressed
    qDebug()  << "Floor signal: Floor up button pressed: " << floor;
    handleFlrPressed(FloorDirection(floor, true));
}

void ElevatorController::buttonPressedDown(int floor)
{
    // a down button on a floor has been pressed
    qDebug() << "Floor signal: Floor down button pressed: " << floor;
    handleFlrPressed(FloorDirection(floor, false));
}

void ElevatorController::handleFlrPressed(FloorDirection fd)
{
    struct BestElevator
    {
        Elevator* ev;
        int cEvMoves = 0;

        BestElevator(Elevator* e)
        {
            ev = e;
            cEvMoves = ev->getNumFloorsReserved();
        }

        bool operator<(const BestElevator& o) const
        {
            return this->cEvMoves < o.cEvMoves;
        }
    };

    std::priority_queue<BestElevator> foundList;
    for(Elevator* pEv : elevators)
    {
        bool addpEv = (pEv->currentState() == Elevator::Idle)
        || (pEv->currentState() == Elevator::MovingUp && fd.up && pEv->currentFloor() < fd.num )
        || (pEv->currentState() == Elevator::MovingDown && !fd.up && pEv->currentFloor() > fd.num);

        if(addpEv)
            foundList.push(BestElevator(pEv));
    }

    if(foundList.size() > 0)
    {
        //int index = fd.time % foundList.size();
        //int index = (fd.time % 2) ? 0 : 1;
        //for(int i = 0; i < index; i++)
            //foundList.pop();
        foundList.top().ev->moveTofloor(fd.num);
    }
    else
        earliestRequestTree.push(fd);
}

void ElevatorController::scanRequestTree()
{
    // happen on a timer, scan request tree realloc elevators if free
    if(AGGRESSIVE_LOGGING && !earliestRequestTree.empty())
        qDebug() << "scanRequestTree-> Request floor:  " << earliestRequestTree.top().num << " up: " << earliestRequestTree.top().up; // << i++;
    if (earliestRequestTree.empty())
    {
        return;
    }

    FloorDirection fd = earliestRequestTree.top();
    earliestRequestTree.pop();
    handleFlrPressed(FloorDirection(fd.num, fd.up));
}

// --- EV REQUEST FUNCS ---
