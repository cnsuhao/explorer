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
#include <QScrollBar>
#include <QTimer>
#include <QThread>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMutex>

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

#include "MainExplorerWindow.h"
#include "ui_MainExplorerWindow.h"
#include "GeometryView.h"
#include "ExplorerIntroWidget.h"
#include "AnaphylaxisShowcaseWidget.h"
#include "MultiTraumaShowcaseWidget.h"
#include "DataRequestsWidget.h"
#include "VitalsMonitorWidget.h"

#include "cdm/CommonDataModel.h"
#include "PulsePhysiologyEngine.h"
#include "cdm/scenario/SEDataRequestManager.h"
#include "cdm/engine/SEEngineTracker.h"
#include "cdm/properties/SEScalarTime.h"


class MainExplorerWindow::Controls : public Ui::MainExplorerWindow
{
public:

  virtual ~Controls()
  {
    delete Pulse;
    delete GeometryView;
    delete MainView;
    delete ExplorerIntroWidget;
    delete AnaphylaxisShowcaseWidget;
    delete VitalsMonitorWidget;
    delete DataRequestsWidget;
  }

  QMutex                            Mutex;
  QPulse*                           Pulse;
  QPointer<QThread>                 Thread;
  QPointer<GeometryView>            GeometryView;
  pqRenderView*                     MainView;
  ExplorerIntroWidget*              ExplorerIntroWidget;
  AnaphylaxisShowcaseWidget*        AnaphylaxisShowcaseWidget;
  MultiTraumaShowcaseWidget*        MultiTraumaShowcaseWidget;
  VitalsMonitorWidget*              VitalsMonitorWidget;
  DataRequestsWidget*               DataRequestsWidget;
  std::stringstream                 Status;
  double                            CurrentSimTime_s;
};

