#ifndef BIOGEARSDEMOMAINWINDOW_H
#define BIOGEARSDEMOMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
  class BioGearsDemoMainWindow;
}

class BioGearsDemoMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit BioGearsDemoMainWindow(QWidget *parent = 0);
  ~BioGearsDemoMainWindow();

private:
  Ui::BioGearsDemoMainWindow *ui;
};

#endif // BIOGEARSDEMOMAINWINDOW_H
