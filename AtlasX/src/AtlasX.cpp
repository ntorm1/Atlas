#include <filesystem>

#include "../include/AtlasX.h"
#include "../include/AtlasXImpl.h"
#include "../include/AtlasXExchangeManager.h"
#include "../include/AtlasXStrategyManager.h"


#include <QLabel>
#include <QFile>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <qmessagebox.h>

#include "DockManager.h"
#include "DockComponentsFactory.h"
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"

using namespace ads;

namespace fs = std::filesystem;

namespace AtlasX
{


//============================================================================
struct AtlasXAppState
{
    String exe_path;
    fs::path env_path;

    QLabel* state_label = nullptr;
    QLabel* sim_time_label = nullptr;
    QLabel* sim_time_next_label = nullptr;
    QProgressBar* progress_bar = nullptr;

    AtlasXAppState()
	{
		exe_path = QCoreApplication::applicationDirPath().toStdString();
	}
};


//============================================================================
static QIcon svgIcon(const QString& File)
{
    // This is a workaround, because in item views SVG icons are not
    // properly scaled and look blurry or pixelate
    QIcon SvgIcon(File);
    SvgIcon.addPixmap(SvgIcon.pixmap(92));
    return SvgIcon;
}


//============================================================================
class CCustomComponentsFactory : public ads::CDockComponentsFactory
{
public:
    using Super = ads::CDockComponentsFactory;
    ads::CDockAreaTitleBar* createDockAreaTitleBar(ads::CDockAreaWidget* DockArea) const override
    {
        auto TitleBar = new ads::CDockAreaTitleBar(DockArea);
        auto CustomButton = new QToolButton(DockArea);
        CustomButton->setToolTip(QObject::tr("Help"));
        //CustomButton->setIcon(svgIcon(":/adsdemo/images/help_outline.svg"));
        CustomButton->setAutoRaise(true);
        int Index = TitleBar->indexOf(TitleBar->button(ads::TitleBarButtonTabsMenu));
        TitleBar->insertWidget(Index + 1, CustomButton);
        return TitleBar;
    }
};


//============================================================================
AtlasXApp::AtlasXApp(QWidget* parent
) :
    QMainWindow(parent),
    ui(new Ui::AtlasXClass())
{
    ui->setupUi(this);
    
    m_state = new AtlasXAppState();
    m_impl = new AtlasXAppImpl();

    resize(1280, 720);
    setGeometry(QStyle::alignedRect(
        Qt::LeftToRight, Qt::AlignCenter, frameSize(),
        QGuiApplication::primaryScreen()->availableGeometry()
    ));


    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::AllTabsHaveCloseButton, true);
    CDockManager::setConfigFlag(CDockManager::DragPreviewIsDynamic, true);
    CDockManager::setConfigFlag(CDockManager::DragPreviewHasWindowFrame, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);

    m_DockManager = new ads::CDockManager(this);

    auto exchange_manager_dock = AtlasXExchangeManager::make(
        this,
        m_impl
    );
    m_impl->exchange_manager = static_cast<AtlasXExchangeManager*>(exchange_manager_dock->widget());
    exchange_manager_dock->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    auto DockArea = m_DockManager->addDockWidget(
        ads::CenterDockWidgetArea,
        exchange_manager_dock
    );


    auto strategy_manager_dock = AtlasXStrategyManager::make(
        this,
        m_impl
    );
    strategy_manager_dock->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    DockArea = m_DockManager->addDockWidgetTabToArea(
		strategy_manager_dock,
        DockArea
   );


    initStyle();
    initToolBar();
    initStateBar();
    initSignals();
    initEnvironment();
    m_impl->env_path = m_state->env_path.string();
}



//============================================================================
void
AtlasXApp::initSignals() noexcept
{
    // connect hydraRestore signal to the exchange manager
    connect(
        this,
        &AtlasXApp::hydraRestore,
        m_impl->exchange_manager,
        &AtlasXExchangeManager::onHydraRestore
    );
    connect(
        this,
        &AtlasXApp::hydraStep,
        m_impl->exchange_manager,
        &AtlasXExchangeManager::onHydraStep
    );
}


