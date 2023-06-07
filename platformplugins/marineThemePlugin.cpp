#include "marineThemePlugin.h"
#include "marinePlatformTheme.h"

QPlatformTheme *
MarinePlatformThemePlugin::create(const QString &key, const QStringList &)
{
    if (key.toLower() == "marine" || key.toLower() == "qt5ct"
#ifdef DEBUGMODE
        || key.toLower() == "marine_test"
#endif
    ) {
        return new MarinePlatformTheme;
    }
    return nullptr;
}
