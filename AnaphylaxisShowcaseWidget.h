/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QObject>
#include <QDockWidget>
#include "QPulse.h"
#include "GeometryView.h"
#include "cdm/scenario/SEDataRequest.h"

namespace Ui {
  class AnaphylaxisShowcaseWidget;
}

class AnaphylaxisShowcaseWidget : public QDockWidget, public PulseListener
{
  Q_OBJECT
public:
  AnaphylaxisShowcaseWidget(QPulse& qp, GeometryView& geometry, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~AnaphylaxisShowcaseWidget();

  void ConfigurePulse(PhysiologyEngine& pulse, SEDataRequestManager& drMgr);
  void ProcessPhysiology(PhysiologyEngine& pulse);

signals:
  void UIChanged();
  void PulseChanged();
  void GeometryChanged();
protected slots:
  void UpdateUI();
  void PulseUpdate();
  void ApplyAirwayObstruction();
  void InjectEpinephrine();

private:
  class Controls;
  Controls* m_Controls;
  std::vector<SEDataRequest*> m_DataRequests;

};