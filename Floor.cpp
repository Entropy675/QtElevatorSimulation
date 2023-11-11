#include "Floor.h"
#include "defs.h"

Floor::Floor(int fn, QWidget *parent) : floorNum(fn), QGroupBox(parent)
{
    setTitle("Floor " + QString::number(fn));

    groupLayout = new QGridLayout;
    this->setLayout(groupLayout);
    this->setFixedWidth(100);

    this->setFixedSize(BUTTON_SCALE*2, BUTTON_SCALE*3);
    upButton = new QPushButton(QString("↑↑"), this);
    upButton->setFixedSize(BUTTON_SCALE, BUTTON_SCALE);
    downButton = new QPushButton(QString("↓↓"), this); //  new QPushButton(QString("↓ F%1 ↓").arg(fn), this);
    downButton->setFixedSize(BUTTON_SCALE, BUTTON_SCALE);

    groupLayout->addWidget(upButton);
    groupLayout->addWidget(downButton);

    connect(upButton, &QPushButton::clicked, this, &Floor::handleUpButtonPress);
    connect(downButton, &QPushButton::clicked, this, &Floor::handleDownButtonPress);

    if(DEBUG_GENVALUES)
    {
        numPeople = fn + 1;
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
    {
        numPeople = 0;
        return false;
    }

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
