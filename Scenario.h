#ifndef SCENARIO_H
#define SCENARIO_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <pqDataRepresentation.h>
#include <pqPipelineSource.h>
#include <pqRenderView.h>


class Scenario : public QObject
{
    Q_OBJECT

public:
    Scenario(pqRenderView* view, QObject* parentObject = NULL);
    ~Scenario();

    void loadScenarioData();
    void colorData(double o2sat);

protected:
    QPointer<pqRenderView> m_View;

    QList< pqPipelineSource* > m_DataSources;
    QList< QPointer< pqDataRepresentation > > m_DataRepresentations;
};

#endif // SCENARIO_H
