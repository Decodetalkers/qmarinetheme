#include "simplestyleplugin.h"
#include "simplestyle.h"
#include <QDebug>
//! [0]
QStringList SimpleStylePlugin::keys() const
{
    return {"SimpleStyle"};
}
//! [0]

//! [1]
QStyle *SimpleStylePlugin::create(const QString &key)
{
    if (key.toLower() == "simplestyle")
        return new SimpleStyle;
    return nullptr;
}
//! [1]
