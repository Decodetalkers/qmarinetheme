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
#include <toml++/toml.h>

using namespace std::string_view_literals;

constexpr auto TOML_EXAMPLE = R"(
    theme = "hello world"
    dialogtype = "xdgdesktopportal"
    iconstyle = "breeze"
)"sv;

Q_LOGGING_CATEGORY(MarineTheme, "MarineTheme")

constexpr std::string CONFIGDIR = "marinetheme5";

constexpr std::string SAVECONFIG = "setting.toml";

constexpr std::string DEFAULT_FILECHOOSER = "default";

constexpr std::string DEFAULT_THEME = "adwaita";

constexpr std::string DEFAULT_ICON = "adwaita";

constexpr auto FILECHOOSER_AVAILABLE = R"(
    gtk3: gtk3 filechooser
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
{
    readSettings();
}

void
MarinePlatformTheme::readSettings()
{
    QString configpath = get_config_path();
    try {
        auto tbl                              = toml::parse_file(configpath.toStdString());
        std::optional<std::string> theme      = tbl["theme"].value<std::string>();
        std::optional<std::string> dialogtype = tbl["dialogtype"].value<std::string>();
        std::optional<std::string> iconstyle  = tbl["iconstyle"].value<std::string>();
        auto keys                             = QPlatformThemeFactory::keys();
        if (dialogtype.has_value()) {
            auto dialogtype_v = QString::fromStdString(dialogtype.value());
            if (keys.contains(dialogtype_v)) {
                m_filechoosertheme = QPlatformThemeFactory::create(dialogtype_v);
            } else {
                qCDebug(MarineTheme) << dialogtype_v << " Not in " << keys;
                m_filechoosertheme =
                  QPlatformThemeFactory::create(QString::fromStdString(DEFAULT_FILECHOOSER));
            }
        } else {
            qCDebug(MarineTheme) << " Not set filechooser, available is " << keys;
            m_filechoosertheme =
              QPlatformThemeFactory::create(QString::fromStdString(DEFAULT_FILECHOOSER));
        }

        m_stylename =
          theme.has_value() ? QStringList{QString::fromStdString(theme.value())} : QStringList{};
        m_iconTheme = QString::fromStdString(iconstyle.value_or(DEFAULT_ICON));

    } catch (const toml::parse_error &err) {
        qWarning(MarineTheme) << "Error parsing file '" << *err.source().path << "':\n"
                              << err.description() << "\n  (" << err.source().begin.line << ")\n";
        qCDebug(MarineTheme) << "There is not a configfile, there is an example";
        qCDebug(MarineTheme).noquote() << TOML_EXAMPLE;
        qCDebug(MarineTheme) << "Write it in " << configpath;

        m_filechoosertheme =
          QPlatformThemeFactory::create(QString::fromStdString(DEFAULT_FILECHOOSER));
        if (QStyleFactory::keys().contains(QString::fromStdString(DEFAULT_THEME))) {
            m_stylename.append(QString::fromStdString(DEFAULT_THEME));
        } else {
            qCWarning(MarineTheme) << "Theme " << DEFAULT_THEME << "not contains in themelists";
            qCDebug(MarineTheme) << "available themelists: " << QStyleFactory::keys();
        }
        m_iconTheme = QString::fromStdString(DEFAULT_ICON);
    }
}

bool
MarinePlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    return m_filechoosertheme ? m_filechoosertheme->usePlatformNativeDialog(type)
                              : QPlatformTheme::usePlatformNativeDialog(type);
}

QPlatformDialogHelper *
MarinePlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    return m_filechoosertheme ? m_filechoosertheme->createPlatformDialogHelper(type)
                              : QPlatformTheme::createPlatformDialogHelper(type);
}

QVariant
MarinePlatformTheme::themeHint(ThemeHint hint) const
{
    switch (hint) {
    case QPlatformTheme::StyleNames:
        return m_stylename;
    case QPlatformTheme::SystemIconThemeName:
        return m_iconTheme;
    case QPlatformTheme::MouseCursorTheme:
        return XCURSOR_THEME.value_or(QPlatformTheme::themeHint(hint));
    case QPlatformTheme::MouseCursorSize:
        return XCURSOR_SIZE.value_or(QPlatformTheme::themeHint(hint));
    default:
        return QPlatformTheme::themeHint(hint);
    }
}
