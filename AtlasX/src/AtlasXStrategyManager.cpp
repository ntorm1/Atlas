
#include <filesystem>
#include <fstream>

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qboxlayout.h>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QListWidget>
#include <QComboBox>
#include <QListWidgetItem>
#include <QSplitter>

#include <DockManager.h>
#include <DockWidget.h>

#include "../include/AtlasXStrategyManager.h"
#include "../include/AtlasXExchangeWidget.h"
#include "../include/AtlasXEditor.h"
#include "../include/AtlasXImpl.h"
#include "../include/AtlasXPy.h"
#include "../include/AtlasXPlot.h"
#include "../include/AtlasXPlotData.h"
#include "../include/AtlasXScene.h"

namespace fs = std::filesystem;
namespace py = pybind11;

namespace AtlasX
{


//============================================================================
struct AtlasXStrategyTemp
{
	String strategy_name;
	String exchange_name;
	String portfolio_name;
	String path = "";
	double portfolio_alloc;

	AtlasXStrategyTemp(
		const String& _strategy_name,
		const String& _exchange_name,
		const String& _portfolio_name,
		const String& _path,
		double _portfolio_alloc
	) noexcept :
		strategy_name(_strategy_name),
		portfolio_alloc(_portfolio_alloc),
		portfolio_name(_portfolio_name),
		exchange_name(_exchange_name),
		path(_path)
	{
	}
};


//============================================================================
struct AtlasXStrategyManagerImpl
{

	AtlasXStrategyManager* self;
	AtlasXAppImpl* app;
	py::scoped_interpreter guard{};
	HashMap<String , py::module_> modules;
	QLabel* strategy_status = nullptr;
	Option<UniquePtr<AtlasXStrategyTemp>> strategy_temp = std::nullopt;
	Option<SharedPtr<AtlasXOptimizer>> optimizer = std::nullopt;
	UniquePtr<QScintillaEditor> editor = nullptr;
	UniquePtr<AtlasPlotWrapper> plot = nullptr;
	UniquePtr<AtlasXPlotBuilder> plot_builder = nullptr;
	bool is_built = false;


	Vector<Int64> chart_timestamps_ms;
	double nlv_min = 1e13;
	double nlv_max = 0.0;


	AtlasXStrategyManagerImpl(
		AtlasXStrategyManager* self,
		AtlasXAppImpl* app
	) noexcept:
		self(self),
		app(app)
	{
		plot_builder = std::make_unique<AtlasXPlotBuilder>(app);
	}
};


//============================================================================
AtlasXStrategyManager::AtlasXStrategyManager(
	QWidget* parent,
	AtlasXAppImpl* app,
	ads::CDockManager* dock_manager
) noexcept:
	QMainWindow(parent),
	m_impl(new AtlasXStrategyManagerImpl(this, app)),
	m_dock_manager(dock_manager)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto tool_bar = new QToolBar(this);

	QIcon icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/add.png"));
	auto a = new QAction(icon, tr("&New Strategy"), this);
	a->setToolTip(tr("Create a new strategy"));
	a->setShortcut(QKeySequence::New);
	connect(
		a, &QAction::triggered,
		this, &AtlasXStrategyManager::newStrategy
	);
	tool_bar->addAction(a);


	icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/select.png"));
	a = new QAction(icon, tr("&Select Strategy"), this);
	a->setToolTip(tr("Select and exsisting strategy"));
	connect(
		a, &QAction::triggered,
		this, &AtlasXStrategyManager::selectStrategy
	);
	tool_bar->addAction(a);


	icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/compile.png"));
	a = new QAction(icon, tr("&Compile"), this);
	a->setToolTip(tr("Compile strategy"));
	a->setShortcut(QKeySequence::Save);
	connect(
		a, &QAction::triggered,
		this, &AtlasXStrategyManager::compileStrategy
	);
	tool_bar->addAction(a);

	icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/comp.png"));
	a = new QAction(icon, tr("&Optimize"), this);
	a->setToolTip(tr("Optimizer"));
	a->setShortcut(QKeySequence::Save);
	connect(
		a, &QAction::triggered,
		this, &AtlasXStrategyManager::openOptimizer
	);
	tool_bar->addAction(a);

	QWidget* spacerWidget = new QWidget(tool_bar);
	spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	tool_bar->addWidget(spacerWidget);
	
