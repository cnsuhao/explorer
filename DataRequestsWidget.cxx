#include "DataRequestsWidget.h"

#include <QVBoxLayout>

#include "cdm/utils/FileUtils.h"
#include "vtkWaveformWidget.h"

#include "cdm/system/equipment/electrocardiogram/SEElectroCardioGramWaveformInterpolator.h"
#include "cdm/properties/SEFunctionElectricPotentialVsTime.h"

class DataRequestsWidget::Controls
{
public:
  Controls(QTextEdit& log) : LogBox(log) {}
  QTextEdit&                         LogBox;
  std::unique_ptr<vtkWaveformWidget> Waveform;
  QVBoxLayout*                       Layout;
};

DataRequestsWidget::DataRequestsWidget(QTextEdit& log, QWidget *parent, Qt::WindowFlags flags) : QWidget(parent,flags)
{
  m_Controls = new Controls(log);

  m_Controls->Waveform = std::unique_ptr<vtkWaveformWidget>(new vtkWaveformWidget());
  m_Controls->Waveform->_chart->SetTitle("Waveform");

  SEElectroCardioGramWaveformInterpolator ecg(nullptr);
  ecg.LoadFile("./ecg/StandardECG.pba");
  SEElectroCardioGramWaveform& lead3 = ecg.GetWaveform(cdm::ElectroCardioGramWaveformData_eLead_Lead3, cdm::eHeartRhythm::NormalSinus);
  SEFunctionElectricPotentialVsTime& wave3 = lead3.GetData();

  m_Controls->Waveform->_table->SetNumberOfRows(wave3.GetTime().size());
  for (int i = 0; i<wave3.GetTime().size(); i++)
  {
    m_Controls->Waveform->_table->SetValue(i, 0, wave3.GetTime()[i]);
    m_Controls->Waveform->_table->SetValue(i, 1, wave3.GetElectricPotential()[i]);
  }

  m_Controls->Layout = new QVBoxLayout;
  m_Controls->Layout->addWidget(m_Controls->Waveform->_widget);
  //m_Controls->Layout->addStretch(1);
  setLayout(m_Controls->Layout);

  connect(this, SIGNAL(UIChanged()), this, SLOT(UpdateUI()));
  connect(this, SIGNAL(PulseChanged()), this, SLOT(PulseUpdate()));
}

DataRequestsWidget::~DataRequestsWidget()
{
  delete m_Controls;
}

void DataRequestsWidget::Reset()
{

}

void DataRequestsWidget::ProcessPhysiology(PhysiologyEngine& pulse)
{
  // This is where we pull data from pulse, and push any actions to it
  // We don't need to update the GUI
  //emit PulseChanged(); // Call this if you need to update the UI with data from pulse
}

void DataRequestsWidget::UpdateUI()
{

}

void DataRequestsWidget::PulseUpdate()
{

}