//============================================================================
void
AtlasXApp::initStyle()
{
    qDebug() << "INIT STYLE APPLY";
    QFile file("./styles/vs_light.css");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        QString stylesheet = stream.readAll();
        file.close();

        // Apply the stylesheet to the main window
        m_DockManager->setStyleSheet(stylesheet);
    }
    else
    {
        assert(false);
    }
    qDebug() << "INIT STYLE COMPLETE";
}


//============================================================================
void
AtlasXApp::initToolBar() noexcept
{
    auto tool_bar = new QToolBar(this);
    tool_bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QAction* a = new QAction("Restore", tool_bar);
    a->setProperty("Floating", true);
    a->setToolTip("Restore hydra instance from env path");
    a->setIcon(svgIcon("./styles/icons/restore.png"));
    connect(a, &QAction::triggered, this, &AtlasXApp::initEnvironment);
    tool_bar->addAction(a);

    a = new QAction("Save", tool_bar);
    a->setToolTip("Save hydra instance to env path");
    a->setIcon(svgIcon("./styles/icons/save.png"));
    connect(a, &QAction::triggered, this, &AtlasXApp::saveEnvironment);
    tool_bar->addAction(a);

    QWidget* spacerWidget = new QWidget(tool_bar);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tool_bar->addWidget(spacerWidget);

    a = new QAction("Build", tool_bar);
    a->setProperty("Floating", true);
    a->setToolTip("Build hydra state");
    a->setIcon(svgIcon("./styles/icons/build.png"));
    connect(a, &QAction::triggered, this, &AtlasXApp::build);
    tool_bar->addAction(a);

    a = new QAction("Step", tool_bar);
    a->setProperty("Floating", true);
    a->setToolTip("Step simulation forward one step");
    a->setIcon(svgIcon("./styles/icons/next.png"));
    connect(a, &QAction::triggered, this, &AtlasXApp::step);
    tool_bar->addAction(a);

    a = new QAction("Run", tool_bar);
    a->setProperty("Floating", true);
    a->setToolTip("Run simulation");
    a->setIcon(svgIcon("./styles/icons/run.png"));
    connect(a, &QAction::triggered, this, &AtlasXApp::run);
    tool_bar->addAction(a);

    addToolBar(Qt::LeftToolBarArea, tool_bar);
}


//============================================================================
void
AtlasXApp::initStateBar() noexcept
{
    auto tool_bar = new QToolBar(this);

    // add 30 point spacer to the left
    QWidget* spacerWidget = new QWidget(tool_bar);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	tool_bar->addWidget(spacerWidget);


    auto current_time = m_impl->convertNanosecondsToTime(m_impl->currentGlobalTime());
    auto next_time = m_impl->convertNanosecondsToTime(m_impl->nextGlobalTime());

    // add a label to the status bar
    m_state->state_label = new QLabel("Status: INIT", this);
    m_state->sim_time_label = new QLabel(("Current Time: " + current_time).c_str(), this);
    m_state->sim_time_next_label = new QLabel(("Next Time: " + next_time).c_str(), this);
    m_state->progress_bar = new QProgressBar(this);
    m_state->progress_bar->setRange(0, 100);
    m_state->progress_bar->setValue(0);
    m_state->progress_bar->setFixedWidth(100);


    tool_bar->addWidget(m_state->state_label);
    tool_bar->addSeparator();
    tool_bar->addWidget(m_state->sim_time_label);
    tool_bar->addSeparator();
    tool_bar->addWidget(m_state->sim_time_next_label);
    tool_bar->addSeparator();
    tool_bar->addWidget(m_state->progress_bar);
    tool_bar->setMinimumHeight(30);  // Adjust the height as needed
    addToolBar(Qt::BottomToolBarArea, tool_bar);
}


