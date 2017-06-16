#ifndef BIOGEARSDEMOMAINWINDOW_H
#define BIOGEARSDEMOMAINWINDOW_H

#include <QMainWindow>

#include "BioGearsPhysiologyEngine.h"
#include "cdm/CommonDataModel.h"
#include "cdm/engine/PhysiologyEngine.h"
#include "cdm/properties/SEScalarTime.h"
#include "cdm/properties/SEScalar0To1.h"
#include "cdm/properties/SEScalarVolume.h"

namespace Ui {
  class BioGearsDemoMainWindow;
}

class BioGearsDemoMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  BioGearsDemoMainWindow();
  ~BioGearsDemoMainWindow();

signals:
  void dataChanged();

protected slots:
  void updateUI();
  void startEngine();
  void addObstruction();
  void advanceTime();
  void playButtonPressed();
  void pauseButtonPressed();
  void keepPlaying();
  void epiButtonPressed();

private:
  class pqInternals;
  pqInternals* Internals;

  std::unique_ptr<PhysiologyEngine> bg;
  bool playing;

  void updateLog();

  QTimer* playTimer;
};

#endif // BIOGEARSDEMOMAINWINDOW_H
