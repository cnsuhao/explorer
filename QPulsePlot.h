/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#pragma once

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

class QPulsePlot
{
public:
  
  QPulsePlot(size_t max_size=300);
  virtual ~QPulsePlot();

  void Reset();

  QtCharts::QLineSeries& GetSeries();
  QtCharts::QChart& GetChart();
  QtCharts::QChartView& GetView();

  void SetDataRange(double min, double max);
  void Append(double time, double value);
  void UpdateUI(bool pad=true);

private:
  class Data;
  Data* m_Data;
};
