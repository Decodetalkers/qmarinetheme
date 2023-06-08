#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QStyleFactory>

#include <QDebug>
#include <qpa/qplatformthemefactory_p.h>

int
main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineOption getStyles(QStringList{"style", "Show styles"});
    QCommandLineOption getPlatFormThemes(QStringList{"platforms", "Show platform themes"});

    QCommandLineParser parser;

    parser.addHelpOption();

    parser.addOption(getStyles);
    parser.addOption(getPlatFormThemes);

    parser.process(app);

    if (parser.isSet(getStyles)) {
        qDebug() << "available Styles: " << QStyleFactory::keys();
    } else if (parser.isSet(getPlatFormThemes)) {
        qDebug() << "available Styles: " << QPlatformThemeFactory::keys();
    }

    return 0;
}
