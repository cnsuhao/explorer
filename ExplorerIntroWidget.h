/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QObject>
#include <QDockWidget>

namespace Ui {
  class ExplorerIntroWidget;
}

class ExplorerIntroWidget : public QDockWidget
{
  Q_OBJECT
public:
  ExplorerIntroWidget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~ExplorerIntroWidget();

  QString GetShowcase();

signals:
  void StartSelectedShowcase();
protected slots:
  void UpdateUI();
  void ReadSelectedShowcase();

private:
  class Controls;
  Controls* m_Controls;
  
};