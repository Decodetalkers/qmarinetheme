#include "chameleonstyle.h"
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {
QStyle::SubControl ChameleonStyle::hitTestComplexControl(QStyle::ComplexControl cc,
                                                         const QStyleOptionComplex *opt,
                                                         const QPoint &pt,
                                                         const QWidget *w) const
{
    return DStyle::hitTestComplexControl(cc, opt, pt, w);
}
}  // namespace chameleon
