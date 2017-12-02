/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QObject>
#include <QWidget>
#include "QPulse.h"

class VitalsMonitorWidget : public QWidget, public PulseListener
{
  Q_OBJECT
public:
  VitalsMonitorWidget(QTextEdit& log, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~VitalsMonitorWidget();

  void Reset();
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