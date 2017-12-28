#include "AnaphylaxisShowcaseWidget.h"
#include "ui_AnaphylaxisShowcase.h"

#include <QMutex>

#include "cdm/CommonDataModel.h"
#include "PulsePhysiologyEngine.h"
#include "cdm/scenario/SEDataRequestManager.h"
#include "cdm/properties/SEScalarTime.h"
#include "cdm/properties/SEScalar0To1.h"
#include "cdm/properties/SEScalarVolume.h"
#include "cdm/properties/SEScalarMassPerVolume.h"
#include "cdm/substance/SESubstance.h"
#include "cdm/substance/SESubstanceManager.h"
#include "cdm/patient/actions/SESubstanceBolus.h"
#include "cdm/patient/actions/SEAirwayObstruction.h"
#include "cdm/system/physiology/SEBloodChemistrySystem.h"

class AnaphylaxisShowcaseWidget::Controls : public Ui::AnaphylaxisShowcaseWidget
{
public:
  Controls(QPulse& qp) : Pulse(qp) {}
  QPulse&              Pulse;
  bool                 ApplyAirwayObstruction=false;
  bool                 InjectEpinephrine=false;
  bool                 ReduceAirwayObstruction=false;
  double               SpO2;
  double               ReduceRatio = 0.1 / 60; // Amount to reduce obstruction size per second.
  QMutex               Mutex;
};

AnaphylaxisShowcaseWidget::AnaphylaxisShowcaseWidget(QPulse& qp, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls(qp);
  m_Controls->setupUi(this);

  connect(m_Controls->ObsButton, SIGNAL(clicked()), this, SLOT(ApplyAirwayObstruction()));
  connect(m_Controls->EpiButton, SIGNAL(clicked()), this, SLOT(InjectEpinephrine()));
}

AnaphylaxisShowcaseWidget::~AnaphylaxisShowcaseWidget()
{
  delete m_Controls;
}

void AnaphylaxisShowcaseWidget::ConfigurePulse(PhysiologyEngine& pulse, SEDataRequestManager& drMgr)
{
  m_Controls->SeveritySlider->setEnabled(true);
  m_Controls->ObsButton->setEnabled(true);
  m_Controls->EpiButton->setEnabled(false);
  m_Controls->ApplyAirwayObstruction = false;
  m_Controls->InjectEpinephrine = false;
  m_Controls->ReduceAirwayObstruction = false;

  if (!pulse.LoadStateFile("./states/StandardMale@0s.pba"))
    throw CommonDataModelException("Unable to load state file");
  m_Controls->Pulse.GetLogBox().append("Anaphylaxis is a serious, potentially life threatening allergic reaction with facial and airway swelling.");
  m_Controls->Pulse.GetLogBox().append("It is an immune response that can occur quickly in response to exposure to an allergen.");
  m_Controls->Pulse.GetLogBox().append("The immune system releases chemicals into the body that cause the blood pressure to drop and the airways to narrow, blocking breathing.");
  m_Controls->Pulse.GetLogBox().append("Anaphylaxis is treated with an injection of epinephrine.");
  m_Controls->Pulse.GetLogBox().append("The anaphylaxis is rapidly reversed by the drug, allowing patient vital signs to return to normal.");
  // Fill out any data requsts that we want to have plotted
  const SESubstance* Epinephrine = pulse.GetSubstanceManager().GetSubstance("Epinephrine");
  drMgr.CreateSubstanceDataRequest(*Epinephrine,"PlasmaConcentration",MassPerVolumeUnit::mg_Per_mL);
}

void AnaphylaxisShowcaseWidget::ProcessPhysiology(PhysiologyEngine& pulse)
{
  m_Controls->Mutex.lock();
  // This is where we pull data from pulse, and push any actions to it
  if (m_Controls->ApplyAirwayObstruction)
  {
    m_Controls->ApplyAirwayObstruction = false;
    SEAirwayObstruction AirwayObstuction;
    AirwayObstuction.GetSeverity().SetValue(m_Controls->SeveritySlider->value());
    pulse.ProcessAction(AirwayObstuction);
    
  }
  if (m_Controls->InjectEpinephrine)
  {
    m_Controls->InjectEpinephrine = false;
    m_Controls->ReduceAirwayObstruction = true;
    const SESubstance* Epinephrine = pulse.GetSubstanceManager().GetSubstance("Epinephrine");
    SESubstanceBolus   EpinephrineBolus(*Epinephrine);
    EpinephrineBolus.GetConcentration().SetValue(1, MassPerVolumeUnit::g_Per_L);
    EpinephrineBolus.GetDose().SetValue(0.3, VolumeUnit::mL);
    EpinephrineBolus.SetAdminRoute(cdm::SubstanceBolusData_eAdministrationRoute_Intravenous);
    pulse.ProcessAction(EpinephrineBolus);
    m_Controls->Pulse.IgnoreAction("Airway Obstruction");
  }
  if (m_Controls->ReduceAirwayObstruction)
  {
    SEAirwayObstruction AirwayObstuction;
    double new_severity = m_Controls->SeveritySlider->value() - (m_Controls->ReduceRatio * m_Controls->Pulse.GetTimeStep_s());
    if (new_severity <= 0)
    {
      new_severity = 0;
      m_Controls->ReduceAirwayObstruction = false;
    }
    m_Controls->SeveritySlider->setValue(new_severity);
    AirwayObstuction.GetSeverity().SetValue(new_severity);
    pulse.ProcessAction(AirwayObstuction);
  }
  m_Controls->Mutex.unlock();
}

void AnaphylaxisShowcaseWidget::ApplyAirwayObstruction()
{
  m_Controls->Mutex.lock();
  m_Controls->ApplyAirwayObstruction = true;
  m_Controls->SeveritySlider->setEnabled(false);
  m_Controls->ObsButton->setEnabled(false);
  m_Controls->EpiButton->setEnabled(true);
  m_Controls->Pulse.GetLogBox().append("Applying anaphylaxis");
  m_Controls->Mutex.unlock();
}

void AnaphylaxisShowcaseWidget::InjectEpinephrine()
{
  m_Controls->Mutex.lock();
  m_Controls->InjectEpinephrine = true;
  m_Controls->EpiButton->setEnabled(false);
  m_Controls->Pulse.GetLogBox().append("Injecting a bolus of epinephrine");
  m_Controls->Mutex.unlock();
}
