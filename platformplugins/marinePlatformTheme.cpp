#include "marinePlatformTheme.h"
#include "qxdgdesktopportalfiledialog_p.h"

#include <QLoggingCategory>

#include <QGuiApplication>
#include <QStyleFactory>
#include <qpa/qplatformthemefactory_p.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QIcon>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QStyleHints>
#include <QTimer>
#include <QVariant>
#include <QWidget>
#include <qpa/qwindowsysteminterface.h>
#include <toml++/toml.h>

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

constexpr std::string COLOR_THEME = "color-scheme";

const std::string SETTINGS_NAMESPACE = "org.freedesktop.appearance";

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

static QString
get_config_dir()
{
    return QString::fromStdString(
      std::format("{}/{}",
                  QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString(),
                  CONFIGDIR));
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
  : m_basetheme(QPlatformThemeFactory::create("gtk3"))
  , m_useXdgDesktopPortal(false)
  , m_useXdgDesktopPortalVersion(0)
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
  , m_colorScheme(Qt::ColorScheme::Unknown)
#endif
{
    readSettings();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    xdpSettingsInit();
#endif
    if (QGuiApplication::desktopSettingsAware()) {
        QMetaObject::invokeMethod(this, "createFsWatcher", Qt::QueuedConnection);
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void
MarinePlatformTheme::xdpSettingsInit()
{
    QDBusMessage message =
      QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                     QStringLiteral("/org/freedesktop/portal/desktop"),
                                     QStringLiteral("org.freedesktop.portal.Settings"),
                                     QStringLiteral("ReadOne"));
    message << QString::fromStdString(SETTINGS_NAMESPACE) << QString::fromStdString(COLOR_THEME);
    QDBusPendingCall pendingCall     = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(
      watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
          QDBusPendingReply<QDBusVariant> reply = *watcher;
          if (reply.isError()) {
              qCDebug(MarineTheme) << "D-Bus Error" << reply.reply().errorMessage();
              watcher->deleteLater();
              return;
          }
          uint theme = reply.value().variant().toUInt();

          colorSchemeUpdateCheck(theme);
          watcher->deleteLater();
      });
    QDBusConnection::sessionBus().connect(QStringLiteral("org.freedesktop.portal.Desktop"),
                                          QStringLiteral("/org/freedesktop/portal/desktop"),
                                          QStringLiteral("org.freedesktop.portal.Settings"),
                                          QStringLiteral("SettingChanged"),
                                          this,
                                          SLOT(xdpSettingsChanged(QString, QString, QDBusVariant)));
}

void
MarinePlatformTheme::colorSchemeUpdateCheck(uint theme)
{
    Qt::ColorScheme new_scheme = Qt::ColorScheme::Unknown;

    if (theme == 1) {
        new_scheme = Qt::ColorScheme::Dark;
    } else if (theme == 2) {
        new_scheme = Qt::ColorScheme::Light;
    } else {
        new_scheme = Qt::ColorScheme::Unknown;
    }
    bool need_update = (m_colorScheme != new_scheme);
    if (need_update) {
        m_colorScheme = new_scheme;

        QWindowSystemInterface::handleThemeChange();
    }
}

void
MarinePlatformTheme::xdpSettingsChanged(QString xdp_namespace, QString key, QDBusVariant value)
{
    if (xdp_namespace != SETTINGS_NAMESPACE) {
        return;
    }
    if (key != COLOR_THEME) {
        return;
    }
    uint theme = value.variant().toUInt();

    colorSchemeUpdateCheck(theme);
}
#endif
void
MarinePlatformTheme::createFsWatcher()
{
    QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
    watcher->addPath(get_config_dir());

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(3000);
    connect(
      watcher, &QFileSystemWatcher::directoryChanged, timer, [timer](QString) { timer->start(); });
    connect(timer, &QTimer::timeout, this, [this] { readSettings(); });
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
            auto dialogtype_v = QString::fromStdString(dialogtype.value());
            if (dialogtype_v == "xdgdesktopportal") {
                m_useXdgDesktopPortal = true;
                readXdgDesktopPortalVersion();
            } else {
                m_useXdgDesktopPortal = false;
                const auto themelist  = myThemeName();
                bool notfindkey =
                  std::find(themelist.begin(), themelist.end(), dialogtype.value()) ==
                  themelist.end();
                if (pakeys.contains(dialogtype_v) && notfindkey) {
                    m_filechoosertheme = QPlatformThemeFactory::create(dialogtype_v);
                } else {
                    qCDebug(MarineTheme) << dialogtype_v << " Not in keys ";
                    qCDebug(MarineTheme) << "all Themes" << pakeys;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    qCDebug(MarineTheme) << "Should not set" << myThemeName();
#endif
                    m_filechoosertheme =
                      QPlatformThemeFactory::create(QString::fromStdString(DEFAULT_FILECHOOSER));
                }
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

void
MarinePlatformTheme::readXdgDesktopPortalVersion()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop",
                                                          "/org/freedesktop/portal/desktop",
                                                          "org.freedesktop.DBus.Properties",
                                                          "Get");
    message << "org.freedesktop.portal.FileChooser"
            << "version";
    QDBusPendingCall pendingCall     = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     watcher,
                     [this](QDBusPendingCallWatcher *watcher) {
                         QDBusPendingReply<QVariant> reply = *watcher;
                         if (reply.isValid()) {
                             this->m_useXdgDesktopPortalVersion = reply.value().toUInt();
                         }
                         watcher->deleteLater();
                     });
}

