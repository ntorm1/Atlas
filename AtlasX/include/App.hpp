#pragma once

#include "ui_AtlasX.h"
#include <QtWidgets/QMainWindow>
#include "AtlasXTypes.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class AtlasXClass;
};
QT_END_NAMESPACE

namespace AtlasX {

struct AppImpl;

//============================================================================
class App : public QMainWindow {
  Q_OBJECT

private:
  UniquePtr<AppImpl> m_impl;
  Ui::AtlasXClass *ui;

  void initState() noexcept;
  void initStyle() noexcept;
  void initToolbar() noexcept;
  void initMenu() noexcept;

  void build() noexcept;
  void step() noexcept;
  void run() noexcept;
  void reset() noexcept;

  AtlasXEditor* createEditor() noexcept;
  void createNewSpace() noexcept;

  String nextComponentId(ComponentType type) noexcept;
  void linkEditor(AtlasXEditor* editor) noexcept;
  void linkFileBrowser(AtlasXFileBrowser* tree) noexcept;
  void linkComponent(UniquePtr<AtlasXComponent> component) noexcept;
  void onEditorIdsRequest() noexcept;

private slots:
  void onFileOpenRequest(String const& editor_id, fs::path const &path) noexcept;

signals:
  void onNewSpace(String const &env_dir);
  void onEditorIds(Vector<String> ids);

public:
  App(QWidget *parent = nullptr) noexcept;
  ~App() noexcept;
};

} // namespace AtlasX