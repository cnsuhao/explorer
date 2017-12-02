/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once
#include <QVTKWidget.h>

#include <vtkAxis.h>
#include <vtkVersion.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkChartXY.h>
#include <vtkTable.h>
#include <vtkPlot.h>
#include <vtkFloatArray.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkPen.h>

class vtkWaveformWidget : public QVTKWidget
{
public:
  vtkWaveformWidget();
  virtual ~vtkWaveformWidget();
  void Clear();

  void PushData(double time_s, double data_value);

  vtkSmartPointer<vtkTable>       _table;
  vtkSmartPointer<vtkFloatArray>  _time;
  vtkSmartPointer<vtkFloatArray>  _data;
  vtkSmartPointer<vtkContextView> _view;
  vtkSmartPointer<vtkChartXY>     _chart;
  QVTKWidget*                     _widget;
  vtkPlot*                        _line;

protected:
  vtkIdType                       _max_values;
};