bool
MarinePlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    if (m_useXdgDesktopPortal) {
        return m_basetheme->usePlatformNativeDialog(type);
    }
    return m_filechoosertheme ? m_filechoosertheme->usePlatformNativeDialog(type)
                              : QPlatformTheme::usePlatformNativeDialog(type);
}

QPlatformDialogHelper *
MarinePlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    if (type == FileDialog && m_useXdgDesktopPortal) {
        // Older versions of FileChooser portal don't support opening directories, therefore we
        // fallback to native file dialog opened inside the sandbox to open a directory.
        if (m_basetheme->usePlatformNativeDialog(type))
            return new QXdgDesktopPortalFileDialog(static_cast<QPlatformFileDialogHelper *>(
                                                     m_basetheme->createPlatformDialogHelper(type)),
                                                   m_useXdgDesktopPortalVersion);

        return new QXdgDesktopPortalFileDialog;
    }
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

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
Qt::ColorScheme
MarinePlatformTheme::colorScheme() const
{
    return m_colorScheme;
}
#endif

#ifdef SUPPORT_KDE
QIconEngine *
MarinePlatformTheme::createIconEngine(const QString &iconName) const
{
    return new KIconEngine(iconName, KIconLoader::global());
}
#endif

#ifdef DEBUGMODE
namespace Tests {

constexpr bool
check_cycle_marine()
{
    const auto themelist     = myThemeName();
    const std::string name_a = "marine";
    return (std::find(themelist.begin(), themelist.end(), name_a) != themelist.end());
}

constexpr bool
check_cycle_marine_test()
{
    const auto themelist     = myThemeName();
    const std::string name_a = "marine_test";
    return (std::find(themelist.begin(), themelist.end(), name_a) != themelist.end());
}

constexpr bool
check_cycle_marine_qt5ct()
{
    const auto themelist     = myThemeName();
    const std::string name_a = "qt5ct";
    return (std::find(themelist.begin(), themelist.end(), name_a) != themelist.end());
}

constexpr bool
check_cycle_marine_xdg()
{
    const auto themelist     = myThemeName();
    const std::string name_a = "xdgdesktopportal";
    return (std::find(themelist.begin(), themelist.end(), name_a) != themelist.end());
}

constexpr void
check()
{
    static_assert(check_cycle_marine(), "Check Keyword marine");
    static_assert(check_cycle_marine_test(), "Check Keyword marine_test");
    static_assert(check_cycle_marine_qt5ct(), "Check Keyword qt5ct");
    static_assert(!check_cycle_marine_xdg(), "Check Keyword xdgdesktopportal not in list");
}

}
#endif
