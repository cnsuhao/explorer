#include "MultiTraumaShowcaseWidget.h"
#include "ui_MultiTraumaShowcase.h"

#include "cdm/properties/SEScalarTime.h"
#include "cdm/properties/SEScalar0To1.h"
#include "cdm/properties/SEScalarVolume.h"
#include "cdm/properties/SEScalarMassPerVolume.h"
#include "cdm/properties/SEScalarVolumePerTime.h"
#include "cdm/substance/SESubstance.h"
#include "cdm/substance/SESubstanceManager.h"
#include "cdm/patient/actions/SESubstanceBolus.h"
#include "cdm/patient/actions/SESubstanceCompoundInfusion.h"
#include "cdm/patient/actions/SEHemorrhage.h"
#include "cdm/patient/actions/SETensionPneumothorax.h"
#include "cdm/patient/actions/SENeedleDecompression.h"

class MultiTraumaShowcaseWidget::Controls : public Ui::MultiTraumaShowcaseWidget
{
public:
  Controls(QPulse& qp, GeometryView& g) : Pulse(qp), Geometry(g) {}
  GeometryView&        Geometry;
  QPulse&              Pulse;
  double               HemorrhageRate_mL_Per_min=0;
  double               PneumothoraxSeverity = 0;
  cdm::eSide           PneumothoraxSide;
  bool                 ApplyHemorrhage = false;
  bool                 ApplyPneumothorax = false;
  bool                 ApplyPressure = false;
  bool                 ApplyNeedleDecompression = false;
  bool                 ApplyTourniquet = false;
  bool                 InfuseSaline = false;
  bool                 InjectMorphine = false;
};

MultiTraumaShowcaseWidget::MultiTraumaShowcaseWidget(QPulse& qp, GeometryView& g, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls(qp,g);
  m_Controls->setupUi(this);

  m_Controls->FlowRateEdit->setValidator(new QDoubleValidator(0, 500, 1, this));

  connect(this, SIGNAL(UIChanged()), this, SLOT(UpdateUI()));
  connect(this, SIGNAL(PulseChanged()), this, SLOT(PulseUpdate()));

  connect(m_Controls->ApplyHemorrhageButton, SIGNAL(clicked()), this, SLOT(ApplyHemorrhage()));
  connect(m_Controls->ApplyPneumoButton, SIGNAL(clicked()), this, SLOT(ApplyPneumothorax()));
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
  m_Controls->HemorrhageRate_mL_Per_min = 0;
  m_Controls->PneumothoraxSeverity = 0;
  m_Controls->PneumothoraxSide;
  m_Controls->ApplyHemorrhage = false;
  m_Controls->ApplyPneumothorax = false;
  m_Controls->ApplyPressure = false;
  m_Controls->ApplyNeedleDecompression = false;
  m_Controls->ApplyTourniquet = false;
  m_Controls->InfuseSaline = false;
  m_Controls->InjectMorphine = false;

  m_Controls->ApplyHemorrhageButton->setEnabled(true);
  m_Controls->FlowRateEdit->setEnabled(true);
  m_Controls->ApplyPneumoButton->setEnabled(true);
  m_Controls->SeveritySlider->setEnabled(true);
  m_Controls->PneumothoraxTypeCombo->setEnabled(true);
  m_Controls->ApplyPressureButton->setEnabled(false);
  m_Controls->NeedleDecompressButton->setEnabled(false);
  m_Controls->ApplyTournyButton->setEnabled(false);
  m_Controls->InfuseSalineButton->setEnabled(false);
  m_Controls->InjectMorphineButton->setEnabled(false);

  if(!pulse.LoadStateFile("states/Soldier@0s.pba"))
    throw CommonDataModelException("Unable to load state file");
  m_Controls->Pulse.GetLogBox().append("Combining the tension pneumothorax with the blood loss from the hemorrhage pushes and eventually exceeds the limits of the homeostatic control mechanisms.");
  // Fill out any data requsts that we want to have plotted
  drMgr.CreatePhysiologyDataRequest("BloodVolume",VolumeUnit::L);
  const SESubstance* Morphine = pulse.GetSubstanceManager().GetSubstance("Morphine");
  drMgr.CreateSubstanceDataRequest(*Morphine, "PlasmaConcentration", MassPerVolumeUnit::mg_Per_mL);
}

