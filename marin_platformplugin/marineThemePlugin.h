#include <qpa/qplatformthemeplugin.h>

class MarinePlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformThemeFactoryInterface.5.1" FILE "marine.json")

public:
    QPlatformTheme *create(const QString &key, const QStringList &params) override;
};
