#include "MultiTraumaShowcaseWidget.h"
#include "ui_MultiTraumaShowcase.h"

#include "cdm/properties/SEScalarTime.h"
#include "cdm/properties/SEScalar0To1.h"
#include "cdm/properties/SEScalarVolume.h"
#include "cdm/properties/SEScalarMassPerVolume.h"
#include "cdm/substance/SESubstance.h"
#include "cdm/substance/SESubstanceManager.h"
#include "cdm/patient/actions/SESubstanceBolus.h"
#include "cdm/patient/actions/SEAirwayObstruction.h"

class MultiTraumaShowcaseWidget::Controls : public Ui::MultiTraumaShowcaseWidget
{
public:
  Controls(QTextEdit& log) : LogBox(log) {}
  QTextEdit&           LogBox;
  bool                 ApplyHemorrhage = false;
  bool                 ApplyPnumothorax = false;
  bool                 ApplyPressure = false;
  bool                 ApplyNeedleDecompression = false;
  bool                 ApplyTourniquet = false;
  bool                 InfuseSaline = false;
  bool                 InjectMorphine = false;
};

MultiTraumaShowcaseWidget::MultiTraumaShowcaseWidget(QTextEdit& log, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls(log);
  m_Controls->setupUi(this);

  m_Controls->FlowRateEdit->setValidator(new QDoubleValidator(0, 500, 1, this));

  connect(this, SIGNAL(UIChanged()), this, SLOT(UpdateUI()));
  connect(this, SIGNAL(PulseChanged()), this, SLOT(PulseUpdate()));

  connect(m_Controls->ApplyHemorrhageButton, SIGNAL(clicked()), this, SLOT(ApplyHemorrhage()));
  connect(m_Controls->ApplyPnumoButton, SIGNAL(clicked()), this, SLOT(ApplyPnumothorax()));
  connect(m_Controls->ApplyPressureButton, SIGNAL(clicked()), this, SLOT(ApplyPressure()));
  connect(m_Controls->NeedleDecompressButton, SIGNAL(clicked()), this, SLOT(ApplyNeedleDecompression()));
  connect(m_Controls->ApplyTournyButton, SIGNAL(clicked()), this, SLOT(ApplyTourniquet()));
  connect(m_Controls->InfuseSalineButton, SIGNAL(clicked()), this, SLOT(InfuseSaline()));
  connect(m_Controls->InjectMorphineButton, SIGNAL(clicked()), this, SLOT(InjectMorphine()));
  emit UIChanged();
}

MultiTraumaShowcaseWidget::~MultiTraumaShowcaseWidget()
{
  delete m_Controls;
}

void MultiTraumaShowcaseWidget::ConfigurePulse(PhysiologyEngine& pulse, SEDataRequestManager& drMgr)
{

  m_Controls->ApplyHemorrhageButton->setEnabled(true);
  m_Controls->FlowRateEdit->setEnabled(true);
  m_Controls->ApplyPnumoButton->setEnabled(true);
  m_Controls->SeveritySlider->setEnabled(true);
  m_Controls->PnumothoraxTypeCombo->setEnabled(true);
  m_Controls->ApplyPressureButton->setEnabled(false);
  m_Controls->NeedleDecompressButton->setEnabled(false);
  m_Controls->ApplyTournyButton->setEnabled(false);
  m_Controls->InfuseSalineButton->setEnabled(false);
  m_Controls->InjectMorphineButton->setEnabled(false);

  pulse.LoadStateFile("states/Soldier@0s.pba");
  m_Controls->LogBox.append("Combining the tension pneumothorax with the blood loss from the hemorrhage pushes and eventually exceeds the limits of the homeostatic control mechanisms.");
  // FIll out any data requsts that we want to have plotted
  drMgr.CreatePhysiologyDataRequest("BloodVolume",VolumeUnit::L);
}

void MultiTraumaShowcaseWidget::ProcessPhysiology(PhysiologyEngine& pulse)
{
  //// This is where we pull data from pulse, and push any actions to it
  //if (m_Controls->ApplyAirwayObstruction)
  //{
  //  m_Controls->ApplyAirwayObstruction = false;
  //  SEAirwayObstruction AirwayObstuction;
  //  AirwayObstuction.GetSeverity().SetValue(m_Controls->SeveritySlider->value());
  //  pulse.ProcessAction(AirwayObstuction);
  //  m_Controls->LogBox.append("Applying MultiTrauma");
  //}
  //if (m_Controls->InjectEpinephrine)
  //{
  //  m_Controls->InjectEpinephrine = false;
  //  m_Controls->ReduceAirwayObstruction = true;
  //  const SESubstance* Epinephrine = pulse.GetSubstanceManager().GetSubstance("Epinephrine");
  //  SESubstanceBolus   EpinephrineBolus(*Epinephrine);
  //  EpinephrineBolus.GetConcentration().SetValue(1, MassPerVolumeUnit::g_Per_L);
  //  EpinephrineBolus.GetDose().SetValue(0.3, VolumeUnit::mL);
  //  EpinephrineBolus.SetAdminRoute(cdm::SubstanceBolusData_eAdministrationRoute_Intravenous);
  //  pulse.ProcessAction(EpinephrineBolus);
  //  m_Controls->LogBox.append("Injecting a bolus of epinephrine");
  //}
  //if (m_Controls->InjectEpinephrine)
  //{

  //}
  // We don't need to update the GUI
  //emit PulseChanged(); // Call this if you need to update the UI with data from pulse
}

void MultiTraumaShowcaseWidget::UpdateUI()
{
  
}

void MultiTraumaShowcaseWidget::PulseUpdate()
{
 
}

void MultiTraumaShowcaseWidget::ApplyHemorrhage()
{
  m_Controls->ApplyHemorrhage = true;
  m_Controls->ApplyHemorrhageButton->setDisabled(true);
  m_Controls->FlowRateEdit->setDisabled(true);
  m_Controls->ApplyPressureButton->setEnabled(true);
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::ApplyPnumothorax()
{
  m_Controls->ApplyPnumothorax = true;
  m_Controls->ApplyPnumoButton->setDisabled(true);
  m_Controls->SeveritySlider->setDisabled(true);
  m_Controls->PnumothoraxTypeCombo->setDisabled(true);
  m_Controls->NeedleDecompressButton->setEnabled(true);
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::ApplyPressure()
{
  m_Controls->ApplyPressure = true;
  m_Controls->ApplyPressureButton->setDisabled(true);
  m_Controls->ApplyTournyButton->setEnabled(true);
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::ApplyNeedleDecompression()
{
  m_Controls->ApplyNeedleDecompression = true;
  m_Controls->NeedleDecompressButton->setEnabled(false);
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::ApplyTourniquet()
{
  m_Controls->ApplyTourniquet = true;
  m_Controls->ApplyTournyButton->setEnabled(false);
  m_Controls->InfuseSalineButton->setEnabled(true);
  m_Controls->InjectMorphineButton->setEnabled(true);
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::InfuseSaline()
{
  m_Controls->InfuseSaline = true;
  m_Controls->InfuseSalineButton->setEnabled(false);
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::InjectMorphine()
{
  m_Controls->InjectMorphine = true;
  m_Controls->InjectMorphineButton->setEnabled(false);
  emit UIChanged();
}
