#include <QApplication>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLayout>
#include <QMainWindow>
#include <QWidget>
#include <QPlainTextEdit>
#include <QFile>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QTimer>

#include <pqActiveObjects.h>
#include <pqAlwaysConnectedBehavior.h>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqPersistentMainWindowStateBehavior.h>
#include <pqContextView.h>
#include <pqXYChartView.h>
#include <pqRenderView.h>

#include <vtkSMProxy.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMReaderFactory.h>

#include "BioGearsDemoMainWindow.h"
#include "ui_BioGearsDemoMainWindow.h"

#include "Scenario.h"

#include "BioGearsPhysiologyEngine.h"
#include "cdm/patient/SEPatient.h"
#include "cdm/system/physiology/SECardiovascularSystem.h"
#include "cdm/system/physiology/SEBloodChemistrySystem.h"
#include "cdm/system/physiology/SERespiratorySystem.h"
#include "cdm/patient/actions/SECardiacArrest.h"
#include "cdm/patient/actions/SEAirwayObstruction.h"
#include "cdm/CommonDataModel.h"
#include "cdm/engine/PhysiologyEngine.h"
#include "cdm/properties/SEScalarTime.h"
#include "cdm/properties/SEScalar0To1.h"
#include "cdm/properties/SEScalarVolume.h"
#include "cdm/properties/SEScalarMass.h"
#include "cdm/properties/SEScalarMassPerVolume.h"
#include "cdm/substance/SESubstanceManager.h"
#include "cdm/patient/actions/SESubstanceBolus.h"

class BioGearsDemoMainWindow::pqInternals : public Ui::BioGearsDemoMainWindow
{
public:
  bool FirstShow;
  QPointer<Scenario> CurrentScenario;

  pqRenderView* MainView;

  pqInternals() :
      FirstShow(true)
  {
  }
};

BioGearsDemoMainWindow::BioGearsDemoMainWindow()
{
  playing = false;

  this->Internals = new pqInternals();
  this->Internals->setupUi(this);

  new pqAlwaysConnectedBehavior(this);
  new pqPersistentMainWindowStateBehavior(this);

  this->Internals->anaphylaxisGroupBox->setVisible(false);
  this->Internals->inputDockWidget->setTitleBarWidget(new QWidget());
  this->Internals->inputDockWidget->show();
  this->Internals->inputDockWidget->raise();

  this->Internals->outputDockWidget->setTitleBarWidget(new QWidget());
  this->Internals->outputDockWidget->show();
  this->Internals->outputDockWidget->raise();

  vtkSMReaderFactory::AddReaderToWhitelist("sources", "XMLPolyDataReader");
  vtkSMReaderFactory::AddReaderToWhitelist("sources", "XMLUnstructuredGridReader");
  pqApplicationCore::instance()->loadConfigurationXML("<xml/>");

  this->Internals->MainView =
      qobject_cast<pqRenderView*>(pqApplicationCore::instance()->getObjectBuilder()->createView(
                                  pqRenderView::renderViewType(),pqActiveObjects::instance().activeServer()));

  this->setCentralWidget(this->Internals->MainView->widget());

  pqXYChartView* chart =
      qobject_cast<pqXYChartView*>(pqApplicationCore::instance()->getObjectBuilder()->createView(
                                   pqXYChartView::XYChartViewType(),pqActiveObjects::instance().activeServer()));

  vtkSMProxy* proxy = chart->getProxy();
  vtkSMPropertyHelper(proxy,"ChartTitle").Set("Test");

//  this->Internals->outputDockWidget->setWidget(chart->widget());
  qobject_cast<QHBoxLayout*>(this->Internals->outputDockWidget->widget()->layout())->insertWidget(0,chart->widget());


  connect(this,SIGNAL(dataChanged()), this, SLOT(updateUI()));
  connect(this->Internals->loadButton, SIGNAL(clicked()),this,SLOT(startEngine()));
  connect(this->Internals->addOstructionButton, SIGNAL(clicked()),this,SLOT(addObstruction()));
  connect(this->Internals->advanceButton, SIGNAL(clicked()), this, SLOT(advanceTime()));
  connect(this->Internals->resetButton, SIGNAL(clicked()), this, SLOT(startEngine()));
  connect(this->Internals->playButton, SIGNAL(clicked()), this, SLOT(playButtonPressed()));
  connect(this->Internals->epiButton, SIGNAL(clicked()), this, SLOT(epiButtonPressed()));

  emit dataChanged();
}