	// add an unlickable icon to the toolbar
	icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/puzzle_part.png"));
	m_impl->strategy_status = new QLabel(this);
	m_impl->strategy_status->setPixmap(icon.pixmap(28, 28));
	tool_bar->addWidget(m_impl->strategy_status);


	addToolBar(Qt::RightToolBarArea, tool_bar);
	initUI();
	initInterpreter();
}


//============================================================================
void
AtlasXStrategyManager::onHydraStep()
{
	if (!m_impl->strategy_temp.has_value()) 
	{
		return;
	}
	auto const& strategy_name = m_impl->strategy_temp.value()->strategy_name;
	auto const& nlv_history = m_impl->app->getStrategyNLV(strategy_name);

	if (!nlv_history.size())
	{
		return;
	}

	auto idx = m_impl->app->getCurrentIdx();
	assert(idx < static_cast<size_t>(nlv_history.size()));
	auto nlv = nlv_history[idx];

	if (nlv < m_impl->nlv_min)
	{
		m_impl->nlv_min = nlv;
	}
	if (nlv > m_impl->nlv_max)
	{
		m_impl->nlv_max = nlv;
	}
}


//============================================================================
AtlasXStrategyManager::~AtlasXStrategyManager() noexcept
{
}


//============================================================================
void
AtlasXStrategyManager::onHydraRun()
{
	if (!m_impl->strategy_temp.has_value())
	{
		return;
	}
	auto const& strategy_name = m_impl->strategy_temp.value()->strategy_name;
	
	if (m_impl->optimizer)
	{
		auto grid_state = m_impl->app->getStrategyGridState(strategy_name);
		if (grid_state.has_value())
		{
			m_impl->optimizer.value()->setGridState(grid_state.value());
		}
	}
}


//============================================================================
void
AtlasXStrategyManager::onHydraReset()
{
	m_impl->plot->onHydraReset();
}


//============================================================================
void
AtlasXStrategyManager::newStrategy() noexcept
{
	// Create a new dialog window
	QDialog dialog(this);
	dialog.setWindowTitle(tr("New Strategy"));

	// Create widgets for the dialog
	QLabel* name_label = new QLabel(tr("Strategy Name:"), this);
	QLineEdit* name_line_edit = new QLineEdit(&dialog);

	QLabel* name_label_exchange = new QLabel(tr("Exchange Name:"), this);
	QComboBox* name_combo_box_exchange = new QComboBox(&dialog);
	for (auto const& [name, id] : m_impl->app->getExchangeIds()) {
		name_combo_box_exchange->addItem(QString::fromStdString(name));
	}

	QLabel* name_label_portfolio = new QLabel(tr("Portfolio Name:"), this);
	QComboBox* name_combo_box_portfolio = new QComboBox(&dialog);
	for (auto const& [name, id] : m_impl->app->getPortfolioIdxMap()) {
		name_combo_box_portfolio->addItem(QString::fromStdString(name));
	}

	QLabel* portfolio_alloc_label = new QLabel(tr("Portfolio Allocation:"), this);
	QDoubleValidator* portfolio_alloc_validator = new QDoubleValidator(0.0, 1.0, 2, this);
	QLineEdit* portfolio_alloc_line_edit = new QLineEdit(&dialog);
	portfolio_alloc_line_edit->setValidator(portfolio_alloc_validator);

	// Create layout for the dialog
	QFormLayout* formLayout = new QFormLayout(&dialog);
	formLayout->addRow(name_label, name_line_edit);
	formLayout->addRow(name_label_exchange, name_combo_box_exchange);
	formLayout->addRow(portfolio_alloc_label, portfolio_alloc_line_edit);
	formLayout->addRow(name_label_portfolio, name_combo_box_portfolio);

	// Add buttons to the dialog
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	formLayout->addWidget(buttonBox);

	// Connect the dialog buttons
	connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	// Execute the dialog and handle user input
	if (dialog.exec() == QDialog::Accepted) {
		// Retrieve values from the dialog
		QString strategy_name = name_line_edit->text();
		QString portfolio_alloc_str = portfolio_alloc_line_edit->text();
		QString exchange_name = name_combo_box_exchange->currentText();
		QString portfolio_name = name_combo_box_portfolio->currentText();
		double portfolio_alloc = portfolio_alloc_str.toDouble();

		if (portfolio_alloc == 0.0f || portfolio_alloc > 1) {
			QMessageBox::critical(this, tr("Invalid Portfolio Allocation"), tr("Portfolio allocation must be between 0 and 1"));
			return;
		}
		
		qDebug() << "Strategy Name: " << strategy_name;
		qDebug() << "Portfolio Allocation: " << portfolio_alloc;

		m_impl->strategy_temp = std::make_unique<AtlasXStrategyTemp>(
			strategy_name.toStdString(),
			exchange_name.toStdString(),
			portfolio_name.toStdString(),
			strategyIdToPath(strategy_name.toStdString()),
			portfolio_alloc
		);
		openStrategy();
	}
}


