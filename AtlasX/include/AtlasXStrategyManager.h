#pragma once
#include <QMainWindow>


#include "../include/AtlasXTypes.h"

namespace ads
{
	class CDockManager;
	class CDockWidget;
};

namespace AtlasX
{

struct AtlasXStrategyManagerImpl;
class AtlasPlotStrategyWrapper;

class AtlasXStrategyManager : public QMainWindow
{
	Q_OBJECT

public slots:
	void onHydraStep();
	void onHydraRun();
	void onHydraReset();

private:
	ads::CDockManager* m_dock_manager;
	AtlasXStrategyManagerImpl* m_impl;

	void newStrategy() noexcept;
	void openStrategy() noexcept;
	void compileStrategy() noexcept;
	void selectStrategy() noexcept;
	void openOptimizer() noexcept;
	void initInterpreter() noexcept;
	void appendIfNotInSysPath(String const& p) noexcept;
	void initUI() noexcept;
	void initSignals() noexcept;
	void updatePlotUI(UniquePtr<AtlasPlotStrategyWrapper> plot) noexcept;
	void initPlot(String const& strategy_name) noexcept;
	void updateStrategyState(bool is_built) noexcept;

	String strategyIdToPath(String const& id) noexcept;

public:
	AtlasXStrategyManager(
		QWidget *parent,
		AtlasXAppImpl* app,
		ads::CDockManager* dock_manager
		) noexcept;
	~AtlasXStrategyManager() noexcept;


	//============================================================================
	static ads::CDockWidget* make(
		QWidget* parent,
		AtlasXAppImpl* hydra,
		ads::CDockManager* dock_manager
	);

};





}