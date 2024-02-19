#include <cassert>

#include "../include/AtlasXPlotImpl.h"
#include "../include/AtlasXPlotData.h"


namespace AtlasX
{

//============================================================================
AtlasPlot::AtlasPlot(
	QWidget* parent
) : QCustomPlot(parent)
{
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);

	xAxis->setRange(-8, 8);
	yAxis->setRange(-5, 5);
	axisRect()->setupFullAxesBox();

	plotLayout()->insertRow(0);
	QCPTextElement* title = new QCPTextElement(this, "AtlasPLot", QFont("sans", 17, QFont::Bold));
	plotLayout()->addElement(0, 0, title);

	xAxis->setLabel("Time");
	yAxis->setLabel("y Axis");
	legend->setVisible(true);
	QFont legendFont = font();
	legendFont.setPointSize(10);
	legend->setFont(legendFont);
	legend->setSelectedFont(legendFont);
	legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

	// set locale to english, so we get english month names:
	setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

	// configure bottom axis to show date instead of number:
	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
	dateTicker->setDateTimeFormat("d. MMMM\nyyyy");
	xAxis->setTicker(dateTicker);

	// connect slot that ties some axis selections together (especially opposite axes):
	connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
	// connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
	connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
	connect(this, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

	// make bottom and left axes transfer their ranges to top and right axes:
	connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
	connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));

	// connect some interaction slots:
	connect(this, SIGNAL(axisDoubleClick(QCPAxis*, QCPAxis::SelectablePart, QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*, QCPAxis::SelectablePart)));
	connect(this, SIGNAL(legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*, QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*)));
	//	connect(title, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClick(QMouseEvent*)));

		// connect slot that shows a message in the status bar when a graph is clicked:
	connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*, int, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, int)));

	// setup policy and connect slot for context menu popup:
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}


//============================================================================
AtlasPlot::~AtlasPlot()
{
}


//============================================================================
void AtlasPlot::setTitle(std::string title)
{
	assert(plotLayout()->hasElement(0, 0));
	auto element = plotLayout()->element(0, 0);
	auto title_ptr = static_cast<QCPTextElement*>(element);
	title_ptr->setText(QString::fromStdString(title));
}


//============================================================================
void
AtlasPlot::plot(
	Vector<Int64> const& x,
	Vector<double> const& y,
	std::string name,
	size_t start_idx
)
{
	auto x_span = std::span(x);
	auto y_span = std::span(y);
	plot(x_span, y_span, name, start_idx);
}


//============================================================================
void
AtlasPlot::plot(
	std::span<const long long> x,
	std::span<const double> y,
	std::string name,
	size_t start_idx
)
{
	auto new_graph = addGraph();
	auto q_name = QString::fromStdString(name);
	new_graph->setName(q_name);

	size_t stride = y.size() / x.size();

	QVector<QCPGraphData> timeData(x.size());
	for (int i = 0; i < x.size(); i++)
	{
		timeData[i].key = x[i] / static_cast<double>(1000000000);
		size_t y_idx = i * stride + start_idx;
		timeData[i].value = y[y_idx];
	}

	QBrush brush;
	QPen pen;
	QColor color = QColor(std::rand() % 245 + 10, std::rand() % 245 + 10, std::rand() % 245 + 10);
	new_graph->setPen(QPen(color));
	new_graph->data()->set(timeData, true);
	rescaleAxes();
	replot();
}


//============================================================================
void
AtlasPlot::contextMenuRequest(QPoint pos)
{
	QMenu* menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	if (legend->selectTest(pos, false) >= 0) // context menu on legend requested
	{
		menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
		menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
		menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
		menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
		menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
	}
	else  // general context menu on graphs requested
	{
		menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
		if (selectedGraphs().size() > 0)
			menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
		if (graphCount() > 0)
			menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
	}

	menu->popup(mapToGlobal(pos));
}


//============================================================================
void
AtlasPlot::moveLegend()
{
	if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
	{
		bool ok;
		int dataInt = contextAction->data().toInt(&ok);
		if (ok)
		{
			axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
			replot();
		}
	}
}


//============================================================================
void
AtlasPlot::mousePress()
{
	// if an axis is selected, only allow the direction of that axis to be dragged
	// if no axis is selected, both directions may be dragged

	if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
		axisRect()->setRangeDrag(xAxis->orientation());
	else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
		axisRect()->setRangeDrag(yAxis->orientation());
	else
		axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
}



//============================================================================
void
AtlasPlot::mouseWheel()
{
	// if an axis is selected, only allow the direction of that axis to be zoomed
	// if no axis is selected, both directions may be zoomed

	if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
		axisRect()->setRangeZoom(xAxis->orientation());
	else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
		axisRect()->setRangeZoom(yAxis->orientation());
	else
		axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}


