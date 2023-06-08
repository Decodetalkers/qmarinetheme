#include "marinePlatformTheme.h"

#include <QLoggingCategory>

#include <QStyleFactory>
#include <qpa/qplatformthemefactory_p.h>

#include <QFile>
#include <QStandardPaths>
#include <QVariant>

#include <toml++/toml.h>

#include <QFileInfo>
#include <QIcon>
#include <QMimeDatabase>
#include <QMimeType>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#ifdef SUPPORT_KDE
#include <KIconLoader>
#include <QIconEngine>
#include <kiconengine.h>
#endif

using namespace std::string_view_literals;

constexpr auto TOML_EXAMPLE = R"(
    theme = "Adwaita"
    dialogtype = "xdgdesktopportal"
    iconstyle = "breeze"
    wheelscroll = 1
)"sv;

Q_LOGGING_CATEGORY(MarineTheme, "MarineTheme")

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
constexpr std::string CONFIGDIR = "marinetheme6";
#else
constexpr std::string CONFIGDIR = "marinetheme5";
#endif

constexpr std::string SAVECONFIG = "setting.toml";

constexpr std::string DEFAULT_FILECHOOSER = "default";

constexpr std::string DEFAULT_THEME = "Adwaita";

constexpr std::string DEFAULT_ICON = "Adwaita";

constexpr std::vector<std::string>
myThemeName()
{
    return {"marine",
            "qt5ct"
#ifdef DEBUGMODE
            ,
            "marine_test"
#endif
    };
};

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
        std::optional<u_int> scroll           = tbl["wheelscroll"].value<u_int>();
        auto pakeys                           = QPlatformThemeFactory::keys();
        if (dialogtype.has_value()) {
            auto dialogtype_v    = QString::fromStdString(dialogtype.value());
            const auto themelist = myThemeName();
            bool notfindkey =
              std::find(themelist.begin(), themelist.end(), dialogtype.value()) == themelist.end();
            if (pakeys.contains(dialogtype_v) && notfindkey) {
                m_filechoosertheme = QPlatformThemeFactory::create(dialogtype_v);
            } else {
                qCDebug(MarineTheme) << dialogtype_v << " Not in keys ";
                qCDebug(MarineTheme) << "all Themes" << pakeys;
                qCDebug(MarineTheme) << "Should not set" << myThemeName();
                m_filechoosertheme =
                  QPlatformThemeFactory::create(QString::fromStdString(DEFAULT_FILECHOOSER));
            }
        } else {
            qCDebug(MarineTheme) << " Not set filechooser, available is " << pakeys;
            m_filechoosertheme =
              QPlatformThemeFactory::create(QString::fromStdString(DEFAULT_FILECHOOSER));
        }
        auto themekeys = QStyleFactory::keys();
        if (themekeys.contains(QString::fromStdString(theme.value_or("")))) {
            m_stylename = theme.has_value() ? QStringList{QString::fromStdString(theme.value())}
                                            : QStringList{};
        } else {
            if (theme.has_value()) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                qCDebug(MarineTheme) << "Theme" << theme.value() << "not in keys";
#else
                qCDebug(MarineTheme)
                  << "Theme" << QString::fromStdString(theme.value()) << "not in keys";
#endif
                qCDebug(MarineTheme) << "Available is " << themekeys;
            }
        }

        m_iconTheme = QString::fromStdString(iconstyle.value_or(DEFAULT_ICON));

        m_scrollLen = scroll;

    } catch (const toml::parse_error &err) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        qWarning(MarineTheme) << "Error parsing file '" << *err.source().path << "':\n"
                              << err.description() << "\n  (" << err.source().begin.line << ")\n";
#else
        qWarning(MarineTheme) << "Error parsing file '"
                              << QString::fromStdString(*err.source().path) << "':\n"
                              << QString::fromStdString(err.description().data()) << "\n  ("
                              << err.source().begin.line << ")\n";
#endif
        qCDebug(MarineTheme) << "There is not a configfile, there is an example";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        qCDebug(MarineTheme).noquote() << TOML_EXAMPLE;
#else
        qCDebug(MarineTheme).noquote() << QString::fromStdString(TOML_EXAMPLE.data());
#endif
        qCDebug(MarineTheme) << "Write it in " << configpath;

        m_filechoosertheme =
          QPlatformThemeFactory::create(QString::fromStdString(DEFAULT_FILECHOOSER));
        if (QStyleFactory::keys().contains(QString::fromStdString(DEFAULT_THEME))) {
            m_stylename.append(QString::fromStdString(DEFAULT_THEME));
        } else {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            qCWarning(MarineTheme) << "Theme " << DEFAULT_THEME << "not contains in themelists";
#else
            qCWarning(MarineTheme)
              << "Theme " << QString::fromStdString(DEFAULT_THEME) << "not contains in themelists";
#endif
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
    case QPlatformTheme::WheelScrollLines:
        return m_scrollLen.value_or(QPlatformTheme::themeHint(hint).toInt());
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    case QPlatformTheme::MouseCursorTheme:
        return XCURSOR_THEME.value_or(QPlatformTheme::themeHint(hint));
    case QPlatformTheme::MouseCursorSize:
        return XCURSOR_SIZE.value_or(QPlatformTheme::themeHint(hint));
#endif
    default:
        return QGenericUnixTheme::themeHint(hint);
    }
}

QIcon
MarinePlatformTheme::fileIcon(const QFileInfo &fileInfo,
                              QPlatformTheme::IconOptions iconOptions) const
{
    if ((iconOptions & DontUseCustomDirectoryIcons) && fileInfo.isDir())
        return QIcon::fromTheme(QLatin1String("inode-directory"));

    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(fileInfo);
    return QIcon::fromTheme(type.iconName());
}

#ifdef SUPPORT_KDE
QIconEngine *
MarinePlatformTheme::createIconEngine(const QString &iconName) const
{
    return new KIconEngine(iconName, KIconLoader::global());
}
#endif
