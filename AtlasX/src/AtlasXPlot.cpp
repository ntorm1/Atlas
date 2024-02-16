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
	AtlasPlot* plot
) : 
	QWidget(parent),
	m_plot(plot)
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
	AtlasXStrategyPlotBuilder* builder,
	String strategy_name
) : 
	AtlasPlotWrapper(parent, new AtlasStrategyPlot(parent, builder, strategy_name))
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


//============================================================================
AtlasPlotAssetWrapper::AtlasPlotAssetWrapper(
	QWidget* parent,
	AtlasXAssetPlotBuilder* builder,
	String asset
) noexcept :
	AtlasPlotWrapper(parent, new AtlasAssetPlot(parent, builder, asset))
{
	m_asset_plot = static_cast<AtlasAssetPlot*>(m_plot);
	m_asset_name = asset;
	m_asset_plot->setTitle(std::format("Asset: {}", asset).c_str());
}


//============================================================================
AtlasPlotAssetWrapper::~AtlasPlotAssetWrapper() noexcept
{
}

}