
#include <filesystem>
#include <fstream>
#include <pybind11/embed.h>

#include "../include/AtlasXStrategyManager.h"
#include "../include/AtlasXExchangeWidget.h"
#include "../include/AtlasXEditor.h"
#include "../include/AtlasXImpl.h"

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

namespace fs = std::filesystem;
namespace py = pybind11;

namespace AtlasX
{


//============================================================================
struct AtlasXStrategyTemp
{
	String strategy_name;
	String path = "";
	double portfolio_alloc;

	AtlasXStrategyTemp(
		const String& _strategy_name,
		const String& _path,
		double _portfolio_alloc
	) noexcept :
		strategy_name(_strategy_name),
		portfolio_alloc(_portfolio_alloc),
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
	Option<UniquePtr<AtlasXStrategyTemp>> strategy_temp = std::nullopt;
	UniquePtr<QScintillaEditor> editor = nullptr;

	AtlasXStrategyManagerImpl(
		AtlasXStrategyManager* self,
		AtlasXAppImpl* app
	) noexcept:
		self(self),
		app(app)
	{
	}

};


//============================================================================
AtlasXStrategyManager::AtlasXStrategyManager(
	QWidget* parent,
	AtlasXAppImpl* app
) noexcept:
	QMainWindow(parent),
	m_impl(new AtlasXStrategyManagerImpl(this, app))
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto tool_bar = new QToolBar(this);

	QIcon icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/add.png"));
	auto a = new QAction(icon, tr("&New Strategy"), this);
	a->setStatusTip(tr("Create a new strategy"));
	a->setShortcut(QKeySequence::New);
	connect(
		a, &QAction::triggered,
		this, &AtlasXStrategyManager::newStrategy
	);
	tool_bar->addAction(a);


	icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/select.png"));
	a = new QAction(icon, tr("&Select Strategy"), this);
	a->setStatusTip(tr("Select and exsisting strategy"));
	connect(
		a, &QAction::triggered,
		this, &AtlasXStrategyManager::selectStrategy
	);
	tool_bar->addAction(a);


	icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/compile.png"));
	a = new QAction(icon, tr("&Compile"), this);
	a->setStatusTip(tr("Compile strategy"));
	a->setShortcut(QKeySequence::Save);
	connect(
		a, &QAction::triggered,
		this, &AtlasXStrategyManager::compileStrategy
	);
	tool_bar->addAction(a);

	addToolBar(Qt::LeftToolBarArea, tool_bar);
	buildUI();
	initInterpreter();
}


//============================================================================
AtlasXStrategyManager::~AtlasXStrategyManager() noexcept
{
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

	QLabel* portfolio_alloc_label = new QLabel(tr("Portfolio Allocation:"), this);
	QDoubleValidator* portfolio_alloc_validator = new QDoubleValidator(0.0, 1.0, 2, this);
	QLineEdit* portfolio_alloc_line_edit = new QLineEdit(&dialog);
	portfolio_alloc_line_edit->setValidator(portfolio_alloc_validator);

	// Create layout for the dialog
	QFormLayout* formLayout = new QFormLayout(&dialog);
	formLayout->addRow(name_label, name_line_edit);
	formLayout->addRow(portfolio_alloc_label, portfolio_alloc_line_edit);

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
		double portfolio_alloc = portfolio_alloc_str.toDouble();

		if (portfolio_alloc == 0.0f || portfolio_alloc > 1) {
			QMessageBox::critical(this, tr("Invalid Portfolio Allocation"), tr("Portfolio allocation must be between 0 and 1"));
			return;
		}
		
		qDebug() << "Strategy Name: " << strategy_name;
		qDebug() << "Portfolio Allocation: " << portfolio_alloc;

		m_impl->strategy_temp = std::make_unique<AtlasXStrategyTemp>(
			strategy_name.toStdString(),
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
		std::ofstream strategy_file(strategy_py);
		strategy_file << "def build():\n";
		strategy_file << "    pass\n";
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
		py::object result = module.attr("build")(323);

		// Convert the result to an integer
		int c = result.cast<int>();

		String message = "Test c = " + std::to_string(c);
		QMessageBox::information(this, tr("Test"), QString::fromStdString(message));
	}
	catch (py::error_already_set const& e) {
		String err = "Error compiling strategy: " + std::string(e.what()) + "\n";
		err += "Python Module path: " + module_path + "\n";
		err += "From Python: \n";
		QMessageBox::critical(this, tr("Python Error"), QString::fromStdString(e.what()));
	}
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
		fs::path strategy_path = strategy_dir / (strategy_name.toStdString() + ".py");

		// open the strategy in the editor
		m_impl->editor->loadFile(QString::fromStdString(strategy_path.string()));

		// init the strategy temp
		m_impl->strategy_temp = std::make_unique<AtlasXStrategyTemp>(
			strategy_name.toStdString(),
			strategy_path.string(),
			0.0
		);
	}
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
AtlasXStrategyManager::buildUI() noexcept
{
	m_impl->editor = std::make_unique<QScintillaEditor>();
	setCentralWidget(m_impl->editor.get());
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
	AtlasXAppImpl* app
)
{
	ads::CDockWidget* dock = new ads::CDockWidget(
		"AtlasX Strategy Manager"
	);
	auto node = new AtlasXStrategyManager(dock, app);
	node->setFocusPolicy(Qt::NoFocus);
	dock->setWidget(node);
	dock->setIcon(QIcon("./styles/icons/flow.png"));
	return dock;
}


}