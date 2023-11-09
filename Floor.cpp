#include "Floor.h"
#include "defs.h"

Floor::Floor(int fn, QWidget *parent) : floorNum(fn), QGroupBox(parent)
{
    setTitle("Floor " + QString::number(fn));

    groupLayout = new QGridLayout;
    this->setLayout(groupLayout);
    this->setFixedWidth(100);

    upButton = new QPushButton(QString("↑ F%1 ↑").arg(fn), this);
    downButton = new QPushButton(QString("↓ F%1 ↓").arg(fn), this);

    groupLayout->addWidget(upButton);
    groupLayout->addWidget(downButton);

    connect(upButton, &QPushButton::clicked, this, &Floor::handleUpButtonPress);
    connect(downButton, &QPushButton::clicked, this, &Floor::handleDownButtonPress);

    if(DEBUG)
    {
        numPeople = fn;
    }
}

Floor::~Floor()
{
    delete groupLayout;
    delete upButton;
    delete downButton;
}

int Floor::peopleOnFloor()
{
    return numPeople;
}

void Floor::addPeople(int n)
{
    numPeople += n;
}

bool Floor::removePeople(int n)
{
    if((numPeople - n) < 0)
        return false;

    numPeople -= n;
    return true;
}

void Floor::handleUpButtonPress()
{
    emit upButtonPressed(floorNum);
}

void Floor::handleDownButtonPress()
{
    emit downButtonPressed(floorNum);
}
