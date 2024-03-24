#include "fstream"
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>

#include "App.hpp"
#include "AppUtil.hpp"

#include "AtlasXTree.hpp"

namespace fs = std::filesystem;
using namespace ads;

BEGIN_NAMESPACE_ATLASX

//============================================================================
struct AppImpl {
  String exe_path;
  fs::path env_path;
  Map<String, UniquePtr<AtlasXComponent>> components;

  fs::path envBase() const noexcept {
    auto parent_dir = fs::path(exe_path).parent_path();
    return parent_dir / "envs";
  }

  void initSpace(String env_name) noexcept {
    ATLASX_DEBUG("init space: {}", env_name);
    env_path = envBase() / env_name;
    if (!fs::exists(env_path)) {
      if (env_name == "default") {
        fs::create_directory(env_path);
      }
      ATLASX_CRITICAL("expected dir: {}", env_path.string());
      return;
    }

    deleteFilesRecursively(env_path);

    static const char *hydra_toml = R"([hydra]
version = "0.11.0"
)";
    auto toml_path = env_path / "hydra.toml";
    try {
      std::ofstream toml_file(toml_path);
      toml_file << hydra_toml;
    } catch (std::exception &e) {
      ATLASX_CRITICAL("failed to write hydra.toml: {}", toml_path.string());
    }
    ATLASX_DEBUG("loaded space {} from {}", env_name, env_path.string());
  }

  AppImpl() noexcept {
    exe_path = QCoreApplication::applicationDirPath().toStdString();
    env_path = envBase() / "default";
  }
};

//============================================================================
App::App(QWidget *parent) noexcept
    : QMainWindow(parent), ui(new Ui::AtlasXClass()) {
  ui->setupUi(this);
  m_impl = std::make_unique<AppImpl>();
  CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
  CDockManager::setConfigFlag(CDockManager::AllTabsHaveCloseButton, true);
  CDockManager::setConfigFlag(CDockManager::DragPreviewIsDynamic, true);
  CDockManager::setConfigFlag(CDockManager::DragPreviewHasWindowFrame, false);
  CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
  m_DockManager = new ads::CDockManager(this);

  initState();
  initStyle();
  initToolbar();
  initMenu();
  m_impl->initSpace("default");
  emit onNewSpace(m_impl->env_path.string());
}

//============================================================================
App::~App() noexcept { delete ui; }

//============================================================================
void App::initState() noexcept {
  ads::CDockWidget *dock = new ads::CDockWidget("AtlasX Space", this);
  auto tree = std::make_unique<AtlasXFileBrowser>(
      nextComponentId(AtlasXFileBrowser::compType()), dock, m_impl->envBase());
  dock->setWidget(tree.get());
  auto dock_area = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, dock);
  dock->setIcon(QIcon("./styles/icons/fileopen.png"));
  linkComponent(std::move(tree));
}

//============================================================================
void App::initStyle() noexcept {
  qDebug() << "INIT STYLE APPLY";
  QFile file("./styles/vs_light.css");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&file);
    QString stylesheet = stream.readAll();
    file.close();
    m_DockManager->setStyleSheet(stylesheet);
  } else {
    qCritical() << "Failed to open stylesheet file";
  }
  qDebug() << "INIT STYLE COMPLETE";
}

//============================================================================
void App::initToolbar() noexcept {}

//============================================================================
void App::initMenu() noexcept {
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QAction *newAction = fileMenu->addAction("&New");
  QAction *openAction = fileMenu->addAction("&Save");
  QAction *exitAction = fileMenu->addAction("&Exit");

  connect(newAction, &QAction::triggered, this, &App::createNewSpace);
}

//============================================================================
void App::createNewSpace() noexcept {
  QDialog dialog(this);
  dialog.setWindowTitle("New Space");
  QLabel *nameLabel = new QLabel("Name:");
  QLineEdit *nameEdit = new QLineEdit(&dialog);
  QFormLayout *formLayout = new QFormLayout(&dialog);
  QHBoxLayout *nameLayout = new QHBoxLayout(&dialog);
  nameLayout->addWidget(nameLabel);
  nameLayout->addWidget(nameEdit);
  formLayout->addRow(nameLayout);
  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  formLayout->addWidget(buttonBox);

  // Connect the dialog buttons
  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  // Execute the dialog and handle user input
  if (dialog.exec() == QDialog::Accepted) {
    auto name = nameEdit->text();
    auto env_dir = m_impl->envBase() / name.toStdString();
    if (fs::exists(env_dir)) {
      QMessageBox::StandardButton confirmOverride = QMessageBox::question(
          this, "Confirm Override",
          QFORMAT("{} Exsits, do you want to replace?", env_dir.string()),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (confirmOverride == QMessageBox::No) {
        return;
      }
    }
    fs::create_directories(env_dir);
    m_impl->initSpace(name.toStdString());
    dialog.accept();
  }
}

//============================================================================
String App::nextComponentId(ComponentType comp_type) noexcept {
  int comp_count = 0;
  for (auto const &[id, comp] : m_impl->components) {
    if (comp->type() == comp_type) {
      comp_count++;
    }
  }
  return std::format("{}_{}", componentTypeToString(comp_type), comp_count);
}

//============================================================================
void App::linkComponent(UniquePtr<AtlasXComponent> component) noexcept {
  connect(this, &App::onNewSpace, component.get(),
          &AtlasXComponent::onNewSpace);
  m_impl->components[component->id()] = std::move(component);
}

END_NAMESPACE_ATLASX