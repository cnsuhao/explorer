#include "Scenario.h"

#include "FileLoader.h"

#include <QColor>
#include <QList>
#include <QVariant>

#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqSMAdaptor.h>
#include <pqRenderView.h>
#include <pqView.h>

#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMPVRepresentationProxy.h>

Scenario::Scenario(pqRenderView* view, QObject* parentObject) :
    m_View(view)
{
}

Scenario::~Scenario()
{
}

void Scenario::loadScenarioData()
{
    vtkSMProxy* renderProxy = m_View->getProxy();
//    this->m_DataSources.push_back(FileLoader::loadDataFile("data/Segment_0.vtp"));
    // Read in lungs
    this->m_DataSources.push_back(FileLoader::loadDataFile("data/lungs.vtp"));
    this->m_DataRepresentations.push_back(pqApplicationCore::instance()->getObjectBuilder()->createDataRepresentation(m_DataSources[0]->getOutputPort(0), m_View));

    vtkSMProxy* lungProxy = m_DataRepresentations[0]->getProxy();
    vtkSMPVRepresentationProxy* lungRepProxy = vtkSMPVRepresentationProxy::SafeDownCast(lungProxy);
    vtkSMPropertyHelper(lungRepProxy,"Opacity").Set(0.75);
    lungRepProxy->UpdateProperty("Opacity");

    vtkSMProperty* diffuse = lungProxy->GetProperty("DiffuseColor");

    QList<QVariant> rgb = pqSMAdaptor::getMultipleElementProperty(diffuse);
    QColor color(Qt::red);

    rgb.clear();
//    rgb.append(color.redF());
//    rgb.append(color.greenF());
//    rgb.append(color.blueF());

    rgb.append(1.0);
    rgb.append(0.45);
    rgb.append(0.66);

    pqSMAdaptor::setMultipleElementProperty(diffuse,rgb);
    lungProxy->UpdateProperty("DiffuseColor");

    // Read in trachea/bronchus
    this->m_DataSources.push_back(FileLoader::loadDataFile("data/trachea.vtp"));
    this->m_DataRepresentations.push_back(pqApplicationCore::instance()->getObjectBuilder()->createDataRepresentation(m_DataSources[1]->getOutputPort(0), m_View));

    this->m_DataSources.push_back(FileLoader::loadDataFile("data/bronchus.vtp"));
    this->m_DataRepresentations.push_back(pqApplicationCore::instance()->getObjectBuilder()->createDataRepresentation(m_DataSources[2]->getOutputPort(0), m_View));

    // Read in skin
    this->m_DataSources.push_back(FileLoader::loadDataFile("data/skin.vtp"));
    this->m_DataRepresentations.push_back(pqApplicationCore::instance()->getObjectBuilder()->createDataRepresentation(m_DataSources[3]->getOutputPort(0), m_View));

    vtkSMProxy* skinProxy = m_DataRepresentations[3]->getProxy();
    vtkSMPVRepresentationProxy* skinRepProxy = vtkSMPVRepresentationProxy::SafeDownCast(skinProxy);
    vtkSMPropertyHelper(skinRepProxy, "Opacity").Set(0.25);
    skinRepProxy->UpdateProperty("Opacity");

    // Set camera view
    double camera_position[3] = {0, -65, 50};
    double camera_focal_point[3] = {0, 1, 50};
    double camera_up[3] = {0,0,1};

//    vtkSMPropertyHelper(renderProxy,"CameraParallelProjection").Set(true);
//    renderProxy->UpdateVTKObjects();

    vtkSMPropertyHelper(renderProxy,"CameraPosition").Set(camera_position,3);
    vtkSMPropertyHelper(renderProxy,"CameraFocalPoint").Set(camera_focal_point,3);
    vtkSMPropertyHelper(renderProxy,"CameraViewUp").Set(camera_up,3);
    m_View->resetCenterOfRotation();

    renderProxy->UpdateVTKObjects();
}

void Scenario::colorData(double o2sat)
{
    std::cout << "Coloring by " << o2sat << std::endl;
    QColor color;

    if (o2sat >= 0.95)
        color = QColor(255,115,170);
    else if (o2sat <= 0.90)
        color = QColor(Qt::blue);
    else
    {
        QColor color1(255,115,170);
        QColor color2(Qt::blue);

        double t = (0.95 - o2sat) / 0.05; // fraction of distance from 0.95 to 0.90

        std::cout << "Using t=" << t << std::endl;

        double r = floor((1-t)*color1.red() + t*color2.red());
        double g = floor((1-t)*color1.green() + t*color2.green());
        double b = floor((1-t)*color1.blue() + t*color2.blue());

        color = QColor( r, g, b);
    }

    vtkSMProxy* proxy = m_DataRepresentations[0]->getProxy();
    vtkSMProperty* diffuse = proxy->GetProperty("DiffuseColor");

    QList<QVariant> rgb = pqSMAdaptor::getMultipleElementProperty(diffuse);
    rgb.clear();
    rgb.append(color.redF());
    rgb.append(color.greenF());
    rgb.append(color.blueF());

    pqSMAdaptor::setMultipleElementProperty(diffuse,rgb);

    proxy->UpdateVTKObjects();
}

