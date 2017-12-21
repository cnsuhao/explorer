/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QObject>
#include <QDockWidget>
#include "QPulse.h"
#include "GeometryView.h"

namespace Ui {
  class MultiTraumaShowcaseWidget;
}

class MultiTraumaShowcaseWidget : public QDockWidget, public PulseListener
{
  Q_OBJECT
public:
  MultiTraumaShowcaseWidget(QPulse& qp, GeometryView& geometry, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~MultiTraumaShowcaseWidget();

  void ConfigurePulse(PhysiologyEngine& pulse, SEDataRequestManager& drMgr);
  void ProcessPhysiology(PhysiologyEngine& pulse);

signals:
  void UIChanged();
  void PulseChanged();
protected slots:
  void UpdateUI();
  void PulseUpdate();
  void ApplyHemorrhage();
  void ApplyPneumothorax();
  void ApplyPressure();
  void ApplyNeedleDecompression();
  void ApplyTourniquet();
  void InfuseSaline();
  void InjectMorphine();

private:
  class Controls;
  Controls* m_Controls;

};