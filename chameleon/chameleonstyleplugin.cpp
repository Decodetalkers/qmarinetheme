#include "chameleonstyleplugin.h"
#include "chameleonstyle.h"

QStringList ChameleonStylePlugin::keys() const
{
    return {"chameleonstyle"};
}
//! [0]

//! [1]
QStyle *ChameleonStylePlugin::create(const QString &key)
{
    if (QStringLiteral("chameleon") != key) {
        return nullptr;
    }
    return new chameleon::ChameleonStyle();
}
