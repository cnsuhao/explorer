#include "QPulse.h"
#include <QThread>
#include <QPointer>
#include <QCoreApplication>
#include <QScrollBar>

#include "cdm/CommonDataModel.h"
#include "PulsePhysiologyEngine.h"
#include "cdm/scenario/SEDataRequestManager.h"
#include "cdm/engine/SEEngineTracker.h"
#include "cdm/system/equipment/electrocardiogram/SEElectroCardioGramWaveformInterpolator.h"
#include "cdm/patient/SEPatient.h"
#include "cdm/system/physiology/SECardiovascularSystem.h"
#include "cdm/system/physiology/SEBloodChemistrySystem.h"
#include "cdm/system/physiology/SERespiratorySystem.h"
#include "cdm/patient/actions/SECardiacArrest.h"
#include "cdm/patient/actions/SEAirwayObstruction.h"
#include "cdm/CommonDataModel.h"
#include "cdm/PhysiologyEngine.h"
#include "cdm/properties/SEScalarTime.h"
#include "cdm/properties/SEScalar0To1.h"
#include "cdm/properties/SEScalarVolume.h"
#include "cdm/properties/SEScalarMass.h"
#include "cdm/properties/SEScalarMassPerVolume.h"
#include "cdm/properties/SEScalarFrequency.h"
#include "cdm/properties/SEFunctionElectricPotentialVsTime.h"
#include "cdm/substance/SESubstance.h"
#include "cdm/substance/SESubstanceManager.h"
#include "cdm/patient/actions/SESubstanceBolus.h"
#include "cdm/utils/TimingProfile.h"
#include <thread>

class LoggerForward2Qt : public LoggerForward
{
public:
  LoggerForward2Qt(QTextEdit& log) : ExplorerLog(log) {}
  virtual ~LoggerForward2Qt() {}
  virtual void ForwardDebug(const std::string& msg, const std::string& origin) { ExplorerLog.append(QString(msg.c_str())); ScrollLogBox(); }
  virtual void ForwardInfo(const std::string& msg, const std::string& origin)
  { 
    for (std::string str : IgnoreActions)
    {
      if (msg.find(str) != str.npos)
        return;
    }
    ExplorerLog.append(QString(msg.c_str()));
    ScrollLogBox();
  }
  virtual void ForwardWarning(const std::string& msg, const std::string& origin) { ExplorerLog.append(QString(msg.c_str())); ScrollLogBox(); }
  virtual void ForwardError(const std::string& msg, const std::string& origin)   { ExplorerLog.append(QString(msg.c_str())); ScrollLogBox(); }
  virtual void ForwardFatal(const std::string& msg, const std::string& origin)   { ExplorerLog.append(QString(msg.c_str())); ScrollLogBox(); }

  void ScrollLogBox()
  {
    QScrollBar *sb = ExplorerLog.verticalScrollBar();
    sb->setValue(sb->maximum());
    ExplorerLog.update();
  }

  QTextEdit& ExplorerLog;
  std::vector<std::string> IgnoreActions;
};

class QPulse::Controls
{
public:
  Controls(QThread& thread, QTextEdit& log) : Thread(thread), Log2Qt(log)
  {
    Pulse = CreatePulseEngine("PulseExplorer.log");
    Pulse->GetLogger()->SetForward(&Log2Qt);
    Pulse->GetLogger()->SetLogLevel(log4cpp::Priority::INFO);
  }
  virtual ~Controls()
  {
   
  }

  std::unique_ptr<PhysiologyEngine> Pulse;
  LoggerForward2Qt                  Log2Qt;
  QThread&                          Thread;
  TimingProfile                     Timer; 
  bool                              Running=false;
  bool                              Paused=false;
  bool                              RunInRealtime=true;
  bool                              Advancing;
  double                            AdvanceStep_s;
  std::vector<PulseListener*>       Listeners;
};

QPulse::QPulse(QThread& thread, QTextEdit& log) : QObject()
{
  m_Controls = new Controls(thread,log);


  connect(this, SIGNAL(RefreshUI()), SLOT(UpdateUI()));
}

QPulse::~QPulse()
{
  Stop();
  delete m_Controls;
}

