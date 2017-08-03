#include <QApplication>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLayout>
#include <QMainWindow>
#include <QWidget>
#include <QPlainTextEdit>
#include <QFile>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QTimer>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

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

#include "PhysiologyExplorerMainWindow.h"
#include "ui_PhysiologyExplorerMainWindow.h"

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
#include "cdm/properties/SEScalarFrequency.h"
#include "cdm/substance/SESubstance.h"
#include "cdm/substance/SESubstanceManager.h"
#include "cdm/patient/actions/SESubstanceBolus.h"

class PhysiologyExplorerMainWindow::pqInternals : public Ui::PhysiologyExplorerMainWindow
{
public:
  bool FirstShow;
  QPointer<Scenario> CurrentScenario;

  QPointer<QtCharts::QLineSeries> o2series;
  QPointer<QtCharts::QChart> o2chart;
  QPointer<QtCharts::QChartView> o2chartView;

  QPointer<QtCharts::QLineSeries> volseries;
  QPointer<QtCharts::QChart> volchart;
  QPointer<QtCharts::QChartView> volchartView;

  double minO2 = 0.8;

  double lastObstSize;
  bool reduceObstruction = false;
  double reduceObstRatio = 0.1/60; // Amount to reduce obstruction size per second.

  pqRenderView* MainView;

  pqInternals() :
      FirstShow(true)
  {
  }
};

PhysiologyExplorerMainWindow::PhysiologyExplorerMainWindow()
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

  this->Internals->o2series = new QtCharts::QLineSeries();

  this->Internals->o2chart = new QtCharts::QChart();
  this->Internals->o2chart->legend()->hide();
  this->Internals->o2chart->addSeries(this->Internals->o2series);
  this->Internals->o2chart->createDefaultAxes();
  this->Internals->o2chart->axisY()->setRange(0,0.8);
  this->Internals->o2chart->setTitle("Oxygen Saturation");

  this->Internals->o2chartView = new QtCharts::QChartView(this->Internals->o2chart);
  this->Internals->o2chartView->setRenderHint(QPainter::Antialiasing);
  this->Internals->o2chartView->setVisible(true);

  qobject_cast<QGridLayout*>(this->Internals->outputDockWidget->widget()->layout())->addWidget(this->Internals->o2chartView,0,0,1,1);

  this->Internals->volseries = new QtCharts::QLineSeries();

  this->Internals->volchart = new QtCharts::QChart();
  this->Internals->volchart->legend()->hide();
  this->Internals->volchart->addSeries(this->Internals->volseries);
  this->Internals->volchart->createDefaultAxes();
  this->Internals->volchart->axisY()->setRange(0,600);
  this->Internals->volchart->setTitle("Tidal Volume");

  this->Internals->volchartView = new QtCharts::QChartView(this->Internals->volchart);
  this->Internals->volchartView->setRenderHint(QPainter::Antialiasing);
  this->Internals->volchartView->setVisible(true);

  qobject_cast<QGridLayout*>(this->Internals->outputDockWidget->widget()->layout())->addWidget(this->Internals->volchartView,0,1,1,1);

  this->Internals->outputDockWidget->setVisible(false);

  connect(this,SIGNAL(dataChanged()), this, SLOT(updateUI()));
  connect(this->Internals->loadButton, SIGNAL(clicked()),this,SLOT(startEngine()));
  connect(this->Internals->addOstructionButton, SIGNAL(clicked()),this,SLOT(addObstruction()));
  connect(this->Internals->advanceButton, SIGNAL(clicked()), this, SLOT(advanceTime()));
  connect(this->Internals->resetButton, SIGNAL(clicked()), this, SLOT(startEngine()));
  connect(this->Internals->playButton, SIGNAL(clicked()), this, SLOT(playButtonPressed()));
  connect(this->Internals->epiButton, SIGNAL(clicked()), this, SLOT(epiButtonPressed()));

  emit dataChanged();
}

PhysiologyExplorerMainWindow::~PhysiologyExplorerMainWindow()
{

}

void PhysiologyExplorerMainWindow::epiButtonPressed()
{
    const SESubstance* epi = bg->GetSubstanceManager().GetSubstance("Epinephrine");

    SESubstanceBolus bolus(*epi);
    bolus.GetConcentration().SetValue(1,MassPerVolumeUnit::g_Per_L);
    bolus.GetDose().SetValue(0.3,VolumeUnit::mL);
    bolus.SetAdminRoute(CDM::enumBolusAdministration::Intramuscular);

    bg->ProcessAction(bolus);
    bg->GetLogger()->Info("Giving epinephrine");

    this->Internals->reduceObstruction = true;
    this->Internals->lastObstSize = this->Internals->severitySlider->value(); 
}

void PhysiologyExplorerMainWindow::playButtonPressed()
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

