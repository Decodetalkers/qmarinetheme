#ifndef NORWEGIANWOODSTYLE_H
#define NORWEGIANWOODSTYLE_H

#include <QPalette>
#include <QProxyStyle>

QT_BEGIN_NAMESPACE
class QPainterPath;
QT_END_NAMESPACE

//! [0]
class NorwegianWoodStyle : public QProxyStyle
{
    Q_OBJECT

public:
    NorwegianWoodStyle();

    void polish(QPalette &palette) override;
    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option,
                    const QWidget *widget) const override;
    int styleHint(StyleHint hint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const override;
    void drawPrimitive(PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget) const override;
    void drawControl(ControlElement control,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget) const override;

private:
    static void setTexture(QPalette &palette, QPalette::ColorRole role, const QPixmap &pixmap);
    static QPainterPath roundRectPath(const QRect &rect);
};
//! [0]

#endif
