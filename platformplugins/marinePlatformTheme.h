#pragma once

#include <QObject>
#include <optional>
#include <qpa/qplatformtheme.h>
#include <sys/types.h>

class MarinePlatformTheme
  : public QObject
  , public QPlatformTheme
{
    Q_OBJECT
public:
    explicit MarinePlatformTheme();

    ~MarinePlatformTheme() = default;

    virtual bool usePlatformNativeDialog(DialogType type) const override;

    virtual QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const override;

    virtual QVariant themeHint(ThemeHint hint) const override;

#ifdef SUPPORT_KDE
    virtual QIconEngine *createIconEngine(const QString &iconName) const override;
#endif

private:
    void readSettings();

private:
    QPlatformTheme *m_filechoosertheme;
    QStringList m_stylename;
    QString m_iconTheme;
    std::optional<u_int> m_scrollLen;
};
