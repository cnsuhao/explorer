#include "VitalsMonitorWidget.h"
#include "ui_VitalsMonitor.h"
#include <QMutex>
#include <QLayout>
#include <QGraphicsLayout>

#include "QPulsePlot.h"

#include "cdm/CommonDataModel.h"
#include "PulsePhysiologyEngine.h"
#include "cdm/utils/FileUtils.h"
#include "cdm/substance/SESubstanceManager.h"
#include "cdm/compartment/SECompartmentManager.h"
#include "cdm/compartment/fluid/SEGasCompartment.h"
#include "cdm/compartment/substances/SEGasSubstanceQuantity.h"
#include "cdm/system/physiology/SEBloodChemistrySystem.h"
#include "cdm/system/physiology/SECardiovascularSystem.h"
#include "cdm/system/physiology/SERespiratorySystem.h"
#include "cdm/system/physiology/SEEnergySystem.h"
#include "cdm/system/equipment/electrocardiogram/SEElectroCardioGram.h"
#include "cdm/properties/SEScalarFrequency.h"
#include "cdm/properties/SEScalarPressure.h"
#include "cdm/Properties/SEScalarElectricPotential.h"
#include "cdm/Properties/SEScalarTemperature.h"
#include "cdm/Properties/SEScalarTime.h"

class VitalsMonitorWidget::Controls : public Ui::VitalsMonitorWidget
{
public:
  Controls(QTextEdit& log) : LogBox(log) {}
  QTextEdit&  LogBox;
  double      HeartRate_bpm;
  double      ECG_III_mV;
  double      ArterialPressure_mmHg;
  double      MeanArterialPressure_mmHg;
  double      DiastolicPressure_mmHg;
  double      SystolicPressure_mmHg;
  double      OxygenSaturation;
  double      EndTidalCarbonDioxidePressure_mmHg;
  double      RespirationRate_bpm;
  double      Temperature_C;
  QMutex      Mutex;
  QPulsePlot* ECG_III_Plot;
  QPulsePlot* ArterialPressure_Plot;
  QPulsePlot* etCO2_Plot;
  SEGasSubstanceQuantity* CarinaCO2=nullptr;
};

VitalsMonitorWidget::VitalsMonitorWidget(QTextEdit& log, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls(log);
  m_Controls->setupUi(this);

  m_Controls->ECG_III_Plot = new QPulsePlot(500);
  m_Controls->ECG_III_Plot->GetSeries().setColor(Qt::green);
  m_Controls->ECG_III_Plot->GetChart().layout()->setContentsMargins(0, 0, 0, 0); 
  m_Controls->ECG_III_Plot->GetChart().setBackgroundRoundness(0);
  m_Controls->ECG_III_Plot->GetChart().setBackgroundBrush(QBrush(Qt::black));
  m_Controls->ECG_III_Plot->GetChart().axisX()->setLinePenColor(Qt::black);
  m_Controls->ECG_III_Plot->GetChart().axisY()->setLinePenColor(Qt::black);
  m_Controls->ECG_III_Plot->GetChart().axisX()->setLabelsVisible(false);
  m_Controls->ECG_III_Plot->GetChart().axisY()->setLabelsVisible(false);
  m_Controls->ECG_III_Plot->GetChart().axisX()->setGridLineVisible(false);
  m_Controls->ECG_III_Plot->GetChart().axisY()->setGridLineVisible(false);
  m_Controls->ECG_III_Plot->SetDataRange(-0.1, 0.9);

  m_Controls->ArterialPressure_Plot = new QPulsePlot(500);
  m_Controls->ArterialPressure_Plot->GetSeries().setColor(Qt::red);
  m_Controls->ArterialPressure_Plot->GetChart().layout()->setContentsMargins(0, 0, 0, 0);
  m_Controls->ArterialPressure_Plot->GetChart().setBackgroundRoundness(0);
  m_Controls->ArterialPressure_Plot->GetChart().setBackgroundBrush(QBrush(Qt::black));
  m_Controls->ArterialPressure_Plot->GetChart().axisX()->setLinePenColor(Qt::black);
  m_Controls->ArterialPressure_Plot->GetChart().axisY()->setLinePenColor(Qt::black);
  m_Controls->ArterialPressure_Plot->GetChart().axisX()->setLabelsVisible(false);
  m_Controls->ArterialPressure_Plot->GetChart().axisY()->setLabelsVisible(false);
  m_Controls->ArterialPressure_Plot->GetChart().axisX()->setGridLineVisible(false);
  m_Controls->ArterialPressure_Plot->GetChart().axisY()->setGridLineVisible(false);
  m_Controls->ArterialPressure_Plot->SetDataRange(70, 115);

  m_Controls->etCO2_Plot = new QPulsePlot(500);
  m_Controls->etCO2_Plot->GetSeries().setColor(Qt::yellow);
  m_Controls->etCO2_Plot->GetChart().layout()->setContentsMargins(0, 0, 0, 0);
  m_Controls->etCO2_Plot->GetChart().setBackgroundRoundness(0);
  m_Controls->etCO2_Plot->GetChart().setBackgroundBrush(QBrush(Qt::black));
  m_Controls->etCO2_Plot->GetChart().axisX()->setLinePenColor(Qt::black);
  m_Controls->etCO2_Plot->GetChart().axisY()->setLinePenColor(Qt::black);
  m_Controls->etCO2_Plot->GetChart().axisX()->setLabelsVisible(false);
  m_Controls->etCO2_Plot->GetChart().axisY()->setLabelsVisible(false);
  m_Controls->etCO2_Plot->GetChart().axisX()->setGridLineVisible(false);
  m_Controls->etCO2_Plot->GetChart().axisY()->setGridLineVisible(false);
  m_Controls->etCO2_Plot->SetDataRange(0.2, 30);

  m_Controls->ECGGraphWidget->layout()->addWidget(&m_Controls->ECG_III_Plot->GetView());
  m_Controls->ABPGraphWidget->layout()->addWidget(&m_Controls->ArterialPressure_Plot->GetView());
  m_Controls->etCO2GraphWidget->layout()->addWidget(&m_Controls->etCO2_Plot->GetView());
}

