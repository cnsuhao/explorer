/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QObject>
#include <QDockWidget>
#include "QPulse.h"

namespace Ui {
  class VitalsMonitorWidget;
}

class VitalsMonitorWidget : public QDockWidget, public PulseListener
{
  Q_OBJECT
public:
  VitalsMonitorWidget(QTextEdit& log, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~VitalsMonitorWidget();

  void Reset();
  void ProcessPhysiology(PhysiologyEngine& pulse);

  void PulseUpdateUI();

//signals:
//protected slots:

private:
  class Controls;
  Controls* m_Controls;
};