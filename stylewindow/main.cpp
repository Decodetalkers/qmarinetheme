#include "widgetgallery.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>

int
main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(styles);

    QApplication app(argc, argv);
    app.setApplicationVersion("11");

    WidgetGallery gallery;
    gallery.show();

    return app.exec();
}
