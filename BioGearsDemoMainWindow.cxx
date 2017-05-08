#include <QApplication>
#include <QMainWindow>

#include <pqActiveObjects.h>
#include <pqAlwaysConnectedBehavior.h>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqPersistentMainWindowStateBehavior.h>
#include <pqRenderView.h>

#include "BioGearsDemoMainWindow.h"
#include "ui_BioGearsDemoMainWindow.h"

class BioGearsDemoMainWindow::pqInternals : public Ui::BioGearsDemoMainWindow
{
public:
  bool FirstShow;
  pqInternals() : FirstShow(true)
  {

  }
};

BioGearsDemoMainWindow::BioGearsDemoMainWindow()
{
  this->Internals = new pqInternals();
  this->Internals->setupUi(this);

  new pqAlwaysConnectedBehavior(this);
  new pqPersistentMainWindowStateBehavior(this);

  this->Internals->inputDockWidget->show();
  this->Internals->inputDockWidget->raise();

  pqRenderView* view =
      qobject_cast<pqRenderView*>(pqApplicationCore::instance()->getObjectBuilder()->createView(
                                  pqRenderView::renderViewType(),pqActiveObjects::instance().activeServer()));

  this->setCentralWidget(view->widget());
}

BioGearsDemoMainWindow::~BioGearsDemoMainWindow()
{

}
