#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QSplitter>
#include <QDatetime>
#include <QScrollArea>
#include <QMessageBox>

#include "../include/AtlasXAssetWidget.h"
#include "../include/AtlasXImpl.h"
#include "../include/AtlasXPlot.h"
#include "../include/AtlasXPlotData.h"
#include "../include/AtlasXHelpers.h"


namespace AtlasX
{

//============================================================================
struct AtlasXAssetImpl
{
	AtlasXAppImpl* app;
	Option<String> asset_name;
	HashMap<String, size_t> headers;
	SharedPtr<Atlas::Exchange> exchange;
	UniquePtr<QTabWidget> tables = nullptr;
	UniquePtr<QTableView> table_view = nullptr;
	UniquePtr<AtlasPlotAssetWrapper> plot = nullptr;
	UniquePtr<AtlasXAssetPlotBuilder> plot_builder = nullptr;

	AtlasXAssetImpl(
		AtlasXAppImpl* a,
		SharedPtr<Atlas::Exchange> _exchange,
		Option<String> asset_name
	) noexcept :
		app(a),
		exchange(_exchange),
		asset_name(asset_name)
	{
		headers = app->getExchangeHeaders(exchange);
		plot_builder = std::make_unique<AtlasXAssetPlotBuilder>(app);
	}
};


//============================================================================
AtlasXAsset::AtlasXAsset(
	QWidget* parent,
	AtlasXAppImpl* app,
	SharedPtr<Atlas::Exchange> exchange,
	Option<String> asset_name
) noexcept :
	QWidget(parent),
	impl(new AtlasXAssetImpl(app, exchange, asset_name))
{
	initUI();
	initSignals();
}


//============================================================================s
void
AtlasXAsset::initUI() noexcept
{
	auto splitter = new QSplitter(Qt::Horizontal, this);
	splitter->setContentsMargins(0, 0, 0, 0);

	// asset table view
	auto asset_internal_layout = new QHBoxLayout(this);

	// init asset table
	auto right_widget = new QWidget(this);
	impl->tables = std::make_unique<QTabWidget>(this);
	impl->table_view = std::make_unique<QTableView>(this);
	impl->table_view->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	impl->table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	if (impl->asset_name.has_value()) 
	{
		initData();
	}
	impl->tables->addTab(impl->table_view.get(), "Data");
	asset_internal_layout->addWidget(impl->tables.get());
	right_widget->setLayout(asset_internal_layout);

	// asset plot
	initPlot();

	// Add widgets to the splitter
	splitter->addWidget(impl->plot.get());
	splitter->addWidget(right_widget);
	QList<int> sizes;
	sizes << splitter->sizeHint().width() * 3 << splitter->sizeHint().width();
	splitter->setSizes(sizes);

	// Add the splitter to the main layout
	auto layout = new QHBoxLayout(this);
	layout->addWidget(splitter);
	setLayout(layout);
}


//============================================================================
void
AtlasXAsset::initSignals() noexcept
{
	connect(
		impl->table_view->horizontalHeader(),
		&QHeaderView::customContextMenuRequested,
		this,
		&AtlasXAsset::showHeaderContextMenu
	);
}


//============================================================================
void
AtlasXAsset::showHeaderContextMenu(QPoint pos) noexcept
{
	QModelIndex index = impl->table_view->indexAt(pos);
	QMenu* menu = new QMenu(this);

	auto plot_action = new QAction("Plot", this);
	connect(
		plot_action,
		&QAction::triggered,
		this,
		[=]() { plotColumn(impl->table_view->currentIndex().column()); }
	);
	menu->addAction(plot_action);
	
	auto remove_action = new QAction("Remove", this);
	connect(
		remove_action,
		&QAction::triggered,
		this,
		[=]() { removeColumn(impl->table_view->currentIndex().column()); }
	);
	menu->addAction(remove_action);
	menu->popup(impl->table_view->viewport()->mapToGlobal(pos));
}


//============================================================================
void
AtlasXAsset::onHydraStep()
{
	auto idx = impl->app->getCurrentIdx();// TODO should be exchange specific
	auto model = impl->table_view->model();
	auto q_idx = model->index(idx, 0);
	auto q_idx_prev = model->index(idx - 1, 0);
	QColor opaqueRed = QColor(Qt::red);
	opaqueRed.setAlpha(150); 
	model->setData(q_idx, QBrush(opaqueRed), Qt::BackgroundRole);
	model->setData(q_idx_prev, QVariant(), Qt::BackgroundRole);
}


//============================================================================'
void
AtlasXAsset::onHydraReset()
{
	auto model = impl->table_view->model();
	for (int i = 0; i < model->rowCount(); ++i)
	{
		auto q_idx = model->index(i, 0);
		model->setData(q_idx, QVariant(), Qt::BackgroundRole);
	}
}

//============================================================================
void
AtlasXAsset::initPlot() noexcept
{
	String asset_name = impl->asset_name.has_value() ? impl->asset_name.value() : "No asset selected";
	impl->plot = std::make_unique<AtlasPlotAssetWrapper>(
		this,
		impl->plot_builder.get(),
		asset_name
	);
}


//============================================================================
void
AtlasXAsset::plotColumn(int columnIndex) noexcept
{
	// find the column name
	String column_name = "";
	for (auto const& [name, index] : impl->headers)
	{
		if (index == columnIndex)
		{
			column_name = name;
			break;
		}
	}
	assert(column_name != "");
	
	auto const& slice = impl->app->getAssetSlice(impl->asset_name.value()).value();
	auto const& timestamps = impl->app->getTimestamps(impl->exchange);
	size_t rows = timestamps.size();
}


//============================================================================
void
AtlasXAsset::removeColumn(int columnIndex) noexcept
{
// find the column name
	String column_name = "";
	for (auto const& [name, index] : impl->headers)
	{
		if (index == columnIndex)
		{
			column_name = name;
			break;
		}
	}
	assert(column_name != "");
}



//============================================================================
void 
AtlasXAsset::initData() noexcept
{
	if (!impl->asset_name)
		return;

	auto slice_opt = impl->app->getAssetSlice(impl->asset_name.value());

	if (!slice_opt)
	{
		QMessageBox::critical(
			this,
			"Error",
			"Failed to get asset slice"
		);
		return;
	}
	auto& slice = slice_opt.value();
	auto const& timestamps = impl->app->getTimestampsStr(impl->exchange);

	QStandardItemModel* model = new QStandardItemModel(this);
	size_t rows = timestamps.size();
	size_t cols = impl->headers.size();
	model->setRowCount(rows);
	model->setColumnCount(cols);
	model->setHorizontalHeaderLabels(mapToQStringList(impl->headers));
	model->setVerticalHeaderLabels(timestamps);

	for (size_t i = 0; i < 100; ++i)
	{
		for (size_t j = 0; j < cols; ++j)
		{
			size_t index = i * cols + j;
			double val = slice(index);
			model->setItem(i, j, new QStandardItem(QString::number(val)));
		}
	}

	impl->table_view->setModel(model);
	impl->table_view->resizeColumnsToContents();
}





//============================================================================
AtlasXAsset::~AtlasXAsset() noexcept
{
	delete impl;
}



}