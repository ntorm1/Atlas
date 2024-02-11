#pragma once
#include <QWidget>

#include "../include/AtlasXTypes.h"

namespace AtlasX
{

class AtlasPlot;
class AtlasStrategyPlot;
class AtlasXPlotBuilder;

//============================================================================
class AtlasPlotWrapper : public QWidget
{

	Q_OBJECT

protected:
	AtlasPlot* m_plot = nullptr;
	AtlasXPlotBuilder* m_builder = nullptr;

public:
	AtlasPlotWrapper(
		QWidget* parent,
		AtlasXPlotBuilder* builder,
		AtlasPlot* plot = nullptr
	);
	~AtlasPlotWrapper();
};


//============================================================================
class AtlasPlotStrategyWrapper : public AtlasPlotWrapper
{
	Q_OBJECT

	friend class AtlasStrategyPlot;

private:
	AtlasStrategyPlot* m_strategy_plot;
	String m_strategy_name;


public:
	AtlasPlotStrategyWrapper(
		QWidget* parent,
		AtlasXPlotBuilder* m_builder,
		String strategy
	);
	~AtlasPlotStrategyWrapper();

	String const& getStrategyName() const { return m_strategy_name; }

};


}