//============================================================================
void
AtlasPlot::selectionChanged()
{
	/*
	 normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
	 the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
	 and the axis base line together. However, the axis label shall be selectable individually.

	 The selection state of the left and right axes shall be synchronized as well as the state of the
	 bottom and top axes.

	 Further, we want to synchronize the selection of the graphs with the selection state of the respective
	 legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
	 or on its legend item.
	*/

	// make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
	if (xAxis->selectedParts().testFlag(QCPAxis::spAxis) || xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
		xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		xAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
		xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
	}
	// make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
	if (yAxis->selectedParts().testFlag(QCPAxis::spAxis) || yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
		yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
	{
		yAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
		yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
	}

	// synchronize selection of graphs with selection of corresponding legend items:
	selected_line = std::nullopt;
	for (int i = 0; i < graphCount(); ++i)
	{
		QCPGraph* graph_int = this->graph(i);
		if (!legend->hasItemWithPlottable(graph_int)) { continue; }
		QCPPlottableLegendItem* item = legend->itemWithPlottable(graph_int);
		if (item->selected() || graph_int->selected())
		{
			item->setSelected(true);
			selected_line = item->plottable()->name().toStdString();
			graph_int->setSelection(QCPDataSelection(graph_int->data()->dataRange()));
		}
	}
}


//============================================================================
void
AtlasPlot::removeSelectedGraph()
{
	if (this->selectedGraphs().size() > 0)
	{
		this->removeGraph(this->selectedGraphs().first());
		this->rescaleAxes();
		this->replot();
	}
}


//============================================================================
void
AtlasPlot::removeGraphByName(std::string const& name)
{
	for (int i = 0; i < this->graphCount(); ++i)
	{
		QCPGraph* graph = this->graph(i);
		if (!legend->hasItemWithPlottable(graph)) { continue; }
		QCPPlottableLegendItem* item = this->legend->itemWithPlottable(graph);
		if (item->plottable()->name().toStdString() == name)
		{
			removeGraph(graph);
			rescaleAxes();
			replot();
		}
	}
}

//============================================================================
void
AtlasPlot::removeAllGraphs()
{
	clearGraphs();
	rescaleAxes();
	replot();
}



//============================================================================
AtlasStrategyPlot::AtlasStrategyPlot(
	QWidget* parent,
	AtlasXStrategyPlotBuilder* builder,
	String const& strategy_name
) : 
	AtlasPlot(parent),
	m_builder(builder),
	m_strategy_name(strategy_name)
{
}


//============================================================================
void
AtlasStrategyPlot::contextMenuRequest(QPoint pos)
{
	QMenu* menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	if (this->legend->selectTest(pos, false) >= 0) // context menu on legend requested
	{
		menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
		menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
		menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
		menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
		menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
	}
	else
	{
		QMenu* moveSubMenu = menu->addMenu("Plot");
		for (auto& [col_std,idx] :m_builder->strategyHistoryTypeMap())
		{
			QString col = QString::fromStdString(col_std);
			QAction* action = moveSubMenu->addAction(col);
			connect(action, &QAction::triggered, this, [this, col]() {
				this->addPlot(col);
			});
		}
		if (selectedGraphs().size() > 0)
			menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
		if (graphCount() > 0)
			menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
	}
	menu->popup(mapToGlobal(pos));
}


//============================================================================
void
AtlasAssetPlot::contextMenuRequest(QPoint pos)
{
	if (!m_exchange)
		return;

	QMenu* menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	if (this->legend->selectTest(pos, false) >= 0) // context menu on legend requested
	{
		menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
		menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
		menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
		menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
		menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
	}
	else {
		auto const& headers = m_builder->getExchangeHeaders(m_exchange);
		QMenu* moveSubMenu = menu->addMenu("Plot");
		for (auto& [col_std, idx] : headers)
		{
			QString col = QString::fromStdString(col_std);
			QAction* action = moveSubMenu->addAction(col);
			connect(action, &QAction::triggered, this, [this, col]() {
				this->addColumn(col);
			});
		}

		moveSubMenu->addSeparator();

		if (selectedGraphs().size() > 0)
			menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
		if (graphCount() > 0)
			menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
	}
	menu->popup(mapToGlobal(pos));
}


//============================================================================
void
AtlasStrategyPlot::addPlot(QString const& name)
{
	auto name_std = name.toStdString();
	auto history = m_builder->getStrategyHistory(m_strategy_name, name_std);

	if (!history)
	{
		auto error_msg = std::format("No history found for strategy {} and column {}", m_strategy_name, name_std);
		QMessageBox::warning(this, "Error", QString::fromStdString(error_msg));
		return;
	}

	auto x = m_builder->getStrategyTimeStamps(m_strategy_name);
	auto y = m_builder->getStrategyHistory(m_strategy_name, name_std);

	if (!x || !y)
	{
		auto error_msg = std::format("No {} history found for strategy {}", name.toStdString(), m_strategy_name);
		QMessageBox::warning(this, "Error", QString::fromStdString(error_msg));
		return;
	}

	plot(
		*x,
		*y,
		name.toStdString()
	);

}


//============================================================================
AtlasStrategyPlot::~AtlasStrategyPlot()
{
}


//============================================================================
void
AtlasAssetPlot::addColumn(QString const& name)
{
	if (m_column_names.contains(name.toStdString()))
	{
		return;
	}

	auto const& headers = m_builder->getExchangeHeaders(m_exchange);
	assert(headers.contains(name.toStdString()));
	size_t col_idx = headers.at(name.toStdString());
	auto const& x = m_builder->getTimestamps(m_exchange);
	plot(
		x,
		m_asset_data,
		name.toStdString(),
		col_idx
	);
	m_column_names.insert(name.toStdString());
}


//============================================================================
AtlasAssetPlot::AtlasAssetPlot(
	QWidget* parent,
	AtlasXAssetPlotBuilder* builder,
	String const& asset_name
) noexcept :
	AtlasPlot(parent),
	m_builder(builder),
	m_asset_name(asset_name)
{
	auto exchange = m_builder->getParentExchange(asset_name);
	if (!exchange)
	{
		m_exchange = nullptr;
	}
	else
	{
		m_exchange = *exchange;
		m_asset_data = m_builder->getAssetSlice(m_exchange, asset_name).value();
	}
}


//============================================================================
AtlasAssetPlot::~AtlasAssetPlot() noexcept
{
}



}