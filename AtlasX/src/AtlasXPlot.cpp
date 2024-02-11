#include <cassert>
#include "qcustomplot.h"

#include "../include/AtlasXPlot.h"
#include "../include/AtlasXPlotImpl.h"
#include "../include/AtlasXPlotData.h"


//import StrategyModule;
//import ExchangeModule;

namespace AtlasX
{

//============================================================================
AtlasPlotWrapper::AtlasPlotWrapper(
	QWidget* parent,
	AtlasXPlotBuilder* builder,
	AtlasPlot* plot
) : 
	QWidget(parent),
	m_plot(plot),
	m_builder(builder)
{
	if (!plot)
	{
		m_plot = new AtlasPlot(parent);
	}

	auto layout = new QVBoxLayout(this);
	layout->addWidget(m_plot);
	setLayout(layout);
}


//============================================================================
AtlasPlotWrapper::~AtlasPlotWrapper()
{
	delete m_plot;
}


//============================================================================
AtlasPlotStrategyWrapper::AtlasPlotStrategyWrapper(
	QWidget* parent,
	AtlasXPlotBuilder* builder,
	String strategy_name
) : 
	AtlasPlotWrapper(parent, builder, new AtlasStrategyPlot(parent, builder, strategy_name))
{
	m_strategy_plot = static_cast<AtlasStrategyPlot*>(m_plot);
	m_strategy_name = strategy_name;
	m_strategy_plot->setTitle(std::format("Strategy: {}", strategy_name).c_str());
}


//============================================================================
void
AtlasPlotStrategyWrapper::onHydraReset()
{
	m_strategy_plot->removeAllGraphs();
}


//============================================================================
AtlasPlotStrategyWrapper::~AtlasPlotStrategyWrapper()
{
}

}