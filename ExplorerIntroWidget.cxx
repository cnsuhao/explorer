#include "ExplorerIntroWidget.h"
#include "ui_ExplorerIntro.h"

#include "cdm/utils/FileUtils.h"

class ExplorerIntroWidget::Controls : public Ui::ExplorerIntroWidget
{
public:
  QString Showcase = "";
};

ExplorerIntroWidget::ExplorerIntroWidget(QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent,flags)
{
  m_Controls = new Controls();
  m_Controls->setupUi(this);

  std::vector<std::string> states;
  ListFiles("./states", states);
  m_Controls->PatientStateComboBox->clear();
  int idx = 0, i=0;
  for (auto s : states)
  {
    s = s.substr(9, s.length()-13);
    if (s.find("StandardMale") != std::string::npos)
      idx = i;
    m_Controls->PatientStateComboBox->addItem(QString(s.c_str()));
    i++;
  }
  m_Controls->PatientStateComboBox->setCurrentIndex(idx);

  m_Controls->LoadPatientState->setEnabled(false);
  m_Controls->CreatePatientButton->setEnabled(false);
  m_Controls->LoadScenarioButton->setEnabled(false);

  connect(this,SIGNAL(dataChanged()), this, SLOT(updateUI()));
  connect(m_Controls->LoadShowcase, SIGNAL(clicked()), this,SLOT(ReadSelectedShowcase()));
  connect(this, SIGNAL(StartSelectedShowcase()), parentWidget(), SLOT(StartShowcase()));
}

ExplorerIntroWidget::~ExplorerIntroWidget()
{
  delete m_Controls;
}


void ExplorerIntroWidget::UpdateUI()
{

}

void ExplorerIntroWidget::ReadSelectedShowcase()
{
  m_Controls->Showcase = m_Controls->ShowcaseComboBox->currentText();
  emit StartSelectedShowcase();
}

QString ExplorerIntroWidget::GetShowcase()
{
  return m_Controls->Showcase;
}
