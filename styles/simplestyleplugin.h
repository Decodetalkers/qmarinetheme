#ifndef SIMPLESTYLEPLUGIN_H
#define SIMPLESTYLEPLUGIN_H

#include <QStylePlugin>

//! [0]
class SimpleStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "simplestyle.json")

public:
    SimpleStylePlugin() = default;

    QStringList keys() const;
    QStyle *create(const QString &key) override;
};
//! [0]

#endif
