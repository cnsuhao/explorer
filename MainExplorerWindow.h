#ifndef PhysiologyExplorerMainWindow_H
#define PhysiologyExplorerMainWindow_H

#include <QMainWindow>
#include <QObject>
#include "QPulse.h"


namespace Ui {
  class MainExplorerWindow;
}

class MainExplorerWindow : public QMainWindow, public PulseListener
{
  Q_OBJECT

public:
  MainExplorerWindow();
  ~MainExplorerWindow();
  void closeEvent(QCloseEvent *event);

  void ProcessPhysiology(PhysiologyEngine& pulse);

signals:
  void UIChanged();
  void PulseChanged();
protected slots:
  void UpdateUI();
  void PulseUpdate();
  void PlayPause();
  void ResetExplorer();
  void ResetShowcase();
  void StartAnaphylaxisShowcase();

private:
  class Controls;
  Controls* m_Controls;
};

#endif // MainExplorerWindow_H