//============================================================================
void 
AtlasXApp::updateStateBar() noexcept
{
    auto current_time = m_impl->convertNanosecondsToTime(m_impl->currentGlobalTime());
	auto next_time = m_impl->convertNanosecondsToTime(m_impl->nextGlobalTime());

	m_state->sim_time_label->setText(("Current Time: " + current_time).c_str());
	m_state->sim_time_next_label->setText(("Next Time: " + next_time).c_str());
}


//============================================================================
void
AtlasXApp::step() noexcept
{
    auto current_idx = m_impl->getCurrentIdx();
    auto sim_length = m_impl->getTimestamps().size();

    if (sim_length == 0)
    {
        QMessageBox::critical(this, "Error", "Simulation has not been built yet");
        return;
    }

    if ((current_idx + 1) == sim_length)
    {
        m_state->state_label->setText("Status: FINISHED");
        updateStateBar();
        return;
    }
    m_impl->step();
    m_state->state_label->setText("Status: RUNING");

    // update progress bar with relative progress
    auto fraction = static_cast<float>(current_idx+1) / static_cast<float>(sim_length);
    auto percent = static_cast<int>(fraction * 100.0f);
    m_state->progress_bar->setValue(percent);
    updateStateBar();
    emit hydraStep();
}


//============================================================================
void
AtlasXApp::run() noexcept
{
    m_impl->run();
}


//============================================================================
void
AtlasXApp::build() noexcept
{
    auto res = m_impl->build();
    if (!res) {
		const auto& error = res.error();
		const QString errorMsg = QString::fromStdString(error.what());
		qCritical() << "FAILED TO BUILD HYDRA: " << errorMsg;
		QMessageBox::critical(this, "Error", "Failed to build hydra: " + errorMsg);
		return;
	}
    m_state->state_label->setText("Status: BUILT");
    updateStateBar();
}



//============================================================================
void
AtlasXApp::saveEnvironment() noexcept
{
	qDebug() << "SAVE ENVIRONMENT";

	auto hydra_config_path = m_state->env_path / "hydra_config.json";
	auto res = m_impl->serialize(hydra_config_path.string());
    if (!res) {
        const auto& error = res.error();
        const QString errorMsg = QString::fromStdString(error.what());
        qCritical() << "FAILED TO SAVE HYDRA CONFIG TO: " << hydra_config_path.string().c_str() << " ERROR: " << errorMsg;
        QMessageBox::critical(this, "Error", "Failed to save hydra config: " + QString::fromStdString(hydra_config_path.string()) + "\nError: " + errorMsg);
        return;
    }
	qDebug() << "SAVE ENVIRONMENT COMPLETE";
}


//============================================================================
void
AtlasXApp::initEnvironment() noexcept
{
    String env_name = "default";
    qDebug() << "INIT ENVIRONMENT";

    fs::path exe_path(m_state->exe_path);

    // Check to see of env master folder exists yet
    auto env_parent_path = exe_path / "envs";
    if (!fs::exists(env_parent_path))
    {
        fs::create_directory(env_parent_path);
    }

    // Create folder for the actual env being created
    m_state->env_path = env_parent_path / env_name;
    if (!fs::exists(m_state->env_path))
    {
        fs::create_directory(m_state->env_path);
    }

    // load in the hydra config file if it exists
    auto hydra_config_path = m_state->env_path / "hydra_config.json";
    if (fs::exists(hydra_config_path))
	{
		qDebug() << "LOADING HYDRA CONFIG";
		auto res = m_impl->deserialize(hydra_config_path.string());
        if (!res)
        {
			qDebug() << "FAILED TO LOAD HYDRA CONFIG FROM: " << hydra_config_path.string().c_str();
            return;
        }
	}
    emit hydraRestore();
    qDebug() << "LOADING ENV: " + env_name + " COMPLETE";
}


//============================================================================


AtlasXApp::~AtlasXApp()
{
    delete ui;
    delete m_impl;
    delete m_state;
}

}