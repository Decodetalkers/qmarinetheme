#include "marinePlatformTheme.h"

#include <QLoggingCategory>

#include <QVariant>
#include <functional>
#include <optional>
#include <qpa/qplatformthemefactory_p.h>
#include <qt/QtWidgets/qstylefactory.h>
#include <string>

Q_LOGGING_CATEGORY(MarineTheme, "MarineTheme")

const std::optional<QVariant> XCURSOR_THEME = std::invoke([]() -> std::optional<QString> {
    if (qEnvironmentVariableIsSet("XCURSOR_THEME")) {
        return qEnvironmentVariable("XCURSOR_THEME");
    }
    return std::nullopt;
});

const std::optional<QVariant> XCURSOR_SIZE = std::invoke([]() -> std::optional<QString> {
    if (qEnvironmentVariableIsSet("XCURSOR_SIZE")) {
        return qEnvironmentVariable("XCURSOR_SIZE");
    }
    return std::nullopt;
});

MarinePlatformTheme::MarinePlatformTheme()
  : m_theme(QPlatformThemeFactory::create("xdgdesktopportal"))
{
    // qDebug() << QStyleFactory::keys();
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
    case QPlatformTheme::MouseCursorTheme:
        return XCURSOR_THEME.value_or(QPlatformTheme::themeHint(hint));
    case QPlatformTheme::MouseCursorSize:
        return XCURSOR_SIZE.value_or(QPlatformTheme::themeHint(hint));
    default:
        return QPlatformTheme::themeHint(hint);
        break;
    }
}
