#pragma once
#include <QMainWindow>

#include <DockWidget.h>

#include "../include/AtlasXTypes.h"

namespace AtlasX
{

struct AtlasXStrategyManagerImpl;

class AtlasXStrategyManager : public QMainWindow
{
	Q_OBJECT

public slots:
	void onHydraStep();
	void onHydraRun();
	void onHydraReset();

private:
	AtlasXStrategyManagerImpl* m_impl;

	void newStrategy() noexcept;
	void openStrategy() noexcept;
	void compileStrategy() noexcept;
	void selectStrategy() noexcept;
	void initInterpreter() noexcept;
	void appendIfNotInSysPath(String const& p) noexcept;
	void initUI() noexcept;
	void initPlot() noexcept;
	void updateStrategyState(bool is_built) noexcept;

	String strategyIdToPath(String const& id) noexcept;

public:
	AtlasXStrategyManager(
		QWidget *parent,
		AtlasXAppImpl* app
		) noexcept;
	~AtlasXStrategyManager() noexcept;


	//============================================================================
	static ads::CDockWidget* make(
		QWidget* parent,
		AtlasXAppImpl* hydra
	);

};





}