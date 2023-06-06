#pragma once

#include <qpa/qplatformtheme.h>

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

private:
    QPlatformTheme *m_theme;
};
