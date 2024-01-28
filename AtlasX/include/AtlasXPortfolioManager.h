#pragma once
#pragma once
#include <QMainWindow>

#include <DockWidget.h>

#include "../include/AtlasXTypes.h"

namespace AtlasX
{

struct AtlasXPortfolioManagerImpl;


class AtlasXPortfolioManager : public QMainWindow
{
	Q_OBJECT

private:

	AtlasXPortfolioManagerImpl* m_impl;
	void initToolbar() noexcept;
	void addPortfolio() noexcept;
	void selectPortfolio() noexcept;

public:
public:
	AtlasXPortfolioManager(
		QWidget* parent,
		AtlasXAppImpl* app
	) noexcept;
	~AtlasXPortfolioManager() noexcept;

	//============================================================================
	static ads::CDockWidget* make(
		QWidget* parent,
		AtlasXAppImpl* hydra
	);

};
}