#include "mainwindow.h"
#include "ElevatorController.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Elevator Simulation");
    ec = new ElevatorController(ui); // you can pass in dimensions or use the defined ones (see defs.h) by default
}

MainWindow::~MainWindow()
{
    delete ui;
    delete ec;
}