void PhysiologyExplorerMainWindow::pauseButtonPressed()
{
    playing = false;
    this->Internals->playButton->setText("Play");

    keepPlaying();
}

void PhysiologyExplorerMainWindow::keepPlaying()
{
    if(playing)
        advanceTime();
    else
        playTimer->stop();
}

void PhysiologyExplorerMainWindow::updateLog()
{
    bg->GetLogger()->Info(std::stringstream() << "Tidal Volume: " << bg->GetRespiratorySystem()->GetTidalVolume(VolumeUnit::mL) << VolumeUnit::mL);
    bg->GetLogger()->Info(std::stringstream() << "Total Volume: " << bg->GetRespiratorySystem()->GetTotalLungVolume(VolumeUnit::mL) << VolumeUnit::mL);
    bg->GetLogger()->Info(std::stringstream() << "O2 Saturation: " << bg->GetBloodChemistrySystem()->GetOxygenSaturation());
    bg->GetLogger()->Info(std::stringstream() << "Respiratory Rate: " << bg->GetRespiratorySystem()->GetRespirationRate(FrequencyUnit::Per_min));
    bg->GetLogger()->Info(std::stringstream() << "EC50: " << bg->GetSubstanceManager().GetSubstance("Epinephrine")->GetPD().GetEC50().GetValue(MassPerVolumeUnit::ug_Per_mL));
    bg->GetLogger()->Info(std::stringstream() << "Conc: " << bg->GetSubstanceManager().GetSubstance("Epinephrine")->GetPlasmaConcentration(MassPerVolumeUnit::ug_Per_mL));

    this->Internals->o2series->append(bg->GetSimulationTime(TimeUnit::s), bg->GetBloodChemistrySystem()->GetOxygenSaturation());

    this->Internals->o2chart->axisX()->setRange(0,bg->GetSimulationTime(TimeUnit::s));
    
    if(bg->GetBloodChemistrySystem()->GetOxygenSaturation() < this->Internals->minO2)
    {
        this->Internals->minO2 = bg->GetBloodChemistrySystem()->GetOxygenSaturation();
    }

    if(this->Internals->minO2 < 0.8)
        this->Internals->o2chart->axisY()->setRange(this->Internals->minO2,1.0);
    else
        this->Internals->o2chart->axisY()->setRange(0.8,1.0);

    
    this->Internals->volseries->append(bg->GetSimulationTime(TimeUnit::s), bg->GetRespiratorySystem()->GetTidalVolume(VolumeUnit::mL));
    this->Internals->volchart->axisX()->setRange(0,bg->GetSimulationTime(TimeUnit::s));

    QFile logFile;
    logFile.setFileName("test.log");
    logFile.open(QIODevice::ReadOnly | QIODevice::Text);

    this->Internals->logOutput->clear();

    while (!logFile.atEnd()) {
        QString line(logFile.readLine());
        this->Internals->logOutput->append(line.simplified());
    }
}

void PhysiologyExplorerMainWindow::startEngine()
{
    this->Internals->CurrentScenario = new Scenario(this->Internals->MainView, this);
    this->Internals->CurrentScenario->loadScenarioData();

    this->Internals->anaphylaxisGroupBox->setVisible(true);
    this->Internals->o2chartView->setVisible(true);
    this->Internals->volchartView->setVisible(true);

    this->Internals->o2series->clear();
    this->Internals->volseries->clear();

    bg = CreateBioGearsEngine("test.log");
    bg->LoadState("states/DefaultMale@0s.xml");
    bg->GetLogger()->SetLogLevel(log4cpp::Priority::DEBUG);
    bg->GetLogger()->Info("BioGears engine started");

    updateLog();

    emit dataChanged();
}

void PhysiologyExplorerMainWindow::addObstruction()
{
    SEAirwayObstruction obst;
    obst.GetSeverity().SetValue(this->Internals->severitySlider->value());
    bg->ProcessAction(obst);

    updateLog();
}

void PhysiologyExplorerMainWindow::advanceTime()
{
    bg->AdvanceModelTime(this->Internals->timeStepBox->value(), TimeUnit::s);

    if (this->Internals->reduceObstruction)
    {
        double newObstSize = this->Internals->lastObstSize - (this->Internals->reduceObstRatio * this->Internals->timeStepBox->value());

        SEAirwayObstruction obst;
        obst.GetSeverity().SetValue(newObstSize);
        bg->ProcessAction(obst);

        this->Internals->lastObstSize = newObstSize;
    }
    
    this->Internals->CurrentScenario->colorData(bg->GetBloodChemistrySystem()->GetOxygenSaturation());

    updateLog();

    emit dataChanged();
}

void PhysiologyExplorerMainWindow::updateUI()
{
    this->Internals->MainView->render();
}
