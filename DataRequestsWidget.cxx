#include "DataRequestsWidget.h"
#include "ui_DataRequests.h"

#include <QMutex>

#include "cdm/utils/FileUtils.h"

#include "cdm/system/equipment/electrocardiogram/SEElectroCardioGramWaveformInterpolator.h"
#include "cdm/properties/SEFunctionElectricPotentialVsTime.h"

class DataRequestsWidget::Controls : public Ui::DataRequestsWidget
{
public:
  Controls(QTextEdit& log) : LogBox(log) {}
  QTextEdit&                         LogBox;
  QMutex                             Mutex;
  std::vector<std::string>           Graphs; // Instead of plots, just names for now
  std::vector<double>                Values; // New value for the plot
};

DataRequestsWidget::DataRequestsWidget(QTextEdit& log, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls(log);
  m_Controls->setupUi(this);

  connect(this, SIGNAL(UIChanged()), this, SLOT(UpdateUI()));
  connect(this, SIGNAL(PulseChanged()), this, SLOT(PulseUpdate()));
}

DataRequestsWidget::~DataRequestsWidget()
{
  delete m_Controls;
}

void DataRequestsWidget::Reset()
{
  m_Controls->DataRequested->clear();
}

void DataRequestsWidget::BuildGraphs(PhysiologyEngine& pulse)
{
  m_Controls->DataRequested->clear();
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
    m_Controls->Graphs.push_back(title);
    m_Controls->Values.push_back(0);
    m_Controls->DataRequested->addItem(QString(title.c_str()));
  }
  
}

void DataRequestsWidget::ProcessPhysiology(PhysiologyEngine& pulse)
{
  // This is where we pull data from pulse, and push any actions to it
  size_t i = 0;
  pulse.GetEngineTracker()->PullData();
  for (SEDataRequest* dr : pulse.GetEngineTracker()->GetDataRequestManager().GetDataRequests())
  {
   /* if(dr->HasUnit())
      m_Controls->Values[i++] = pulse.GetEngineTracker()->GetScalar(*dr)->GetValue(*dr->GetUnit());
    else
      m_Controls->Values[i++] = pulse.GetEngineTracker()->GetScalar(*dr)->GetValue();*/
  }
  emit PulseChanged(); // Call this if you need to update the UI with data from pulse
}

void DataRequestsWidget::UpdateUI()
{

}

void DataRequestsWidget::PulseUpdate()
{
  // This is where we take the pulse data we pulled and push it to a UI widget
  /*if (++m_Controls->Count == 50)
  {
    m_Controls->Count = 0;
    m_Controls->Mutex.lock();
    for (size_t i = 0; i < m_Controls->Graphs.size(); i++)
      m_Controls->LogBox.append(QString("%1 : %2").arg(m_Controls->Graphs[i].c_str()).arg(m_Controls->Values[i]));
    m_Controls->Mutex.unlock();
  }*/
}

