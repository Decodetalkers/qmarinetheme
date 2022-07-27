#pragma once

#include <QStylePlugin>

class ChameleonStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "chameleon.json")

public:
    ChameleonStylePlugin() = default;

    QStringList keys() const;
    QStyle *create(const QString &key) override;
};
