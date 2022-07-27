#include "chameleonstyle.h"
#include <DApplication>
#include <DNativeSettings>
#include <DPlatformTheme>
#include <DPlatformWindowHandle>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {

QBrush ChameleonStyle::generatedBrush(
    StateFlags flags, const QBrush &base, QPalette::ColorGroup cg, QPalette::ColorRole role, const QStyleOption *option) const
{
    return DStyle::generatedBrush(flags, base, cg, role, option);
}

QBrush ChameleonStyle::generatedBrush(DStyle::StateFlags flags,
                                      const QBrush &base,
                                      QPalette::ColorGroup cg,
                                      DPalette::ColorType type,
                                      const QStyleOption *option) const
{
    return DStyle::generatedBrush(flags, base, cg, type, option);
}
}  // namespace chameleon