//============================================================================
void
AtlasXStrategyManager::openStrategy() noexcept
{
	assert(m_impl->strategy_temp.has_value());

	auto const& env_path_str = m_impl->app->getEnvPath();
	fs::path env_path(env_path_str);
	fs::path strategy_dir = env_path / "strategies";
	if (!fs::exists(strategy_dir)) 
	{
		fs::create_directory(strategy_dir);
	}

	fs::path strategy_py = strategy_dir / (m_impl->strategy_temp.value()->strategy_name + ".py");
	if (!fs::exists(strategy_py))
	{
		String alloc = std::to_string(m_impl->strategy_temp.value()->portfolio_alloc);
		std::ofstream strategy_file(strategy_py);
		strategy_file << "from AtlasPy.core import Strategy\n";
		strategy_file << "from AtlasPy.ast import *\n";
		strategy_file << "\n\n";
		strategy_file << "strategy_id = \"" << m_impl->strategy_temp.value()->strategy_name << "\"\n";
		strategy_file << "exchange_id = \"" << m_impl->strategy_temp.value()->exchange_name << "\"\n";
		strategy_file << "portfolio_id = \"" << m_impl->strategy_temp.value()->portfolio_name << "\"\n";
		strategy_file << "alloc = " << alloc << "\n\n";
		strategy_file << "def build(hydra):\n";
		strategy_file << "\thydra.removeStrategy" << "(strategy_id)\n";
		strategy_file << "\texchange = hydra.getExchange(exchange_id)\n";
		strategy_file << "\tportfolio = hydra.getPortfolio(portfolio_id)\n\n\n";
		strategy_file << "\tstrategy = hydra.addStrategy(Strategy(strategy_id, strategy_node, alloc))\n";
		strategy_file.close();
	}
	QString strategy_path = QString::fromStdString(strategy_py.string());
	m_impl->editor->loadFile(strategy_path);
	m_impl->strategy_temp.value()->path = strategy_path.toStdString();
	qDebug() << "Strategy Path: " << strategy_path << " loaded\n";
}


//============================================================================
void
AtlasXStrategyManager::compileStrategy() noexcept
{
	m_impl->editor->forceSave();
	if (!m_impl->strategy_temp.has_value()) {
		QMessageBox::critical(this, tr("No Strategy"), tr("No strategy loaded"));
		return;
	}
	String module_path;
	try {
		// load the strategy file into the sys path and import it
		String const& py_path = m_impl->strategy_temp.value()->path;
		fs::path py_dir = fs::path(py_path).parent_path();
		appendIfNotInSysPath(py_dir.string());

		String const& strategy_name= m_impl->strategy_temp.value()->strategy_name;
		if (m_impl->modules.contains(strategy_name))
		{
			qDebug() << "Reloading module: " << strategy_name.c_str();
			auto& module = m_impl->modules[strategy_name];
			module.reload();
		}
		else 
		{
			qDebug() << "Importing module: " << strategy_name.c_str();
			m_impl->modules[strategy_name] = py::module::import((m_impl->strategy_temp.value()->strategy_name).c_str());
		}
		auto& module = m_impl->modules[strategy_name];
			
		// get full path of module
		module_path = module.attr("__file__").cast<String>();

		// Call the 'main' function and get the result
		auto res = buildStrategy(
			m_impl->app,
			module
		);
		if (!res.has_value()) {
			QMessageBox::critical(this, tr("Error"), QString::fromStdString(res.error().what()));
			updateStrategyState(false);
			return;
		}
	}
	catch (py::error_already_set const& e) {
		String err = "Error compiling strategy: " + std::string(e.what()) + "\n";
		err += "Python Module path: " + module_path + "\n";
		err += "From Python: \n";
		QMessageBox::critical(this, tr("Python Error"), QString::fromStdString(e.what()));
		updateStrategyState(false);
		return;
	}
	updateStrategyState(true);
	initPlot(m_impl->strategy_temp.value()->strategy_name);
}


