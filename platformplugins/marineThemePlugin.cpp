#include "marineThemePlugin.h"
#include "marinePlatformTheme.h"

QPlatformTheme *
MarinePlatformThemePlugin::create(const QString &key, const QStringList &)
{
    if (key.toLower() == "marine" || key.toLower() == "qt5ct"
#ifdef DEBUGMODE
        || key.toLower() == "marine_test"
#endif
#ifdef USE_AS_KDE_PLUGIN
        || key.toLower() == "kde"
#endif
    ) {
        return new MarinePlatformTheme;
    }
    return nullptr;
}
