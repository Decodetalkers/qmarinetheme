#pragma once

#include <QObject>
#include <optional>
#include <qdbusextratypes.h>
#include <qvariant.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))
#include <private/qgenericunixtheme_p.h>
#else
#include <private/qgenericunixthemes_p.h>
#endif
#include <qpa/qplatformtheme.h>
#include <sys/types.h>

class MarinePlatformTheme
    : public QObject,
      public QGenericUnixTheme {
    Q_OBJECT
public:

    explicit MarinePlatformTheme();

    ~MarinePlatformTheme() = default;

    virtual bool usePlatformNativeDialog(DialogType type) const override;

    virtual QPlatformDialogHelper* createPlatformDialogHelper(DialogType type) const override;

    virtual QVariant themeHint(ThemeHint hint) const override;

    virtual QIcon fileIcon(const QFileInfo& fileInfo,
        QPlatformTheme::IconOptions iconOptions = {}) const override;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    virtual Qt::ColorScheme colorScheme() const override;
#endif

#ifdef SUPPORT_KDE
    virtual QIconEngine* createIconEngine(const QString& iconName) const override;
#endif

private slots:
    void createFsWatcher();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void xdpSettingsChanged(QString xdp_namespace, QString key, QDBusVariant value);
#endif

private:
    void readSettings();
    void readXdgDesktopPortalVersion();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void xdpSettingsInit();
    void colorSchemeUpdateCheck(uint scheme);
#endif

private:
    QPlatformTheme* m_filechoosertheme;
    bool m_useXdgDesktopPortal;
    int m_useXdgDesktopPortalVersion;
    QPlatformTheme* m_basetheme;
    QStringList m_stylename;
    QString m_iconTheme;
    std::optional<u_int> m_scrollLen;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    Qt::ColorScheme m_colorScheme;
#endif
};
