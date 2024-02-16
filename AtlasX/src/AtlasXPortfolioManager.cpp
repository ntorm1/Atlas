
#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "../include/AtlasXPortfolioManager.h"
#include "../include/AtlasXImpl.h"


namespace AtlasX
{

//============================================================================
struct AtlasXPortfolioManagerImpl
{
	AtlasXAppImpl* app;
	String selected_portfolio = "";

	AtlasXPortfolioManagerImpl(AtlasXAppImpl* _app) noexcept
		: app(_app)
	{
	}

};


//============================================================================
AtlasXPortfolioManager::AtlasXPortfolioManager(
	QWidget* parent,
	AtlasXAppImpl* app
) noexcept 	: QMainWindow(parent)
{
	m_impl = new AtlasXPortfolioManagerImpl(app);
	initToolbar();
}


//============================================================================
void
AtlasXPortfolioManager::initToolbar() noexcept
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto tool_bar = new QToolBar(this);

	QIcon icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/add.png"));
	auto a = new QAction(icon, tr("&New Portfolio"), this);
	a->setStatusTip(tr("Create a new strategy"));
	a->setShortcut(QKeySequence::New);
	connect(
		a, &QAction::triggered,
		this, &AtlasXPortfolioManager::addPortfolio
	);
	tool_bar->addAction(a);


	icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/select.png"));
	a = new QAction(icon, tr("&Select Portfolio"), this);
	a->setStatusTip(tr("Select and exsisting strategy"));
	connect(
		a, &QAction::triggered,
		this, &AtlasXPortfolioManager::selectPortfolio
	);
	tool_bar->addAction(a);
	addToolBar(Qt::TopToolBarArea, tool_bar);
}


//============================================================================
void AtlasXPortfolioManager::addPortfolio() noexcept
{	
	// Create a new dialog window
	QDialog dialog(this);
	dialog.setWindowTitle(tr("New Portfolio"));

	// Create widgets for the dialog
	QLabel* name_label = new QLabel(tr("Portfolio Name:"), this);
	QLineEdit* name_line_edit = new QLineEdit(&dialog);

	QLabel* name_label_exchange = new QLabel(tr("Exchange:"), this);
	QComboBox* name_combo_box_exchange = new QComboBox(&dialog);
	for (auto const& [name, id] : m_impl->app->getExchangeIds()) {
		name_combo_box_exchange->addItem(QString::fromStdString(name));
	}

	QLabel* portfolio_alloc_label = new QLabel(tr("Intial Cash:"), this);
	QDoubleValidator* portfolio_alloc_validator = new QDoubleValidator(0.0, 1e9, 2, this);
	QLineEdit* portfolio_alloc_line_edit = new QLineEdit(&dialog);
	portfolio_alloc_line_edit->setValidator(portfolio_alloc_validator);

	// Create layout for the dialog
	QFormLayout* formLayout = new QFormLayout(&dialog);
	formLayout->addRow(name_label, name_line_edit);
	formLayout->addRow(name_label_exchange, name_combo_box_exchange);
	formLayout->addRow(portfolio_alloc_label, portfolio_alloc_line_edit);
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	formLayout->addWidget(buttonBox);

	// Connect the dialog buttons
	connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	// Execute the dialog and handle user input
	if (dialog.exec() == QDialog::Accepted) {
		// Retrieve values from the dialog
		QString portfolio_name = name_line_edit->text();
		QString portfolio_alloc_str = portfolio_alloc_line_edit->text();
		QString exchange_name = name_combo_box_exchange->currentText();
		double portfolio_alloc = portfolio_alloc_str.toDouble();

		auto exchange = m_impl->app->getExchange(exchange_name.toStdString());
		if (!exchange)
		{
			QMessageBox::critical(this, tr("Invalid Exchange"), tr("Exchange was not found"));
			return;
		}

		auto res = m_impl->app->addPortfolio(
			portfolio_name.toStdString(),
			*exchange,
			portfolio_alloc
		);

		if (!res)
		{
			QMessageBox::critical(this, tr("Invalid Portfolio Allocation"), tr("Portfolio allocation must be between 0 and 1"));
			return;
		}

		qDebug() << "CREATED NEW PORTFOLIO: " << portfolio_name << " " << portfolio_alloc << " " << exchange_name;
	}
}


//============================================================================
void AtlasXPortfolioManager::selectPortfolio() noexcept
{
	Vector<String> portfolio_names;
	for (auto const& [name, index]: m_impl->app->getPortfolioIdxMap()) {
		portfolio_names.push_back(name);
	}
	// open a dialog to select a strategy
	QDialog dialog(this);
	dialog.setWindowTitle(tr("Select Strategy"));

	// Create widgets for the dialog
	QLabel* name_label = new QLabel(tr("Portfolio Name:"), this);
	QComboBox* name_combo_box = new QComboBox(&dialog);
	for (auto const& strategy_file : portfolio_names) {
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
		QString portfolio_name = name_combo_box->currentText();
		m_impl->selected_portfolio = portfolio_name.toStdString();
	}
	else {
		m_impl->selected_portfolio = "";
	}
}

//============================================================================
AtlasXPortfolioManager::~AtlasXPortfolioManager() noexcept
{
	delete m_impl;
}

//============================================================================
ads::CDockWidget*
AtlasXPortfolioManager::make(
	QWidget* parent,
	AtlasXAppImpl* app
)
{
	ads::CDockWidget* dock = new ads::CDockWidget(
		"AtlasX Portfolio Manager"
	);
	auto node = new AtlasXPortfolioManager(dock, app);
	node->setFocusPolicy(Qt::NoFocus);
	dock->setWidget(node);
	dock->setIcon(QIcon("./styles/icons/piechart.png"));
	return dock;
}

}