#include "marineThemePlugin.h"
#include "marinePlatformTheme.h"

QPlatformTheme *MarinePlatformThemePlugin::create(const QString &key, const QStringList &)
{
    if (key.toLower() == "marine" || key.toLower() == "qt5ct") {
        return new MarinePlatformTheme;
    }
    return nullptr;
}