//============================================================================
void
AtlasXStrategyManager::selectStrategy() noexcept
{
	// get a list of all .py files in the strategies directory
	auto const& env_path_str = m_impl->app->getEnvPath();
	fs::path env_path(env_path_str);
	fs::path strategy_dir = env_path / "strategies";
	if (!fs::exists(strategy_dir))
	{
		fs::create_directory(strategy_dir);
	}
	Vector<String> strategy_names;
	for (auto const& entry : fs::directory_iterator(strategy_dir)) {
		if (entry.path().extension() == ".py") {
			qDebug() << entry.path().filename().string().c_str();
			strategy_names.push_back(entry.path().filename().string());
		}
	}
	// open a dialog to select a strategy
	QDialog dialog(this);
	dialog.setWindowTitle(tr("Select Strategy"));

	// Create widgets for the dialog
	QLabel* name_label = new QLabel(tr("Strategy Name:"), this);
	QComboBox* name_combo_box = new QComboBox(&dialog);
	for (auto const& strategy_file : strategy_names) {
		// remove .py extension
		String strategy_name = strategy_file.substr(0, strategy_file.size() - 3);
		name_combo_box->addItem(QString::fromStdString(strategy_name));
	}

	// Create layout for the dialog
	QFormLayout* formLayout = new QFormLayout(&dialog);
	formLayout->addRow(name_label, name_combo_box);

	// Add buttons to the dialog
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	formLayout->addWidget(buttonBox);

	// Connect the dialog buttons
	connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	// Execute the dialog and handle user input
	if (dialog.exec() == QDialog::Accepted) {
		// Retrieve values from the dialog
		QString strategy_name = name_combo_box->currentText();
		qDebug() << "Loading Strategy Name: " << strategy_name;

		// get the full path to the strategy
		auto strategy_name_std = strategy_name.toStdString();
		fs::path strategy_path = strategy_dir / (strategy_name_std + ".py");

		// open the strategy in the editor
		m_impl->editor->loadFile(QString::fromStdString(strategy_path.string()));

		// init the strategy temp
		m_impl->strategy_temp = std::make_unique<AtlasXStrategyTemp>(
			strategy_name_std,
			"",
			"",
			strategy_path.string(),
			0.0
		);

		// load in strategy plot if it exists
		auto strategy = m_impl->app->getStrategy(strategy_name_std);
		if (strategy)
		{
			initPlot(strategy_name_std);
		}
	}
}


//============================================================================
void
AtlasXStrategyManager::openOptimizer() noexcept
{
	ads::CDockWidget* dock = new ads::CDockWidget(
		"AtlasX Strategy Optimizer"
	);
	m_impl->optimizer = std::make_shared<AtlasXOptimizer>(this);
	dock->setWidget(m_impl->optimizer.value().get());
	dock->setIcon(QIcon("./styles/icons/comp.png"));
	auto DockArea = m_dock_manager->addDockWidgetFloating(dock);
}


//============================================================================
void
AtlasXStrategyManager::initInterpreter() noexcept
{
	auto const& env_path_str = m_impl->app->getEnvPath();
	fs::path env_path(env_path_str);

	// go up two directories to get to the root of the project exe
	fs::path root_path = env_path.parent_path().parent_path();

	// make sure AtlasPy.pyd is in the root directory
	fs::path atlas_pyd = root_path / "AtlasPy.pyd";
	if (!fs::exists(atlas_pyd)) {
		String err = "AtlasPy.pyd is missing from the root directory\n";
		err += "Expected in directory: " + root_path.string() + "\n";
		QMessageBox::critical(this, tr("Missing AtlasPy.pyd"), tr(err.c_str()));
		return;
	}
	
	// add the root path containing the python module to the interpreter path
	appendIfNotInSysPath(root_path.string());
}


//============================================================================
void
AtlasXStrategyManager::appendIfNotInSysPath(String const& p) noexcept
{
	py::list sys_path = py::module::import("sys").attr("path");
	for (auto const& path : sys_path) {
		if (path.cast<String>() == p) {
			return;
		}
	}
	sys_path.append(p);
}


