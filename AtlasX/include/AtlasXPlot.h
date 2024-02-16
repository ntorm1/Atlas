#pragma once
#include <QWidget>

#include "../include/AtlasXTypes.h"

namespace AtlasX
{

class AtlasPlot;
class AtlasAssetPlot;
class AtlasStrategyPlot;
class AtlasXStrategyPlotBuilder;
class AtlasXAssetPlotBuilder;

//============================================================================
class AtlasPlotWrapper : public QWidget
{

	Q_OBJECT

protected:
	AtlasPlot* m_plot = nullptr;

public:
	AtlasPlotWrapper(
		QWidget* parent,
		AtlasPlot* plot = nullptr
	);

	virtual void onHydraReset() {}

	~AtlasPlotWrapper();
};


//============================================================================
class AtlasPlotStrategyWrapper : public AtlasPlotWrapper
{
	Q_OBJECT

	friend class AtlasStrategyPlot;

private:
	AtlasStrategyPlot* m_strategy_plot;
	AtlasXStrategyPlotBuilder* m_builder = nullptr;
	String m_strategy_name;

public:
	AtlasPlotStrategyWrapper(
		QWidget* parent,
		AtlasXStrategyPlotBuilder* m_builder,
		String strategy
	);
	~AtlasPlotStrategyWrapper();

	String const& getStrategyName() const { return m_strategy_name; }
	void onHydraReset() override;
};


//============================================================================
class AtlasPlotAssetWrapper : public AtlasPlotWrapper
{
	Q_OBJECT

	friend class AtlasAssetPlot;
private:
	String m_asset_name;
	AtlasAssetPlot* m_asset_plot;

public:
	AtlasPlotAssetWrapper(
		QWidget* parent,
		AtlasXAssetPlotBuilder* builder,
		String asset
	) noexcept;
	~AtlasPlotAssetWrapper() noexcept;

};


}