VitalsMonitorWidget::~VitalsMonitorWidget()
{
  delete m_Controls;
}

void VitalsMonitorWidget::Reset()
{
  m_Controls->HeartRateValue->setText("0");
  m_Controls->BloodPressureValues->setText("0/0");
  m_Controls->MeanBloodPressureValue->setText("(0)");
  m_Controls->SpO2Value->setText("0");
  m_Controls->etCO2Value->setText("0");
  m_Controls->RespiratoryRateValue->setText("0");
  m_Controls->TempeartureValue->setText("0");

  m_Controls->ECG_III_Plot->Reset();
  m_Controls->ArterialPressure_Plot->Reset();
  m_Controls->etCO2_Plot->Reset();

  m_Controls->CarinaCO2 = nullptr;
}

void VitalsMonitorWidget::ProcessPhysiology(PhysiologyEngine& pulse)
{
  // This is where we pull data from pulse, and push any actions to it
  m_Controls->Mutex.lock();

  m_Controls->HeartRate_bpm = pulse.GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min);
  m_Controls->ECG_III_mV = pulse.GetElectroCardioGram()->GetLead3ElectricPotential(ElectricPotentialUnit::mV);
  m_Controls->ArterialPressure_mmHg = pulse.GetCardiovascularSystem()->GetArterialPressure(PressureUnit::mmHg);
  m_Controls->MeanArterialPressure_mmHg = pulse.GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg);
  m_Controls->DiastolicPressure_mmHg = pulse.GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg);
  m_Controls->SystolicPressure_mmHg = pulse.GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg);
  m_Controls->OxygenSaturation = pulse.GetBloodChemistrySystem()->GetOxygenSaturation();
  m_Controls->RespirationRate_bpm = pulse.GetRespiratorySystem()->GetRespirationRate(FrequencyUnit::Per_min);
  m_Controls->EndTidalCarbonDioxidePressure_mmHg = pulse.GetRespiratorySystem()->GetEndTidalCarbonDioxidePressure(PressureUnit::mmHg);
  m_Controls->Temperature_C = pulse.GetEnergySystem()->GetCoreTemperature(TemperatureUnit::C);

  if (m_Controls->CarinaCO2 == nullptr)
  {
    SESubstance* CO2 = pulse.GetSubstanceManager().GetSubstance("CarbonDioxide");
    m_Controls->CarinaCO2 = pulse.GetCompartments().GetGasCompartment(pulse::PulmonaryCompartment::Carina)->GetSubstanceQuantity(*CO2);
  }
  double time_s = pulse.GetSimulationTime(TimeUnit::s);
  m_Controls->ECG_III_Plot->Append(time_s, m_Controls->ECG_III_mV);
  m_Controls->ArterialPressure_Plot->Append(time_s, m_Controls->ArterialPressure_mmHg);
  m_Controls->etCO2_Plot->Append(time_s, m_Controls->CarinaCO2->GetPartialPressure(PressureUnit::mmHg));

  m_Controls->Mutex.unlock();
}

void VitalsMonitorWidget::PulseUpdateUI()
{
  // This is where we take the pulse data we pulled and push it to a UI widget

    m_Controls->Mutex.lock();
    m_Controls->HeartRateValue->setText(QString::number(int(m_Controls->HeartRate_bpm),'d',0));
    m_Controls->BloodPressureValues->setText(QString::number(int(m_Controls->SystolicPressure_mmHg), 'd', 0)+"/"+QString::number(int(m_Controls->DiastolicPressure_mmHg), 'd', 0));
    m_Controls->MeanBloodPressureValue->setText("("+QString::number(int(m_Controls->MeanArterialPressure_mmHg), 'd', 0)+")");
    m_Controls->SpO2Value->setText(QString::number(int(m_Controls->OxygenSaturation*100), 'd', 0));
    m_Controls->etCO2Value->setText(QString::number(int(m_Controls->EndTidalCarbonDioxidePressure_mmHg), 'd', 0));
    m_Controls->RespiratoryRateValue->setText(QString::number(int(m_Controls->RespirationRate_bpm), 'd', 0));
    m_Controls->TempeartureValue->setText(QString::number(m_Controls->Temperature_C, 'd', 1));
    
    m_Controls->ECG_III_Plot->UpdateUI(false); 
    m_Controls->ArterialPressure_Plot->UpdateUI(false);
    m_Controls->etCO2_Plot->UpdateUI(false);
   
    m_Controls->Mutex.unlock();
}
