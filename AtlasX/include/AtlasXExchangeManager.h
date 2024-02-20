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

	void hydraStep();
	void hydraReset();

private slots:
	void onAddExchange(
		SharedPtr<Atlas::Exchange> exchange,
		String exchange_id
	);
	void onExchangeSelected(
		SharedPtr<Atlas::Exchange> exchange,
		String exchange_id
	);
	void onExchangeRemove(
		String exchange_id
	);


public slots:
	void onHydraRestore();
	void onHydraStep();
	void onHydraReset();

private:
	AtlasXAppImpl* m_app = nullptr;
	AtlasXExchangeManagerImpl* m_impl = nullptr;

	void removeExchange();
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