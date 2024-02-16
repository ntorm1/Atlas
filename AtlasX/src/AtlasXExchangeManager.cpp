
#include "../include/AtlasXExchangeManager.h"
#include "../include/AtlasXExchangeWidget.h"
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



namespace AtlasX
{


//============================================================================
struct AtlasXExchangeManagerImpl
{
    Vector<String> exchange_ids;
};


//============================================================================
AtlasXExchangeManager::AtlasXExchangeManager(
	QWidget* parent,
    AtlasXAppImpl* app
) :
	QMainWindow(parent),
    m_app(app),
    m_impl(new AtlasXExchangeManagerImpl)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto tool_bar = new QToolBar(this);

	// add new exchange tool button
	const QIcon new_Exchange_icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/add.png"));
	auto new_exchange_action = new QAction(new_Exchange_icon, tr("&New Exchange"), this);
	new_exchange_action->setStatusTip(tr("Create a new exchange"));
	new_exchange_action->setShortcut(QKeySequence::New);
	connect(
		new_exchange_action, &QAction::triggered,
		this, &AtlasXExchangeManager::newExchange
	);
    tool_bar->addAction(new_exchange_action);

    // add a select exchange tool button
    const QIcon select_Exchange_icon = QIcon::fromTheme("document-new", QIcon("./styles/icons/select.png"));
    auto select_exchange_action = new QAction(select_Exchange_icon, tr("&Select Exchange"), this);
    select_exchange_action->setStatusTip(tr("Select and exsisting exchange"));
    connect(
        select_exchange_action, &QAction::triggered,
        this, &AtlasXExchangeManager::selectExchange
    );
    tool_bar->addAction(select_exchange_action);
    addToolBar(Qt::TopToolBarArea, tool_bar);
    buildUI();
    buildSignals();
}



//============================================================================
void
AtlasXExchangeManager::buildUI()
{
    // Set up the layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    setCentralWidget(new QWidget(this));
    centralWidget()->setLayout(mainLayout);
}

//============================================================================
void
AtlasXExchangeManager::buildSignals()
{
    // connect exchange added signal 
    connect(
		this,
        SIGNAL(exchangeAdded(SharedPtr<Atlas::Exchange>, String)),
		this,
        SLOT(onAddExchange(SharedPtr<Atlas::Exchange>, String))
    );

    // connect exchange selected signal
    connect(
		this,
		SIGNAL(exchangeSelected(SharedPtr<Atlas::Exchange>, String)),
		this,
		SLOT(onExchangeSelected(SharedPtr<Atlas::Exchange>, String))
	);
}


//============================================================================
AtlasXExchangeManager::~AtlasXExchangeManager()
{
	delete m_impl;
}


//============================================================================
void
AtlasXExchangeManager::onExchangeSelected(
    SharedPtr<Atlas::Exchange> exchange, 
    String exchange_id)
{
    auto widget = new AtlasXExchange(
        this,
        m_app,
        exchange,
        exchange_id
    );
    connectExchange(widget);
    setCentralWidget(widget);
}


//============================================================================
void
AtlasXExchangeManager::connectExchange(AtlasXExchange* exchange_widget)
{
    connect(
        this,
        &AtlasXExchangeManager::hydraStep,
        exchange_widget,
        &AtlasXExchange::onHydraStep
    );
    connect(
		this,
		&AtlasXExchangeManager::hydraReset,
		exchange_widget,
		&AtlasXExchange::onHydraReset
	);
}


//============================================================================
void
AtlasXExchangeManager::onHydraStep()
{
    emit hydraStep();
}

void AtlasXExchangeManager::onHydraReset()
{
    emit hydraReset();
}