void MultiTraumaShowcaseWidget::ProcessPhysiology(PhysiologyEngine& pulse)
{
  // This is where we pull data from pulse, and push any actions to it

  if (m_Controls->ApplyPneumothorax)
  {
    m_Controls->ApplyPneumothorax = false;
    SETensionPneumothorax pneumothorax;
    pneumothorax.SetSide(m_Controls->PneumothoraxSide);
    pneumothorax.SetType(cdm::eGate::Closed);
    pneumothorax.GetSeverity().SetValue(m_Controls->PneumothoraxSeverity);
    pulse.ProcessAction(pneumothorax);
  }
  if (m_Controls->ApplyNeedleDecompression)
  {
    m_Controls->ApplyNeedleDecompression = false;
    SENeedleDecompression needle;
    needle.SetActive(true);
    needle.SetSide(m_Controls->PneumothoraxSide);
    pulse.ProcessAction(needle);
  }

  if (m_Controls->ApplyHemorrhage)
  {
    m_Controls->ApplyHemorrhage = false;
    SEHemorrhage hemorrhage;
    hemorrhage.GetRate().SetValue(m_Controls->HemorrhageRate_mL_Per_min, VolumePerTimeUnit::mL_Per_min);
    hemorrhage.SetCompartment(pulse::VascularCompartment::RightLeg);
    pulse.ProcessAction(hemorrhage);
  }
  if (m_Controls->ApplyPressure)
  {
    m_Controls->ApplyPressure = false;
    SEHemorrhage hemorrhage;
    hemorrhage.GetRate().SetValue(m_Controls->HemorrhageRate_mL_Per_min*0.15, VolumePerTimeUnit::mL_Per_min);
    hemorrhage.SetCompartment(pulse::VascularCompartment::RightLeg);
    pulse.ProcessAction(hemorrhage);
  }
  if (m_Controls->ApplyTourniquet)
  {
    m_Controls->ApplyTourniquet = false;
    SEHemorrhage hemorrhage;
    hemorrhage.GetRate().SetValue(0,VolumePerTimeUnit::mL_Per_min);
    hemorrhage.SetCompartment(pulse::VascularCompartment::RightLeg);
    pulse.ProcessAction(hemorrhage);
  }

  if (m_Controls->InfuseSaline)
  {
    m_Controls->InfuseSaline = false;
    const SESubstanceCompound* Saline = pulse.GetSubstanceManager().GetCompound("Saline");
    SESubstanceCompoundInfusion   SalineInfusion(*Saline);
    SalineInfusion.GetBagVolume().SetValue(500, VolumeUnit::mL);
    SalineInfusion.GetRate().SetValue(100, VolumePerTimeUnit::mL_Per_min);
    pulse.ProcessAction(SalineInfusion);
  }

  if (m_Controls->InjectMorphine)
  {
    m_Controls->InjectMorphine = false;
    const SESubstance* Morphine = pulse.GetSubstanceManager().GetSubstance("Morphine");
    SESubstanceBolus   MorphineBolus(*Morphine);
    MorphineBolus.GetConcentration().SetValue(1000, MassPerVolumeUnit::ug_Per_mL);
    MorphineBolus.GetDose().SetValue(5.0, VolumeUnit::mL);
    MorphineBolus.SetAdminRoute(cdm::SubstanceBolusData_eAdministrationRoute_Intravenous);
    pulse.ProcessAction(MorphineBolus);
  }
  
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
  m_Controls->HemorrhageRate_mL_Per_min = m_Controls->FlowRateEdit->text().toDouble();
  m_Controls->ApplyHemorrhage = true;
  m_Controls->ApplyHemorrhageButton->setDisabled(true);
  m_Controls->FlowRateEdit->setDisabled(true);
  m_Controls->ApplyPressureButton->setEnabled(true);
  m_Controls->Pulse.GetLogBox().append("Appling hemorrhage");
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::ApplyPneumothorax()
{
  m_Controls->PneumothoraxSide = m_Controls->PneumothoraxTypeCombo->currentIndex()==0?cdm::eSide::Left:cdm::eSide::Right;
  m_Controls->PneumothoraxSeverity = m_Controls->SeveritySlider->value();
  m_Controls->ApplyPneumothorax = true;
  m_Controls->ApplyPneumoButton->setDisabled(true);
  m_Controls->SeveritySlider->setDisabled(true);
  m_Controls->PneumothoraxTypeCombo->setDisabled(true);
  m_Controls->NeedleDecompressButton->setEnabled(true);
  m_Controls->Pulse.GetLogBox().append("Appling Pneumothorax");
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::ApplyPressure()
{
  m_Controls->ApplyPressure = true;
  m_Controls->ApplyPressureButton->setDisabled(true);
  m_Controls->ApplyTournyButton->setEnabled(true);
  m_Controls->Pulse.GetLogBox().append("Appling pressure to the wound");
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::ApplyNeedleDecompression()
{
  m_Controls->ApplyNeedleDecompression = true;
  m_Controls->NeedleDecompressButton->setEnabled(false);
  m_Controls->Pulse.GetLogBox().append("Appling Needle Decompression");
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::ApplyTourniquet()
{
  m_Controls->ApplyTourniquet = true;
  m_Controls->ApplyTournyButton->setEnabled(false);
  m_Controls->InfuseSalineButton->setEnabled(true);
  m_Controls->InjectMorphineButton->setEnabled(true);
  m_Controls->Pulse.GetLogBox().append("Appling Tourniquet");
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::InfuseSaline()
{
  m_Controls->InfuseSaline = true;
  m_Controls->InfuseSalineButton->setEnabled(false);
  m_Controls->Pulse.GetLogBox().append("Infusing saline");
  emit UIChanged();
}

void MultiTraumaShowcaseWidget::InjectMorphine()
{
  m_Controls->InjectMorphine = true;
  m_Controls->InjectMorphineButton->setEnabled(false);
  m_Controls->Pulse.GetLogBox().append("Injecting a bolus of morphine");
  emit UIChanged();
}
