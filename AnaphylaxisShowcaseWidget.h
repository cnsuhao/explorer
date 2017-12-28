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
  AnaphylaxisShowcaseWidget(QPulse& qp, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~AnaphylaxisShowcaseWidget();

  void ConfigurePulse(PhysiologyEngine& pulse, SEDataRequestManager& drMgr);
  void ProcessPhysiology(PhysiologyEngine& pulse);

signals:
protected slots:
  void ApplyAirwayObstruction();
  void InjectEpinephrine();

private:
  class Controls;
  Controls* m_Controls;
};