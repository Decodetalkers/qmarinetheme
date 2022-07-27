
#include "chameleonstyle.h"
#include <DApplication>
#include <DButtonBox>
#include <DNativeSettings>
#include <DPlatformTheme>
#include <DPlatformWindowHandle>
#include <QLineEdit>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {

void ChameleonStyle::unpolish(QWidget *w)
{
    DStyle::unpolish(w);

    resetAttribute(w, false);

    if (w && qobject_cast<QLineEdit *>(w)) {
        w->setProperty("_d_dtk_lineeditActionWidth", QVariant());
        w->setProperty("_d_dtk_lineeditActionMargin", QVariant());
    }
}

void ChameleonStyle::unpolish(QApplication *application)
{
    DStyle::unpolish(application);
}

}  // namespace chameleon
