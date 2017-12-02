/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include "cdm/CommonDataModel.h"
#include "PulsePhysiologyEngine.h"

#include <QObject>
#include <QTextEdit>

class PulseListener
{
public:
  // This is where we pull data from pulse, and push any actions to it
  virtual void ProcessPhysiology(PhysiologyEngine& pulse) = 0;
};

class QPulse : public QObject
{
  Q_OBJECT
public:
  QPulse(QThread& thread, QTextEdit& log);
  virtual ~QPulse();

public:
  PhysiologyEngine& GetEngine();

  void Reset();
  void Start();
  void Stop();
  bool PlayPause();//return true=paused
  void RegisterListener(PulseListener* listener);
  void AdvanceTime();

protected:

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