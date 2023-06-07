#include <qpa/qplatformthemeplugin.h>

class MarinePlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformThemeFactoryInterface.5.1" FILE
#ifdef DEBUGMODE
                          "marinetest.json")
#else
                          "marine.json")
#endif

public:
    QPlatformTheme *create(const QString &key, const QStringList &params) override;
};
