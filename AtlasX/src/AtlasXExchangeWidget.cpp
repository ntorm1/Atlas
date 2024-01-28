
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QStackedWidget>
#include <QSplitter>

#include "../include/AtlasXExchangeWidget.h"
#include "../include/AtlasXAssetWidget.h"
#include "../include/AtlasXImpl.h"


namespace AtlasX
{


//============================================================================
struct AtlasXExchangeImpl
{
	AtlasXAppImpl* app;
	SharedPtr<Atlas::Exchange> exchange = nullptr;
	QMap<QString, int> assetIndexMap;
	UniquePtr<QStackedWidget> stacked_widget = std::make_unique<QStackedWidget>();
	HashMap<String, size_t> asset_ids;
	HashMap<String, size_t> headers;
	String exchange_id;

	AtlasXExchangeImpl(
		AtlasXAppImpl* app,
		SharedPtr<Atlas::Exchange> exchange,
		String const& exchange_id
	) noexcept :
		app(app),
		exchange(exchange),
		exchange_id(exchange_id),
		headers(app->getExchangeHeaders(exchange))
	{ }
};


//============================================================================
AtlasXExchange::AtlasXExchange(
	QWidget* parent,
	AtlasXAppImpl* app,
	SharedPtr<Atlas::Exchange> exchange,
	String const& exchange_id
) noexcept 	: 
	QWidget(parent)
{
	m_impl = new AtlasXExchangeImpl(app, exchange, exchange_id);
	m_impl->asset_ids = m_impl->app->getAssetMap(exchange);
	initUi();
}


//============================================================================
void
AtlasXExchange::initUi() noexcept
{
	auto splitter = new QSplitter(Qt::Horizontal, this);
	splitter->setContentsMargins(0, 0, 0, 0);

	// exchange gui layout
	auto exchange_layout = new QVBoxLayout(this);
	exchange_layout->setContentsMargins(0, 0, 0, 0);
	auto exchange_label = new QLabel(("Exchange ID: " + m_impl->exchange_id).c_str(), this);
	exchange_label->setAlignment(Qt::AlignCenter);
	exchange_layout->addWidget(exchange_label);

	// available assets
	auto asset_list = new QListWidget(this);
	asset_list->setSelectionMode(QAbstractItemView::SingleSelection);
	for (auto& [asset_id, index] : m_impl->asset_ids)
	{
		auto item = new QListWidgetItem(asset_id.c_str(), asset_list);
		item->setFlags(item->flags() | Qt::ItemIsSelectable);
	}
	connect(asset_list, &QListWidget::itemSelectionChanged, this, &AtlasXExchange::onAssetSelected);
	exchange_layout->addWidget(asset_list);

	// Set up the left widget
	auto leftWidget = new QWidget(this);
	leftWidget->setLayout(exchange_layout);

	// child asset gui layout
	m_impl->stacked_widget = std::make_unique<QStackedWidget>(this);
	auto asset_widget = new AtlasXAsset(
		this,
		m_impl->app,
		m_impl->exchange,
		std::nullopt
	);
	m_impl->stacked_widget->addWidget(asset_widget);

	// Set up the right widget
	auto rightWidget = new QWidget(this);
	rightWidget->setLayout(new QVBoxLayout);
	rightWidget->layout()->addWidget(m_impl->stacked_widget.get());

	// Add widgets to the splitter
	splitter->addWidget(leftWidget);
	splitter->addWidget(rightWidget);

	// Set the initial size of the right side to 80% of the width
	splitter->setStretchFactor(0, 2);  // Left widget takes up 2/3 of the space
	splitter->setStretchFactor(1, 8);  // Right widget takes up 8/3 of the space (80%)

	// Add the splitter to the main layout
	auto mainLayout = new QHBoxLayout(this);
	mainLayout->addWidget(splitter);

	setLayout(mainLayout);
}


//============================================================================
void
AtlasXExchange::onAssetSelected() noexcept
{
	auto asset_list = findChild<QListWidget*>();
	auto selected = asset_list->selectedItems();

	if (selected.size() == 0)
	{
		m_impl->stacked_widget->setCurrentIndex(0);
		return;
	}

	auto selected_item = selected[0];
	auto asset_id = selected_item->text().toStdString();

	// Check if the asset_id is already in the map
	if (!m_impl->assetIndexMap.contains(asset_id.c_str()))
	{
		// If not, create a new widget and add it to the stacked widget
		auto asset_widget = new AtlasXAsset(
			this,
			m_impl->app,
			m_impl->exchange,
			asset_id
		);
		connectAsset(asset_widget);
		int index = m_impl->stacked_widget->addWidget(asset_widget);

		// Add the asset_id and index to the map
		m_impl->assetIndexMap[asset_id.c_str()] = index;
	}

	// Set the current widget based on the selected asset
	m_impl->stacked_widget->setCurrentIndex(m_impl->assetIndexMap[asset_id.c_str()]);
}


//============================================================================
void
AtlasXExchange::connectAsset(AtlasXAsset* asset_widget) noexcept
{
	connect(
		this, 
		&AtlasXExchange::hydraStep,
		asset_widget,
		&AtlasXAsset::onHydraStep
	);
}


//============================================================================
void
AtlasXExchange::onHydraStep()
{
	emit hydraStep();
}


//============================================================================
AtlasXExchange::~AtlasXExchange()
{
	delete m_impl;
}


}