QTextEdit& QPulse::GetLogBox()
{
  return m_Controls->Log2Qt.ExplorerLog;
}
void QPulse::ScrollLogBox()
{
  m_Controls->Log2Qt.ScrollLogBox();
}

void QPulse::IgnoreAction(const std::string& name)
{
  m_Controls->Log2Qt.IgnoreActions.push_back(name);
}

PhysiologyEngine& QPulse::GetEngine()
{
  return *m_Controls->Pulse;
}

SEEngineTracker& QPulse::GetEngineTracker()
{
  return *m_Controls->Pulse->GetEngineTracker();
}

double QPulse::GetTimeStep_s()
{
  return m_Controls->AdvanceStep_s;
}

void QPulse::Start()
{
  Worker* worker = new Worker(*this);
  worker->moveToThread(&m_Controls->Thread);
  connect(&m_Controls->Thread, SIGNAL(started()), worker, SLOT(Work()));
  connect(&m_Controls->Thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  m_Controls->Thread.start();
}
void Worker::Work()
{
  _qpulse.AdvanceTime();
}

void QPulse::Reset()
{
  Stop();
  m_Controls->Running = false;
  m_Controls->Paused = false;
  m_Controls->Advancing = false;
  m_Controls->RunInRealtime = true;
  m_Controls->Log2Qt.IgnoreActions.clear();
}

bool QPulse::PlayPause()
{
  if (m_Controls->Thread.isRunning())
    m_Controls->Paused = !m_Controls->Paused;
  return m_Controls->Paused;
}

bool QPulse::ToggleRealtime()
{
  if (m_Controls->Thread.isRunning())
    m_Controls->RunInRealtime = !m_Controls->RunInRealtime;

  //if (m_Controls->RunInRealtime)
  //  m_Controls->AdvanceStep_s = m_Controls->Pulse->GetTimeStep(TimeUnit::s);
  //else
  //  m_Controls->AdvanceStep_s = 0.1;

  return m_Controls->RunInRealtime;
}

void QPulse::Stop()
{
  if (m_Controls->Thread.isRunning())
  {
    m_Controls->Running = false;
    while(m_Controls->Advancing)
      std::this_thread::sleep_for(std::chrono::seconds(1));
    m_Controls->Thread.quit();
  }
  
}

void QPulse::RegisterListener(PulseListener* l)
{
  if (l == nullptr)
    return;
  auto itr = std::find(m_Controls->Listeners.begin(), m_Controls->Listeners.end(), l);
  if (itr == m_Controls->Listeners.end())
    m_Controls->Listeners.push_back(l);
}

void QPulse::RemoveListener(PulseListener* l)
{
  auto itr = std::find(m_Controls->Listeners.begin(), m_Controls->Listeners.end(), l);
  if (itr != m_Controls->Listeners.end())
    m_Controls->Listeners.erase(itr);
}

void QPulse::AdvanceTime()
{
  long long sleep_ms;
  TimingProfile timer;
  m_Controls->Running = true;
  m_Controls->Advancing = true;
  m_Controls->AdvanceStep_s = m_Controls->Pulse->GetTimeStep(TimeUnit::s);
  timer.Start("ui");
  while (m_Controls->Running)
  {
    if (m_Controls->Paused)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else
    {
      timer.Start("r");
      try {
        m_Controls->Pulse->AdvanceModelTime(m_Controls->AdvanceStep_s, TimeUnit::s);
      } catch(CommonDataModelException ex) { }
      for (PulseListener* l : m_Controls->Listeners)
        l->ProcessPhysiology(*m_Controls->Pulse);
      sleep_ms = (long long)((m_Controls->AdvanceStep_s - timer.GetElapsedTime_s("r"))*1000);
      if (m_Controls->RunInRealtime && sleep_ms > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));// Wait for real time to catch up
    }
    if (timer.GetElapsedTime_s("ui") > 0.1)
    {
      emit RefreshUI();// Only update the UI every 0.1 seconds
      timer.Start("ui");// Reset our timer
    }
  }
  m_Controls->Advancing = false;
}

void QPulse::UpdateUI()
{
  if (m_Controls->Running)
  {
    for (PulseListener* l : m_Controls->Listeners)
      l->PulseUpdateUI();
  }
}

