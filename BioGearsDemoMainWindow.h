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
  BioGearsDemoMainWindow();
  ~BioGearsDemoMainWindow();

private:

  class pqInternals;
  pqInternals* Internals;
};

#endif // BIOGEARSDEMOMAINWINDOW_H
