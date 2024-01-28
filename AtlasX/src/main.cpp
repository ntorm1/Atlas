
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
    

    //QFile f("./styles/QDarkStyleSheet/qdarkstyle/light/lightstyle.qss");
    QFile f("./styles/vs_light.css");
    if (!f.exists()) {
        printf("Unable to set stylesheet, file not found\n");
        return 1;
    }
    else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
    
    AtlasXApp w;
    w.show();
    return a.exec();
}
