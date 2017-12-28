/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QObject>
#include <QDockWidget>
#include "QPulse.h"

namespace Ui {
  class DataRequestsWidget;
}

class DataRequestsWidget : public QDockWidget, public PulseListener
{
  Q_OBJECT
public:
  DataRequestsWidget(QTextEdit& log, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~DataRequestsWidget();

  void Reset();
  void BuildGraphs(PhysiologyEngine& pulse);
  void ProcessPhysiology(PhysiologyEngine& pulse);

  void PulseUpdateUI();// Main Window will call this to update UI Components

signals:
protected slots:
  void ChangePlot(int);

private:
  class Controls;
  Controls* m_Controls;
};