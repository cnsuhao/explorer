#include "VitalsMonitorWidget.h"
#include <QMutex>

#include "cdm/utils/FileUtils.h"

#include "cdm/system/physiology/SEBloodChemistrySystem.h"
#include "cdm/system/physiology/SECardiovascularSystem.h"
#include "cdm/system/physiology/SERespiratorySystem.h"
#include "cdm/system/equipment/electrocardiogram/SEElectroCardioGram.h"
#include "cdm/properties/SEScalarFrequency.h"
#include "cdm/properties/SEScalarPressure.h"
#include "cdm/Properties/SEScalarElectricPotential.h"

class VitalsMonitorWidget::Controls
{
public:
  Controls(QTextEdit& log) : LogBox(log) {}
  QTextEdit&  LogBox;
  double      HeartRate_bpm;
  double      ECG_III_mV;
  double      ArterialPressure_mmHg;
  double      DiastolicPressure_mmHg;
  double      SystolicPressure_mmHg;
  double      OxygenSaturation;
  double      PulseOximetry;
  double      RespirationRate_bpm;
  QMutex      Mutex;
  int         Count = 0;//Just outputting data to the log every 5 seconds, take out when plots are working
};

VitalsMonitorWidget::VitalsMonitorWidget(QTextEdit& log, QWidget *parent, Qt::WindowFlags flags) : QWidget(parent,flags)
{
  m_Controls = new Controls(log);

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
  m_Controls->DiastolicPressure_mmHg = pulse.GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg);
  m_Controls->SystolicPressure_mmHg = pulse.GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg);
  m_Controls->OxygenSaturation = pulse.GetBloodChemistrySystem()->GetOxygenSaturation();
  m_Controls->PulseOximetry = pulse.GetBloodChemistrySystem()->GetPulseOximetry();
  m_Controls->RespirationRate_bpm = pulse.GetRespiratorySystem()->GetRespirationRate(FrequencyUnit::Per_min);
  m_Controls->Mutex.unlock();

  emit PulseChanged(); // Call this if you need to update the UI with data from pulse
}

void VitalsMonitorWidget::UpdateUI()
{

}

void VitalsMonitorWidget::PulseUpdate()
{
  // This is where we take the pulse data we pulled and push it to a UI widget
  if (++m_Controls->Count == 50)
  {
    m_Controls->Count = 0;
    m_Controls->Mutex.lock();
    m_Controls->LogBox.append(QString("Heart Rate : %1 bpm").arg(m_Controls->HeartRate_bpm));
    m_Controls->LogBox.append(QString("ECG III : %1 mV").arg(m_Controls->ECG_III_mV));
    m_Controls->LogBox.append(QString("Arterial Pressure : %1 mmHg").arg(m_Controls->ArterialPressure_mmHg));
    m_Controls->LogBox.append(QString("Diastolic Pressure : %1 mmHg").arg(m_Controls->DiastolicPressure_mmHg));
    m_Controls->LogBox.append(QString("Systolic Pressure : %1 mmHg").arg(m_Controls->SystolicPressure_mmHg));
    m_Controls->LogBox.append(QString("OxygenSaturation : %1").arg(m_Controls->OxygenSaturation));
    m_Controls->LogBox.append(QString("Pulse Oximetry : %1").arg(m_Controls->PulseOximetry));
    m_Controls->LogBox.append(QString("Respiration Rate : %1 bpm").arg(m_Controls->RespirationRate_bpm));
    m_Controls->Mutex.unlock();
  }
}