//============================================================================
void
AtlasXStrategyManager::initUI() noexcept
{
	auto splitter = new QSplitter(Qt::Horizontal, this);
	splitter->setContentsMargins(0, 0, 0, 0);

	initPlot("");
	
	m_impl->editor = std::make_unique<QScintillaEditor>(this);
	m_impl->plot = std::make_unique<AtlasPlotWrapper>(this, m_impl->plot_builder.get());

	splitter->addWidget(m_impl->plot.get());
	splitter->addWidget(m_impl->editor.get());
	setCentralWidget(splitter);
}


//============================================================================
void
AtlasXStrategyManager::initSignals() noexcept
{
}


//============================================================================
void
AtlasXStrategyManager::updatePlotUI(UniquePtr<AtlasPlotStrategyWrapper> new_plot) noexcept
{
	// Get the splitter containing the plot
	QSplitter* splitter = qobject_cast<QSplitter*>(centralWidget());
	assert(splitter);

	// Get the index of the plot widget in the splitter
	int plotIndex = splitter->indexOf(m_impl->plot.get());
	assert(plotIndex != -1);

	// Replace the old plot widget with the new one
	splitter->replaceWidget(plotIndex, new_plot.get());

	// Set the new plot widget
	m_impl->plot = std::move(new_plot);
}


//============================================================================
void
AtlasXStrategyManager::initPlot(String const& strategy_name) noexcept
{
	// check if the strategy has a plot
	if (strategy_name == "")
	{
		m_impl->plot = std::make_unique<AtlasPlotWrapper>(
			this, m_impl->plot_builder.get()
		);
		return;
	}

	// check if the init call is on the same strategy as the current plot
	auto strategy_wrapper = dynamic_cast<AtlasPlotStrategyWrapper*>(m_impl->plot.get());
	if (strategy_wrapper && strategy_wrapper->getStrategyName() == strategy_name)
	{
		return;
	}

	auto strategy = m_impl->app->getStrategy(strategy_name);
	assert(strategy.has_value());

	// build the new plot instance and rebuild the splitter
	auto plot = std::make_unique<AtlasPlotStrategyWrapper>(
		this, m_impl->plot_builder.get(), strategy_name
	);
	updatePlotUI(std::move(plot));
}


//============================================================================
void
AtlasXStrategyManager::updateStrategyState(bool is_built_update) noexcept
{
	if (is_built_update && !m_impl->is_built) {
		auto icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/puzzle_full.png"));
		m_impl->strategy_status->setPixmap(icon.pixmap(28, 28));
		m_impl->strategy_status->setPixmap(QPixmap("./styles/icons/puzzle_full.png"));
	}
	else if (!is_built_update && m_impl->is_built) {
		auto icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/puzzle_part.png"));
		m_impl->strategy_status->setPixmap(icon.pixmap(28, 28));
		m_impl->strategy_status->setPixmap(QPixmap("./styles/icons/puzzle_part.png"));
	}

	if (is_built_update)
	{
		auto parent_exchange_name = m_impl->app->getStrategyParentExchange(
			m_impl->strategy_temp.value()->strategy_name
		);
		assert(parent_exchange_name.has_value());
		m_impl->strategy_temp.value()->exchange_name = parent_exchange_name.value();
	}
}


//============================================================================
String AtlasXStrategyManager::strategyIdToPath(String const& id) noexcept
{
auto const& env_path_str = m_impl->app->getEnvPath();
	fs::path env_path(env_path_str);
	fs::path strategy_dir = env_path / "strategies";
	if (!fs::exists(strategy_dir))
	{
		fs::create_directory(strategy_dir);
	}
	fs::path strategy_py = strategy_dir / (id + ".py");
	return strategy_py.string();
}


//============================================================================
ads::CDockWidget*
AtlasXStrategyManager::make(
	QWidget* parent,
	AtlasXAppImpl* app,
	ads::CDockManager* dock_manager
)
{
	ads::CDockWidget* dock = new ads::CDockWidget(
		"AtlasX Strategy Manager"
	);
	auto node = new AtlasXStrategyManager(dock, app, dock_manager);
	node->setFocusPolicy(Qt::NoFocus);
	dock->setWidget(node);
	dock->setIcon(QIcon("./styles/icons/flow.png"));
	return dock;
}


}