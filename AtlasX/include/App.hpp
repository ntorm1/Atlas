#pragma once

#include "ui_AtlasX.h"
#include <QtWidgets/QMainWindow>
#include "DockManager.h"
#include "AtlasXTypes.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class AtlasXClass;
};
QT_END_NAMESPACE

namespace AtlasX {

struct AppImpl;

class App : public QMainWindow {
  Q_OBJECT

private:
  UniquePtr<AppImpl> m_impl;
  Ui::AtlasXClass *ui;
  ads::CDockManager *m_DockManager;

  void initState() noexcept;
  void initStyle() noexcept;
  void initToolbar() noexcept;
  void initMenu() noexcept;
  void createNewSpace() noexcept;

  String nextComponentId(ComponentType type) noexcept;
  void linkComponent(UniquePtr<AtlasXComponent> component) noexcept;

signals:
  void onNewSpace(String const &env_dir);

public:
  App(QWidget *parent = nullptr) noexcept;
  ~App() noexcept;
};

} // namespace AtlasX