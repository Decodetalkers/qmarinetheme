#pragma once

#include <QObject>
#include <optional>
#include <private/qgenericunixthemes_p.h>
#include <qpa/qplatformtheme.h>
#include <sys/types.h>

class MarinePlatformTheme
  : public QObject
  , public QGenericUnixTheme
{
    Q_OBJECT
public:
    explicit MarinePlatformTheme();

    ~MarinePlatformTheme() = default;

    virtual bool usePlatformNativeDialog(DialogType type) const override;

    virtual QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const override;

    virtual QVariant themeHint(ThemeHint hint) const override;

    virtual QIcon fileIcon(const QFileInfo &fileInfo,
                           QPlatformTheme::IconOptions iconOptions = {}) const override;

#ifdef SUPPORT_KDE
    virtual QIconEngine *createIconEngine(const QString &iconName) const override;
#endif

private:
    void readSettings();
    void readXdgDesktopPortalVersion();

private:
    QPlatformTheme *m_filechoosertheme;
    bool m_useXdgDesktopPortal;
    int m_useXdgDesktopPortalVersion;
    QPlatformTheme *m_basetheme;
    QStringList m_stylename;
    QString m_iconTheme;
    std::optional<u_int> m_scrollLen;
};
