#pragma once
#include <QMainWindow>

#include <DockWidget.h>

#include "../include/AtlasXTypes.h"

namespace AtlasX
{


struct AtlasXExchangeManagerImpl;

class AtlasXExchangeManager : public QMainWindow
{
	Q_OBJECT


signals:
	//============================================================================
	void exchangeAdded(
		SharedPtr<Atlas::Exchange> exchange,
		String exchange_id
	);

	//============================================================================
	void exchangeSelected(
		SharedPtr<Atlas::Exchange> exchange,
		String exchange_id
	);

	//============================================================================
	void hydraStep();

private slots:
	//============================================================================
	void onAddExchange(
		SharedPtr<Atlas::Exchange> exchange,
		String exchange_id
	);

	//============================================================================
	void onExchangeSelected(
		SharedPtr<Atlas::Exchange> exchange,
		String exchange_id
	);

public slots:
	void onHydraRestore();
	void onHydraStep();

private:
	AtlasXAppImpl* m_app = nullptr;
	AtlasXExchangeManagerImpl* m_impl = nullptr;

	void newExchange();
	void selectExchange();
	void buildUI();
	void buildSignals();
	void connectExchange(AtlasXExchange*);

public:
	//============================================================================
	AtlasXExchangeManager(
		QWidget *parent,
		AtlasXAppImpl* hydra
	);

	//============================================================================
	~AtlasXExchangeManager();

	//============================================================================
	static ads::CDockWidget* make(
		QWidget *parent,
		AtlasXAppImpl* hydra
	);

};


}