MainExplorerWindow::MainExplorerWindow()
{
  m_Controls = new Controls();
  m_Controls->setupUi(this);

  new pqAlwaysConnectedBehavior(this);
  new pqPersistentMainWindowStateBehavior(this);

  vtkSMReaderFactory::AddReaderToWhitelist("sources", "XMLPolyDataReader");
  vtkSMReaderFactory::AddReaderToWhitelist("sources", "XMLUnstructuredGridReader");
  pqApplicationCore::instance()->loadConfigurationXML("<xml/>");

  // This is the widget all input widgets will use
  m_Controls->InputWidget->setTitleBarWidget(new QWidget());
  m_Controls->InputWidget->show();
  m_Controls->InputWidget->raise();
  m_Controls->InputWidget->setVisible(true);

  // This is the logger widget
  m_Controls->OutputWidget->setTitleBarWidget(new QWidget());
  m_Controls->OutputWidget->show();
  m_Controls->OutputWidget->raise();
  m_Controls->OutputWidget->setVisible(true);
  m_Controls->LogBox->setFontPointSize(10);
  
  m_Controls->Thread = new QThread(parent());
  m_Controls->Pulse = new QPulse(*m_Controls->Thread, *m_Controls->LogBox);
  m_Controls->Pulse->RegisterListener(this);
  m_Controls->Status << "Current Simulation Time : 0s";

  // Add the Intro Widget to the Main control area
  m_Controls->ExplorerIntroWidget = new ExplorerIntroWidget(this);
  m_Controls->ExplorerIntroWidget->setTitleBarWidget(new QWidget());
  m_Controls->InputWidget->layout()->addWidget(m_Controls->ExplorerIntroWidget);

  // Add ParaView view to the tabWidget
  m_Controls->MainView =
      qobject_cast<pqRenderView*>(pqApplicationCore::instance()->getObjectBuilder()->createView(
                                  pqRenderView::renderViewType(),pqActiveObjects::instance().activeServer()));
  m_Controls->GeometryView = new GeometryView(m_Controls->MainView, this);
  m_Controls->GeometryView->LoadGeometry();
  m_Controls->Pulse->RegisterListener(m_Controls->GeometryView);
  this->setCentralWidget(m_Controls->TabWidget);
  m_Controls->TabWidget->widget(0)->layout()->addWidget(m_Controls->MainView->widget());
  m_Controls->VitalsMonitorWidget = new VitalsMonitorWidget(*m_Controls->LogBox, this);
  m_Controls->Pulse->RegisterListener(m_Controls->VitalsMonitorWidget);
  m_Controls->TabWidget->widget(1)->layout()->addWidget(m_Controls->VitalsMonitorWidget);
  m_Controls->DataRequestsWidget = new DataRequestsWidget(*m_Controls->LogBox, this);
  m_Controls->DataRequestsWidget->setTitleBarWidget(new QWidget());
  m_Controls->Pulse->RegisterListener(m_Controls->DataRequestsWidget);
  m_Controls->TabWidget->widget(2)->layout()->addWidget(m_Controls->DataRequestsWidget);

  m_Controls->RunInRealtime->setVisible(false);
  m_Controls->PlayPauseButton->setVisible(false);
  m_Controls->ResetExplorer->setVisible(false);
  m_Controls->ResetShowcaseButton->setVisible(false);

  // Add Scenario Widgets

  m_Controls->AnaphylaxisShowcaseWidget = new AnaphylaxisShowcaseWidget(*m_Controls->Pulse, this);
  m_Controls->AnaphylaxisShowcaseWidget->setTitleBarWidget(new QWidget());
  m_Controls->InputWidget->layout()->addWidget(m_Controls->AnaphylaxisShowcaseWidget);
  m_Controls->AnaphylaxisShowcaseWidget->setVisible(false);

  m_Controls->MultiTraumaShowcaseWidget = new MultiTraumaShowcaseWidget(*m_Controls->Pulse,  this);
  m_Controls->MultiTraumaShowcaseWidget->setTitleBarWidget(new QWidget());
  m_Controls->InputWidget->layout()->addWidget(m_Controls->MultiTraumaShowcaseWidget);
  m_Controls->MultiTraumaShowcaseWidget->setVisible(false);

  connect(this,SIGNAL(PulseChanged()), this, SLOT(PulseUpdate()));
  connect(m_Controls->RunInRealtime, SIGNAL(clicked()), this, SLOT(RunInRealtime()));
  connect(m_Controls->PlayPauseButton, SIGNAL(clicked()), this, SLOT(PlayPause()));
  connect(m_Controls->ResetExplorer, SIGNAL(clicked()), this, SLOT(ResetExplorer()));
  connect(m_Controls->ResetShowcaseButton, SIGNAL(clicked()), this, SLOT(ResetShowcase()));
}

MainExplorerWindow::~MainExplorerWindow()
{
  
  delete m_Controls;
}


void MainExplorerWindow::closeEvent(QCloseEvent *event)
{
  // Are these questions too annoying?
  /*QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Explorer",
    tr("Are you sure you want to exit?\n"),
    QMessageBox::No | QMessageBox::Yes,
    QMessageBox::Yes);
  if (resBtn != QMessageBox::Yes) {
    event->ignore();
  }
  else {
    event->accept();
  }*/
  m_Controls->Pulse->Stop();
  QMainWindow::closeEvent(event);
}

void MainExplorerWindow::PlayPause()
{
  if (m_Controls->Pulse->PlayPause())
    m_Controls->PlayPauseButton->setText("Play");
  else
    m_Controls->PlayPauseButton->setText("Pause");
}

void MainExplorerWindow::RunInRealtime()
{
  if (m_Controls->Pulse->ToggleRealtime())
    m_Controls->RunInRealtime->setChecked(true);
  else
    m_Controls->RunInRealtime->setChecked(false);
}

