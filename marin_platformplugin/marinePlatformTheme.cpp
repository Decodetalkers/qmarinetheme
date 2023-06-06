#include "marinePlatformTheme.h"

#include <QLoggingCategory>

#include <QVariant>
#include <qpa/qplatformthemefactory_p.h>
#include <qt/QtWidgets/qstylefactory.h>
#include <string>
Q_LOGGING_CATEGORY(MarineTheme, "MarineTheme")

constexpr std::string THEMENAME = "marine";

MarinePlatformTheme::MarinePlatformTheme()
  : m_theme(QPlatformThemeFactory::create("xdgdesktopportal"))
{
    qDebug() << QStyleFactory::keys();
}

bool
MarinePlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    return m_theme->usePlatformNativeDialog(type);
}

QPlatformDialogHelper *
MarinePlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    return m_theme->createPlatformDialogHelper(type);
}

QVariant
MarinePlatformTheme::themeHint(ThemeHint hint) const
{
    switch (hint) {
    case QPlatformTheme::StyleNames:
        return QStringList() << "adwaita";
        break;
    default:
        return QPlatformTheme::themeHint(hint);
        break;
    }
}