BioGearsDemoMainWindow::~BioGearsDemoMainWindow()
{

}

void BioGearsDemoMainWindow::epiButtonPressed()
{
    const SESubstance* epi = bg->GetSubstanceManager().GetSubstance("Epinephrine");

    SESubstanceBolus bolus(*epi);
    bolus.GetConcentration().SetValue(1,MassPerVolumeUnit::g_Per_L);
    bolus.GetDose().SetValue(0.3,VolumeUnit::mL);
    bolus.SetAdminRoute(CDM::enumBolusAdministration::Intramuscular);

    bg->ProcessAction(bolus);
    bg->GetLogger()->Info("Giving epinephrine");
}

void BioGearsDemoMainWindow::playButtonPressed()
{
    if (!playing)
    {
        playing = true;
        this->Internals->playButton->setText("Pause");

        playTimer = new QTimer(this);
        connect(playTimer, SIGNAL(timeout()), this, SLOT(keepPlaying()));
        playTimer->start( (1000*this->Internals->timeStepBox->value()) / this->Internals->playbackSpeedBox->value() );
    }
    else
    {
        playing = false;
        this->Internals->playButton->setText("Play");

        keepPlaying();
    }

}

void BioGearsDemoMainWindow::pauseButtonPressed()
{
    playing = false;
    this->Internals->playButton->setText("Play");

    keepPlaying();
}

void BioGearsDemoMainWindow::keepPlaying()
{
    if(playing)
        advanceTime();
    else
        playTimer->stop();
}

void BioGearsDemoMainWindow::updateLog()
{
    bg->GetLogger()->Info(std::stringstream() << "Tidal Volume: " << bg->GetRespiratorySystem()->GetTidalVolume(VolumeUnit::mL) << VolumeUnit::mL);
    bg->GetLogger()->Info(std::stringstream() << "O2 Saturation: " << bg->GetBloodChemistrySystem()->GetOxygenSaturation());

    QFile logFile;
    logFile.setFileName("test.log");
    logFile.open(QIODevice::ReadOnly | QIODevice::Text);

    this->Internals->logOutput->clear();

    while (!logFile.atEnd()) {
        QString line(logFile.readLine());
        this->Internals->logOutput->append(line.simplified());
    }
}

void BioGearsDemoMainWindow::startEngine()
{
    this->Internals->CurrentScenario = new Scenario(this->Internals->MainView, this);
    this->Internals->CurrentScenario->loadScenarioData();

    this->Internals->anaphylaxisGroupBox->setVisible(true);

    bg = CreateBioGearsEngine("test.log");
    bg->LoadState("states/DefaultMale@0s.xml");
    bg->GetLogger()->SetLogLevel(log4cpp::Priority::DEBUG);
    bg->GetLogger()->Info("BioGears engine started");

    updateLog();

    emit dataChanged();
}

void BioGearsDemoMainWindow::addObstruction()
{
    SEAirwayObstruction obst;
    obst.GetSeverity().SetValue(this->Internals->severitySlider->value());
    bg->ProcessAction(obst);

    updateLog();
}

void BioGearsDemoMainWindow::advanceTime()
{
    bg->AdvanceModelTime(this->Internals->timeStepBox->value(), TimeUnit::s);

    this->Internals->CurrentScenario->colorData(bg->GetBloodChemistrySystem()->GetOxygenSaturation());

    updateLog();

    emit dataChanged();
}

void BioGearsDemoMainWindow::updateUI()
{
    this->Internals->MainView->render();
}
