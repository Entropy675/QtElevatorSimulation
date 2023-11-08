#ifndef FLOOR_H
#define FLOOR_H

#include <QObject>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>

class Floor : public QGroupBox
{
    Q_OBJECT
	
public:
    Floor(int floorNum, QWidget *parent = nullptr);
    ~Floor(); // may need this

    int peopleOnFloor();

signals: // delegates to controller... emit to controller?
    void upButtonPressed(int floor);
    void downButtonPressed(int floor);

private slots:
    void handleUpButtonPress();
    void handleDownButtonPress();

private:
	const int floorNum; // must be in member initialization list 
    int numPeople;

    QGridLayout* groupLayout;
    QPushButton* upButton;
    QPushButton* downButton;
    // Private internals, whatever you gotta do to implement the interface
};


#endif
