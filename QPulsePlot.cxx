/* Distributed under the Apache License, Version 2.0.
See accompanying NOTICE file for details.*/
#include "QPulsePlot.h"


#include <QVector>
#include <deque>

class QPulsePlot::Data
{
public:
  QtCharts::QLineSeries* Series;
  QtCharts::QChart*      Chart;
  QtCharts::QChartView*  View;
  std::deque<double>     Times;
  std::deque<double>     Values;
  double                 MaxY;
  double                 MinY;
  size_t                 MaxSize;
};

QPulsePlot::QPulsePlot(size_t max_points)
{
  m_Data = new QPulsePlot::Data();
  m_Data->Series = new QtCharts::QLineSeries();
  m_Data->Series->setUseOpenGL(true);
  m_Data->Chart = new QtCharts::QChart();
  m_Data->Chart->legend()->hide();
  m_Data->Chart->addSeries(m_Data->Series);
  m_Data->Chart->createDefaultAxes();
  m_Data->View = new QtCharts::QChartView(m_Data->Chart);
  m_Data->View->setRenderHint(QPainter::Antialiasing);
  m_Data->View->setVisible(false);
  m_Data->MinY = std::numeric_limits<double>::max();
  m_Data->MaxY = -std::numeric_limits<double>::max();
  m_Data->MaxSize = max_points;
}

QPulsePlot::~QPulsePlot()
{
  delete m_Data->Series;
  delete m_Data->Chart;
  delete m_Data->View;
  delete m_Data;
}

void QPulsePlot::Reset()
{
  m_Data->Series->clear();
  m_Data->Times.clear();
  m_Data->Values.clear();
}

QtCharts::QLineSeries& QPulsePlot::GetSeries() { return *m_Data->Series; }
QtCharts::QChart& QPulsePlot::GetChart() { return *m_Data->Chart;  }
QtCharts::QChartView& QPulsePlot::GetView() { return *m_Data->View; }

void QPulsePlot::SetDataRange(double min, double max)
{
  m_Data->MinY = min;
  m_Data->MaxY = max;
  m_Data->Chart->axisY()->setRange(min, max);
}

void QPulsePlot::Append(double time, double value)
{
  m_Data->Times.push_back(time);
  m_Data->Values.push_back(value);

  double t = time;
  if (m_Data->Values.size() < m_Data->MaxSize)
  {
    for (size_t i = m_Data->Values.size(); i <= m_Data->MaxSize; i++)
    {
      t -= 1./50.;
      m_Data->Values.push_back(value);
      m_Data->Times.push_front(t);
    }
  }

  m_Data->Values.pop_front();
  m_Data->Times.pop_front();
}

void QPulsePlot::UpdateUI(bool pad)
{
  size_t size = m_Data->Values.size();
  if (size > 2)
  {
    QVector<QPointF> points;

    double v;

    for (size_t i = 0; i < size; i++)
    {
      v = m_Data->Values[i];
      if (v > m_Data->MaxY)
        m_Data->MaxY = v;
      if (v < m_Data->MinY)
        m_Data->MinY = v;
      points.append(QPointF(m_Data->Times[i], v));
    }
    if (m_Data->MinY == m_Data->MaxY)
    {
      m_Data->MinY -= 0.5;
      m_Data->MaxY += 0.5;
    }
    if(pad)
      m_Data->Chart->axisY()->setRange(m_Data->MinY - (m_Data->MinY*0.05), m_Data->MaxY + (m_Data->MaxY*0.05));
    else
      m_Data->Chart->axisY()->setRange(m_Data->MinY, m_Data->MaxY);
    m_Data->Chart->axisX()->setRange(m_Data->Times[0], m_Data->Times[size - 1]);
    m_Data->Series->replace(points);
  }
  m_Data->View->setVisible(true);
}

