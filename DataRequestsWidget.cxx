#include "DataRequestsWidget.h"
#include "ui_DataRequests.h"

#include <QMutex>
#include <QLayout>

#include "QPulsePlot.h"

#include "cdm/CommonDataModel.h"
#include "PulsePhysiologyEngine.h"
#include "cdm/engine/SEEngineTracker.h"
#include "cdm/utils/FileUtils.h"
#include "cdm/system/equipment/electrocardiogram/SEElectroCardioGramWaveformInterpolator.h"
#include "cdm/properties/SEScalarTime.h"



class DataRequestsWidget::Controls : public Ui::DataRequestsWidget
{
public:
  Controls(QTextEdit& log) : LogBox(log) {}
  QTextEdit&                         LogBox;
  QMutex                             Mutex;
  size_t                             CurrentPlot=-1;
  std::vector<QPulsePlot*>           Plots;
  std::vector<double>                Values; // New value for the plot
};

DataRequestsWidget::DataRequestsWidget(QTextEdit& log, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls(log);
  m_Controls->setupUi(this);

  connect(m_Controls->DataRequested, SIGNAL(currentIndexChanged(int)), SLOT(ChangePlot(int)));
}

DataRequestsWidget::~DataRequestsWidget()
{
  Reset();
  delete m_Controls;
}

void DataRequestsWidget::Reset()
{
  m_Controls->CurrentPlot = -1;
  DELETE_VECTOR(m_Controls->Plots);
  m_Controls->DataRequested->clear();
}

void DataRequestsWidget::ChangePlot(int idx) 
{
  m_Controls->Mutex.lock();
  if (m_Controls->CurrentPlot != -1)
  {
    m_Controls->Plots[m_Controls->CurrentPlot]->GetView().setVisible(false);
    m_Controls->CurrentPlot = idx;
  }
  m_Controls->Mutex.unlock();
}

void DataRequestsWidget::BuildGraphs(PhysiologyEngine& pulse)
{
  Reset();
  std::stringstream ss;
  SEDataRequestManager& drMgr = pulse.GetEngineTracker()->GetDataRequestManager();
  std::string title;
  std::string unit;
  for (SEDataRequest* dr : drMgr.GetDataRequests())
  {
    if (dr->HasUnit())
      unit = " (" + dr->GetUnit()->GetString() + ")";
    else
      unit = "";
    switch (dr->GetCategory())
    {
    case cdm::DataRequestData_eCategory_Patient:
      title = "Patient " + dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_Physiology:
      title = dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_Environment:
      title = dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_GasCompartment:
    case cdm::DataRequestData_eCategory_LiquidCompartment:
      if (dr->HasSubstanceName())
        title = dr->GetCompartmentName() + " " + dr->GetSubstanceName() + " " + dr->GetPropertyName() + unit;
      else
        title = dr->GetCompartmentName() + " " + dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_ThermalCompartment:
      title = dr->GetCompartmentName() + " " + dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_TissueCompartment:
      title = dr->GetCompartmentName() + " " + dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_Substance:
      if (dr->HasCompartmentName())
        title = dr->GetSubstanceName() + " " + dr->GetCompartmentName() + " " + dr->GetPropertyName() + unit;
      else
        title = dr->GetSubstanceName() + " " + dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_AnesthesiaMachine:
      title = dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_ECG:
      title = dr->GetPropertyName() + unit;
      break;
    case cdm::DataRequestData_eCategory_Inhaler:
      title = dr->GetPropertyName() + unit;
      break;
    }
    if (!pulse.GetEngineTracker()->TrackRequest(*dr))
    {// Could not hook this up, get rid of it
      ss << "Unable to find data for " << title;
      m_Controls->LogBox.append(ss.str().c_str());
      continue;
    }
    QPulsePlot *p = new QPulsePlot(1000);
    p->GetChart().setTitle(title.c_str());
    m_Controls->DataGraphWidget->layout()->addWidget(&p->GetView());
    m_Controls->Plots.push_back(p);
    m_Controls->DataRequested->addItem(QString(title.c_str()));
  }
  
  m_Controls->CurrentPlot = 0;
  m_Controls->DataRequested->setCurrentIndex(0); 
  m_Controls->Plots[0]->GetView().setVisible(true);
}

void DataRequestsWidget::ProcessPhysiology(PhysiologyEngine& pulse)
{
  // This is where we pull data from pulse, and push any actions to it
  size_t i = 0;
  QPulsePlot* plot;
  pulse.GetEngineTracker()->PullData();
  double  v;
  m_Controls->Mutex.lock();
  for (SEDataRequest* dr : pulse.GetEngineTracker()->GetDataRequestManager().GetDataRequests())
  {
    plot = m_Controls->Plots[i++];
    if (dr->HasUnit())
     v=pulse.GetEngineTracker()->GetScalar(*dr)->GetValue(*dr->GetUnit());
    else
     v=pulse.GetEngineTracker()->GetScalar(*dr)->GetValue();
    plot->Append(pulse.GetSimulationTime(TimeUnit::s),v);
  }
  m_Controls->Mutex.unlock();
}

void DataRequestsWidget::PulseUpdateUI()
{
  m_Controls->Mutex.lock();
  m_Controls->Plots[m_Controls->CurrentPlot]->UpdateUI();
  m_Controls->Mutex.unlock();
}

