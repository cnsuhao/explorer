#include "BioGearsDemoMainWindow.h"
#include "ui_BioGearsDemoMainWindow.h"

BioGearsDemoMainWindow::BioGearsDemoMainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::BioGearsDemoMainWindow)
{
  ui->setupUi(this);
}

BioGearsDemoMainWindow::~BioGearsDemoMainWindow()
{
  delete ui;
}
