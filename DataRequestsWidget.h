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

signals:
  void UIChanged();
  void PulseChanged();
protected slots:
  void UpdateUI();
  void PulseUpdate();

private:
  class Controls;
  Controls* m_Controls;
};