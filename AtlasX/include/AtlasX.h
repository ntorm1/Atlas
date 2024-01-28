#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AtlasX.h"
#include "DockManager.h"

#include "AtlasXTypes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AtlasXClass; };
QT_END_NAMESPACE



namespace AtlasX
{

struct AtlasXAppImpl;
struct AtlasXAppState;

class AtlasXApp : public QMainWindow
{
    Q_OBJECT

signals:
    void hydraRestore();
    void hydraStep();

private:
    Ui::AtlasXClass* ui;
    ads::CDockManager* m_DockManager;
    AtlasXAppImpl* m_impl;
    AtlasXAppState* m_state;

    void initStyle();
    void initSignals() noexcept;
    void initStateBar() noexcept;
    void initToolBar() noexcept;
    void initEnvironment() noexcept;
    void saveEnvironment() noexcept;
    void updateStateBar() noexcept;

    void step() noexcept;
    void run() noexcept;
    void build() noexcept;

public:
    AtlasXApp(QWidget* parent = nullptr);
    ~AtlasXApp();


};


}