//============================================================================
void
AtlasXExchangeManager::newExchange()
{
    // Create a new dialog window
    QDialog dialog(this);
    dialog.setWindowTitle(tr("New Exchange"));

    // Create widgets for the dialog
    QLabel* nameLabel = new QLabel(tr("Exchange Name:"), this);
    QLineEdit* nameLineEdit = new QLineEdit(&dialog);

    QLabel* locationLabel = new QLabel(tr("Exchange Location:"));
    QLineEdit* locationLineEdit = new QLineEdit(&dialog);
    QPushButton* browseButton = new QPushButton(tr("Browse"), &dialog);

    // Create layout for the dialog
    QFormLayout* formLayout = new QFormLayout(&dialog);
    formLayout->addRow(nameLabel, nameLineEdit);

    QHBoxLayout* locationLayout = new QHBoxLayout(this);
    locationLayout->addWidget(locationLabel);
    locationLayout->addWidget(locationLineEdit);
    locationLayout->addWidget(browseButton);
    formLayout->addRow(locationLayout);

    // Connect signals and slots
    connect(browseButton, &QPushButton::clicked, [&]() {
        QString directory = QFileDialog::getExistingDirectory(&dialog, tr("Select Folder"), QDir::homePath(), QFileDialog::DontUseNativeDialog);

        if (!directory.isEmpty()) {
            locationLineEdit->setText(directory);
        }
        });

    // Add buttons to the dialog
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    formLayout->addWidget(buttonBox);

    // Connect the dialog buttons
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Execute the dialog and handle user input
    if (dialog.exec() == QDialog::Accepted) {
        // Retrieve values from the dialog
        QString exchangeName = nameLineEdit->text();
        QString exchangeLocation = locationLineEdit->text();

        auto new_exchange_res = m_app->addExchange(
            exchangeName.toStdString(),
            exchangeLocation.toStdString()
        );

        if (!new_exchange_res) {
            QMessageBox::critical(
                &dialog,
                tr("Error"),
                tr("Failed to create a new exchange: %1").arg(new_exchange_res.error().what())
            );
            dialog.reject();
            return;
        }
        qDebug() << "Exchange created: " << exchangeName << " source: " << exchangeLocation;
        emit exchangeAdded(
            *new_exchange_res,
            exchangeName.toStdString()
        );
        dialog.accept();
    }
}


//============================================================================
void AtlasXExchangeManager::selectExchange()
{
// Create a new dialog window
	QDialog dialog(this);
	dialog.setWindowTitle(tr("Select Exchange"));

	// Create widgets for the dialog
	QLabel* nameLabel = new QLabel(tr("Exchange Name:"), this);

    // Create a combo box for the exchanges
    auto combo = new QComboBox(&dialog);
	for (auto& exchange_id : m_impl->exchange_ids) {
        combo->addItem(exchange_id.c_str());
	}

	// Create layout for the dialog
	QFormLayout* formLayout = new QFormLayout(&dialog);
	formLayout->addRow(nameLabel, combo);

	// Add buttons to the dialog
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	formLayout->addWidget(buttonBox);

	// Connect the dialog buttons
	connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	// Execute the dialog and handle user input
	if (dialog.exec() == QDialog::Accepted) {
		// Retrieve values from the dialog
		QString exchangeName = combo->currentText();
		auto exchange = m_app->getExchange(exchangeName.toStdString());
		if (!exchange) {
			QMessageBox::critical(
				&dialog,
				tr("Error"),
				tr("Failed to select exchange: %1").arg(exchange.error().what())
			);
			dialog.reject();
			return;
		}
		qDebug() << "Exchange selected: " << exchangeName;
        emit exchangeSelected(
			*exchange,
			exchangeName.toStdString()
		);
		dialog.accept();
	}
}


//============================================================================
void 
AtlasXExchangeManager::onHydraRestore()
{
    auto const& exchange_ids = m_app->getExchangeIds();
    m_impl->exchange_ids.clear();
    for (auto const& [exchange_id, index] : exchange_ids) 
    {
		m_impl->exchange_ids.push_back(exchange_id);
	}
}


//============================================================================
void AtlasXExchangeManager::onAddExchange(
    SharedPtr<Atlas::Exchange> exchange,
    String exchangeName
)
{
    m_impl->exchange_ids.push_back(exchangeName);
}


//============================================================================
ads::CDockWidget*
AtlasXExchangeManager::make(
    QWidget* parent,
    AtlasXAppImpl* app
)
{
	ads::CDockWidget* dock = new ads::CDockWidget(
		"AtlasX Exchange Manager"
	);
	auto node = new AtlasXExchangeManager(dock, app);
	node->setFocusPolicy(Qt::NoFocus);
	dock->setWidget(node);
    dock->setIcon(QIcon("./styles/icons/exchange.png"));
	return dock;
}


}
