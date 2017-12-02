#ifndef SCENARIO_H
#define SCENARIO_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <pqDataRepresentation.h>
#include <pqPipelineSource.h>
#include <pqRenderView.h>


class GeometryView : public QObject
{
  Q_OBJECT

public:
  GeometryView(pqRenderView* view, QObject* parentObject = NULL);
  ~GeometryView();

  void LoadGeometry();
  void RenderSPO2(double spO2);

protected:
  QPointer<pqRenderView> m_View;

  QList< pqPipelineSource* > m_DataSources;
  QList< QPointer< pqDataRepresentation > > m_DataRepresentations;
};

#endif // SCENARIO_H
