#ifndef SCENARIO_H
#define SCENARIO_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <pqDataRepresentation.h>
#include <pqPipelineSource.h>
#include <pqRenderView.h>

#include "QPulse.h"


class GeometryView : public QObject, public PulseListener
{
  Q_OBJECT

public:
  GeometryView(pqRenderView* view, QObject* parentObject = NULL);
  ~GeometryView();

  void Reset();

  void LoadGeometry();
  void RenderSpO2(bool b);

  void ProcessPhysiology(PhysiologyEngine& pulse);
  void PulseUpdateUI();


protected:
  class Data;
  Data* m_Data;
  
  QPointer<pqRenderView> m_View;

  QList< pqPipelineSource* > m_DataSources;
  QList< QPointer< pqDataRepresentation > > m_DataRepresentations;
};

#endif // SCENARIO_H
