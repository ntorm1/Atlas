
#include <QHBoxLayout>
#include <QListWidget>

#include "../include/AtlasXExchangeWidget.h"
#include "../include/AtlasXImpl.h"


namespace AtlasX
{

//============================================================================
struct AtlasXExchangeImpl
{
	AtlasXAppImpl* app;
	SharedPtr<Atlas::Exchange> exchange;
	HashMap<String, size_t> asset_ids;
};


//============================================================================
AtlasXExchange::AtlasXExchange(
	QWidget* parent,
	AtlasXAppImpl* app,
	SharedPtr<Atlas::Exchange> exchange
) noexcept 	: 
	QWidget(parent),
	m_impl(new AtlasXExchangeImpl)
{
	m_impl->app = app;
	m_impl->exchange = exchange;
	m_impl->asset_ids = m_impl->app->getAssetMap(exchange);

	initUi();
}


//============================================================================
void
AtlasXExchange::initUi() noexcept
{
	auto layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	auto asset_list = new QListWidget(this);
	for (auto& [asset_id, index] :m_impl->asset_ids)
	{
		auto item = new QListWidgetItem(asset_id.c_str(), asset_list);
		item->setFlags(item->flags() | Qt::ItemIsSelectable);
	}
	layout->addWidget(asset_list);

	setLayout(layout);
}

//============================================================================
AtlasXExchange::~AtlasXExchange()
{
	delete m_impl;
}


}