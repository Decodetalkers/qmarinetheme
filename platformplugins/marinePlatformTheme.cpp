#include "marinePlatformTheme.h"

#include <QLoggingCategory>

#include <QFile>
#include <QVariant>
#include <format>
#include <functional>
#include <optional>
#include <qpa/qplatformthemefactory_p.h>
#include <qstandardpaths.h>
#include <qt/QtWidgets/qstylefactory.h>
#include <string>

using namespace std::string_view_literals;

constexpr auto TOML_EXAMPLE = R"(
    theme = "hello world"
    dialogType = "xdgdesktopportal"
    iconstyle = "breeze"
)"sv;

Q_LOGGING_CATEGORY(MarineTheme, "MarineTheme")

constexpr std::string CONFIGDIR = "MarineTheme";

constexpr std::string SAVECONFIG = "setting.toml";

constexpr std::string DEFAULT_FILECHOOSER = "default";

constexpr auto FILECHOOSER_AVAILABLE = R"(
    default : default filechooser,
    gtk2: gtk2 filechooser, theme will be qt5gtk2
    gtk3: gtk3 filechooser, theme will be qt5gtk3
    xdgdesktopportal: use xdgdesktopportal filechooser
)"sv;

static QString
get_config_path()
{
    return QString::fromStdString(
      std::format("{}/{}/{}",
                  QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString(),
                  CONFIGDIR,
                  SAVECONFIG));
}

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
  : m_filechoosertheme(QPlatformThemeFactory::create("xdgdesktopportal"))
{
    QString configpath = get_config_path();
    QFile file(configpath);
    if (!file.exists()) {
        qCDebug(MarineTheme) << "There is not a configfile, there is an example";
        qCDebug(MarineTheme).noquote() << TOML_EXAMPLE;
        qCDebug(MarineTheme) << "Write it in " << configpath;
    }

    // qDebug() << QStyleFactory::keys();
}

bool
MarinePlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    return m_filechoosertheme->usePlatformNativeDialog(type);
}

QPlatformDialogHelper *
MarinePlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    return m_filechoosertheme->createPlatformDialogHelper(type);
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
