/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QObject>
#include <QTextEdit>
class PhysiologyEngine;
class SEEngineTracker;
class SEDataRequestManager;

class PulseListener
{
public:
  // This is where we pull data from pulse, and push any actions to it
  virtual void ProcessPhysiology(PhysiologyEngine& pulse) = 0;
  // This is where we take data that we pulleds from pulse and do anything to our UI based on it
  virtual  void PulseUpdateUI() { }
};

class QPulse : public QObject
{
  Q_OBJECT
public:
  QPulse(QThread& thread, QTextEdit& log);
  virtual ~QPulse();

public:
  PhysiologyEngine& GetEngine();
  SEEngineTracker& GetEngineTracker();

  QTextEdit& GetLogBox();
  void IgnoreAction(const std::string& name);

  void Reset();
  void Start();
  void Stop(); 
  bool ToggleRealtime();//return true=yes
  bool PlayPause();//return true=paused
  void RegisterListener(PulseListener* listener);
  void RemoveListener(PulseListener* listener);
  void AdvanceTime();
  double GetTimeStep_s();


signals:
  void RefreshUI();
protected slots :
  void UpdateUI();

private:
  class Controls;
  Controls* m_Controls;
};

class Worker : public QObject
{
  Q_OBJECT
public:
  Worker(QPulse& qpulse) : _qpulse(qpulse) {};
  virtual ~Worker() {}

public slots:
  void Work();

protected:
  QPulse& _qpulse;
};