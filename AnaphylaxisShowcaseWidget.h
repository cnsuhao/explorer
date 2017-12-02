/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QObject>
#include <QDockWidget>
#include "QPulse.h"

namespace Ui {
  class AnaphylaxisShowcaseWidget;
}

class AnaphylaxisShowcaseWidget : public QDockWidget, public PulseListener
{
  Q_OBJECT
public:
  AnaphylaxisShowcaseWidget(QTextEdit& log, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~AnaphylaxisShowcaseWidget();

  void ConfigurePulse(PhysiologyEngine& pulse);
  void ProcessPhysiology(PhysiologyEngine& pulse);

signals:
  void UIChanged();
  void PulseChanged();
protected slots:
  void UpdateUI();
  void PulseUpdate();
  void ApplyAirwayObstruction();
  void InjectEpinephrine();

private:
  class Controls;
  Controls* m_Controls;

};