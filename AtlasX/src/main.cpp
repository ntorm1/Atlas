
#include "../include/AtlasX.h"

#include <QtWidgets/QApplication>
#include <QFile>

using namespace AtlasX;

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if QT_VERSION >= 0x050600
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#endif
    QApplication a(argc, argv);
    
    a.setApplicationName("AtlasX");
    a.setQuitOnLastWindowClosed(true);
    
    QFile StyleSheetFile("./styles/vs_light.css");
    StyleSheetFile.open(QIODevice::ReadOnly);
    QTextStream StyleSheetStream(&StyleSheetFile);
    a.setStyleSheet(StyleSheetStream.readAll());
    StyleSheetFile.close();
    
    AtlasXApp w;
    w.show();
    return a.exec();
}
