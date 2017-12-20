#include "AnaphylaxisShowcaseWidget.h"
#include "ui_AnaphylaxisShowcase.h"


#include "cdm/properties/SEScalarTime.h"
#include "cdm/properties/SEScalar0To1.h"
#include "cdm/properties/SEScalarVolume.h"
#include "cdm/properties/SEScalarMassPerVolume.h"
#include "cdm/substance/SESubstance.h"
#include "cdm/substance/SESubstanceManager.h"
#include "cdm/patient/actions/SESubstanceBolus.h"
#include "cdm/patient/actions/SEAirwayObstruction.h"

class AnaphylaxisShowcaseWidget::Controls : public Ui::AnaphylaxisShowcaseWidget
{
public:
  Controls(QTextEdit& log) : LogBox(log) {}
  QTextEdit&           LogBox;
  bool                 ApplyAirwayObstruction=false;
  bool                 InjectEpinephrine=false;
  bool                 ReduceAirwayObstruction=false;
  double               ReduceRatio = 0.1 / 60; // Amount to reduce obstruction size per second.
};

AnaphylaxisShowcaseWidget::AnaphylaxisShowcaseWidget(QTextEdit& log, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls(log);
  m_Controls->setupUi(this);

  connect(this, SIGNAL(UIChanged()), this, SLOT(UpdateUI()));
  connect(this, SIGNAL(PulseChanged()), this, SLOT(PulseUpdate()));

  connect(m_Controls->ObsButton, SIGNAL(clicked()), this, SLOT(ApplyAirwayObstruction()));
  connect(m_Controls->EpiButton, SIGNAL(clicked()), this, SLOT(InjectEpinephrine()));
  emit UIChanged();
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

  if (!pulse.LoadStateFile("states/StandardMale@0s.pba"))
    throw CommonDataModelException("Unable to load state file");
  m_Controls->LogBox.append("Anaphylaxis is a serious, potentially life threatening allergic reaction with facial and airway swelling.");
  m_Controls->LogBox.append("It is an immune response that can occur quickly in response to exposure to an allergen.");
  m_Controls->LogBox.append("The immune system releases chemicals into the body that cause the blood pressure to drop and the airways to narrow, blocking breathing.");
  m_Controls->LogBox.append("Anaphylaxis is treated with an injection of epinephrine.");
  m_Controls->LogBox.append("The anaphylaxis is rapidly reversed by the drug, allowing patient vital signs to return to normal.");
  // FIll out any data requsts that we want to have plotted
  const SESubstance* Epinephrine = pulse.GetSubstanceManager().GetSubstance("Epinephrine");
  drMgr.CreateSubstanceDataRequest(*Epinephrine,"PlasmaConcentration",MassPerVolumeUnit::mg_Per_mL);
}

void AnaphylaxisShowcaseWidget::ProcessPhysiology(PhysiologyEngine& pulse)
{
  // This is where we pull data from pulse, and push any actions to it
  if (m_Controls->ApplyAirwayObstruction)
  {
    m_Controls->ApplyAirwayObstruction = false;
    SEAirwayObstruction AirwayObstuction;
    AirwayObstuction.GetSeverity().SetValue(m_Controls->SeveritySlider->value());
    pulse.ProcessAction(AirwayObstuction);
    m_Controls->LogBox.append("Applying anaphylaxis");
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
    m_Controls->LogBox.append("Injecting a bolus of epinephrine");
  }
  if (m_Controls->ReduceAirwayObstruction)
  {

  }
  // We don't need to update the GUI
  //emit PulseChanged(); // Call this if you need to update the UI with data from pulse
}

void AnaphylaxisShowcaseWidget::UpdateUI()
{
  
}

void AnaphylaxisShowcaseWidget::PulseUpdate()
{
 
}

void AnaphylaxisShowcaseWidget::ApplyAirwayObstruction()
{
  m_Controls->ApplyAirwayObstruction = true;
  m_Controls->ObsButton->setEnabled(false);
  m_Controls->EpiButton->setEnabled(true);
  emit UIChanged();
}

void AnaphylaxisShowcaseWidget::InjectEpinephrine()
{
  m_Controls->InjectEpinephrine = true;
  m_Controls->EpiButton->setEnabled(false);
  emit UIChanged();
}
