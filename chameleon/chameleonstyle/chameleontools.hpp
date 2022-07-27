#pragma once
#include <DApplicationHelper>
#include <QCalendarWidget>
#include <QColor>
#include <QTabBar>
#include <QScrollBar>
#include <QTextCharFormat>
static QColor getThemTypeColor(QColor lightColor, QColor darkColor)
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        return lightColor;
    else
        return darkColor;
}
// 判断是TabBar否为竖直方向
inline static bool verticalTabs(QTabBar::Shape shape)
{
    return shape == QTabBar::RoundedWest || shape == QTabBar::RoundedEast || shape == QTabBar::TriangularWest ||
           shape == QTabBar::TriangularEast;
}
static void updateWeekendTextFormat(QCalendarWidget *calendar, QColor)
{
    if (!calendar)
        return;

    QTextCharFormat fmt;
    fmt.setForeground(QBrush(calendar->palette().highlight()));
    calendar->setWeekdayTextFormat(Qt::Saturday, fmt);
    calendar->setWeekdayTextFormat(Qt::Sunday, fmt);
}
static QWidget *getSbarParentWidget(QScrollBar *sbar)
{
    if (!sbar)
        return nullptr;
    QWidget *pw = sbar->parentWidget();
    if (!pw)
        return nullptr;

    bool isContainer = !pw->objectName().compare(QLatin1String("qt_scrollarea_vcontainer")) ||
                       !pw->objectName().compare(QLatin1String("qt_scrollarea_hcontainer"));

    return isContainer ? pw->parentWidget() : pw;
}