void MainExplorerWindow::ResetExplorer()
{
  m_Controls->Pulse->Reset();
  m_Controls->DataRequestsWidget->Reset();
  m_Controls->VitalsMonitorWidget->Reset();
  m_Controls->RunInRealtime->setChecked(true);
  m_Controls->PlayPauseButton->setText("Pause");
  m_Controls->LogBox->clear();
  m_Controls->Status << "Current Simulation Time : 0s";
  m_Controls->ExplorerIntroWidget->setVisible(true);
  m_Controls->RunInRealtime->setVisible(false);
  m_Controls->PlayPauseButton->setVisible(false);
  m_Controls->ResetExplorer->setVisible(false);
  m_Controls->ResetShowcaseButton->setVisible(false);
  m_Controls->AnaphylaxisShowcaseWidget->setVisible(false);
  m_Controls->MultiTraumaShowcaseWidget->setVisible(false);
  m_Controls->Pulse->RemoveListener(m_Controls->AnaphylaxisShowcaseWidget);
  m_Controls->Pulse->RemoveListener(m_Controls->MultiTraumaShowcaseWidget);
}

void MainExplorerWindow::ResetShowcase()
{
  m_Controls->Pulse->Reset();
  m_Controls->GeometryView->Reset();
  m_Controls->DataRequestsWidget->Reset();
  m_Controls->VitalsMonitorWidget->Reset();
  m_Controls->RunInRealtime->setChecked(true);
  m_Controls->PlayPauseButton->setText("Pause");
  m_Controls->LogBox->clear();  
  m_Controls->Pulse->RemoveListener(m_Controls->AnaphylaxisShowcaseWidget);
  m_Controls->Pulse->RemoveListener(m_Controls->MultiTraumaShowcaseWidget);
  StartShowcase();
}

void MainExplorerWindow::StartShowcase()
{
  m_Controls->ExplorerIntroWidget->setVisible(false);
  m_Controls->RunInRealtime->setVisible(true);
  m_Controls->PlayPauseButton->setVisible(true);
  m_Controls->ResetExplorer->setVisible(true);
  m_Controls->ResetShowcaseButton->setVisible(true);
  QString showcase = m_Controls->ExplorerIntroWidget->GetShowcase();
  m_Controls->Pulse->GetEngineTracker().Clear();
  if(showcase == "Anaphylaxis")
  {
    m_Controls->GeometryView->RenderSpO2(true);
    m_Controls->AnaphylaxisShowcaseWidget->setVisible(true);
    m_Controls->AnaphylaxisShowcaseWidget->ConfigurePulse(m_Controls->Pulse->GetEngine(),m_Controls->Pulse->GetEngineTracker().GetDataRequestManager());
    m_Controls->Pulse->RegisterListener(m_Controls->AnaphylaxisShowcaseWidget);
  }
  else if(showcase == "MultiTrauma")
  {
    m_Controls->MultiTraumaShowcaseWidget->setVisible(true);
    m_Controls->MultiTraumaShowcaseWidget->ConfigurePulse(m_Controls->Pulse->GetEngine(), m_Controls->Pulse->GetEngineTracker().GetDataRequestManager());
    m_Controls->Pulse->RegisterListener(m_Controls->MultiTraumaShowcaseWidget);
  }
  m_Controls->DataRequestsWidget->BuildGraphs(m_Controls->Pulse->GetEngine());
  m_Controls->Pulse->Start();
}

void MainExplorerWindow::PulseUpdateUI()
{
  m_Controls->Mutex.lock();
  m_Controls->Status.str("");
  m_Controls->Status << "Current Simulation Time : " << m_Controls->CurrentSimTime_s << "s";
  m_Controls->StatusBar->showMessage(QString(m_Controls->Status.str().c_str()));
  m_Controls->MainView->render();
  m_Controls->Mutex.unlock();
}

void MainExplorerWindow::ProcessPhysiology(PhysiologyEngine& pulse)
{
  m_Controls->Mutex.lock();
  // This is where we pull data from pulse, and push any actions to it
  m_Controls->CurrentSimTime_s = pulse.GetSimulationTime(TimeUnit::s);
  m_Controls->Mutex.unlock();
}
