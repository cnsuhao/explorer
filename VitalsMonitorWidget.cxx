#include "VitalsMonitorWidget.h"
#include "ui_VitalsMonitor.h"
#include <QMutex>

#include "cdm/utils/FileUtils.h"

#include "cdm/system/physiology/SEBloodChemistrySystem.h"
#include "cdm/system/physiology/SECardiovascularSystem.h"
#include "cdm/system/physiology/SERespiratorySystem.h"
#include "cdm/system/physiology/SEEnergySystem.h"
#include "cdm/system/equipment/electrocardiogram/SEElectroCardioGram.h"
#include "cdm/properties/SEScalarFrequency.h"
#include "cdm/properties/SEScalarPressure.h"
#include "cdm/Properties/SEScalarElectricPotential.h"
#include "cdm/Properties/SEScalarTemperature.h"

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
  int         Count = 0;//Just outputting data to the log every 5 seconds, take out when plots are working
};

VitalsMonitorWidget::VitalsMonitorWidget(QTextEdit& log, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls(log);
  m_Controls->setupUi(this);

  connect(this, SIGNAL(UIChanged()), this, SLOT(UpdateUI()));
  connect(this, SIGNAL(PulseChanged()), this, SLOT(PulseUpdate()));
}

VitalsMonitorWidget::~VitalsMonitorWidget()
{
  delete m_Controls;
}

void VitalsMonitorWidget::Reset()
{
  m_Controls->Count = 0;
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
  m_Controls->Mutex.unlock();

  emit PulseChanged(); // Call this if you need to update the UI with data from pulse
}

void VitalsMonitorWidget::UpdateUI()
{

}

void VitalsMonitorWidget::PulseUpdate()
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
    // Throw this data to the Graphs
    //m_Controls->ECGGraph.append(m_Controls->ECG_III_mV);
    //m_Controls->APBGraph.append(m_Controls->ArterialPressure_mmHg);
    //m_Controls->CO2Graph.append(m_Controls->EndTidalCarbonDioxidePressure_mmHg);
    m_Controls->Mutex.unlock();
}
