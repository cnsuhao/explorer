#include "vtkWaveformWidget.h"


vtkWaveformWidget::vtkWaveformWidget()
{
  _max_values = 250;
  _time = vtkSmartPointer<vtkFloatArray>::New();
  _time->SetName("Time");
  _data = vtkSmartPointer<vtkFloatArray>::New();
  _data->SetName("Data");

  _table = vtkSmartPointer<vtkTable>::New();
  _table->AddColumn(_time);
  _table->AddColumn(_data);

  _chart = vtkSmartPointer<vtkChartXY>::New();
  _chart->GetAxis(vtkAxis::BOTTOM)->SetTitle("");
  _chart->GetAxis(vtkAxis::LEFT)->SetTitle("");
  _line = _chart->AddPlot(vtkChart::LINE);
  _line->SetInputData(_table, 0, 1);
  _line->SetColor(0, 255, 0, 255);
  _line->SetWidth(1.0);

  _view = vtkSmartPointer<vtkContextView>::New();
  _view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
  _view->GetScene()->AddItem(_chart);

  _widget = new QVTKWidget();
  _view->SetInteractor(_widget->GetInteractor());
  _widget->SetRenderWindow(_view->GetRenderWindow());
  _widget->show();
}

vtkWaveformWidget::~vtkWaveformWidget()
{
  delete _widget;
}

void vtkWaveformWidget::Clear()
{
  //_chart->ClearPlots();
}

void vtkWaveformWidget::PushData(double time_s, double data_value)
{
  if (_table->GetNumberOfRows() == 0)
  {
    _table->SetNumberOfRows(1);
    _table->SetValue(0, 0, time_s);
    _table->SetValue(0, 1, data_value);
    return;
  }
  vtkIdType num_values = _table->GetNumberOfRows();
  if (_view->GetScene()->GetNumberOfItems() == 0 && num_values >= 2)
  {
    
  }
  if (num_values + 1 > _max_values)
  {
    for (vtkIdType i = 0; i < num_values-1; ++i)
    {
      _table->SetValue(i, 0, _table->GetValue(0,i+1));
      _table->SetValue(i, 1, _table->GetValue(1,i+1));
    }
    _table->SetValue(num_values, 0, time_s);
    _table->SetValue(num_values, 1, data_value);
    _view->GetInteractor()->Render();
    return;
  }
  // Just add it to the end
  _table->SetNumberOfRows(num_values+1);
  _table->SetValue(num_values, 0, time_s);
  _table->SetValue(num_values, 1, data_value);


  _view->GetScene()->AddItem(_chart);
  _line->SetInputData(_table);
  _view->GetInteractor()->Render();
}