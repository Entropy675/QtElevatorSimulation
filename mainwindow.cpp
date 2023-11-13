#include "mainwindow.h"
#include "ElevatorController.h"
#include "./ui_mainwindow.h"
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Elevator Simulation");
    ec = new ElevatorController(ui); // you can pass in dimensions or use the defined ones (see defs.h) by default

    connect(this, &MainWindow::screenResized, ec, &ElevatorController::handleScreenResized);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete ec;
}


void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event); // base class implementation
    emit screenResized(event->size().width(), event->size().height()); // Emit to ec with new screen dimensions
}
