/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "chameleonstyle.h"
#include "common.h"
#include "chameleonstyle/chameleontools.hpp"
#include <DApplication>
#include <DApplicationHelper>
#include <DButtonBox>
#include <DNativeSettings>
#include <DPlatformTheme>
#include <DPlatformWindowHandle>
#include <DSearchEdit>
#include <DSlider>
#include <DStyleOption>
#include <DTabBar>
#include <DWindowManagerHelper>

#include <DDateTimeEdit>
#include <DIconButton>
#include <DListView>
#include <DSpinBox>
#include <DTabBar>
#include <DTreeView>
#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QCheckBox>
#include <QTableView>
#include <QToolButton>
#include <QtGlobal>
#include <QtMath>
#include <dpalettehelper.h>
#include <private/qcombobox_p.h>
#include <private/qcommonstyle_p.h>

#include <qdrawutil.h>
#include <qpa/qplatformwindow.h>

#include "dstyleanimation.h"

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {






ChameleonStyle::ChameleonStyle()
    : DStyle()
{
}

// 按动画效果渐变隐藏滚动条，返回值为`true`表示隐藏滚动条，`false`表示继续绘制滚动条(改变了透明度)
bool ChameleonStyle::hideScrollBarByAnimation(const QStyleOptionSlider *scrollBar, QPainter *p) const
{
    QScrollBar *sbar = qobject_cast<QScrollBar *>(scrollBar->styleObject);
    if (!sbar)
        return false;

    // QScrollBar 可以通过设置属性 _d_slider_always_show 为 true 的方式，使得 Slider 一直显示
    // scrollBarObj 获取的即应用界面上 QScrollBar 控件本身的指针值
    if (sbar->property("_d_dtk_slider_always_show").toBool())
        return false;

    // ScrollBarAlwaysOn 也可以控制一直显示
    QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(getSbarParentWidget(sbar));
    if (sa) {
        const QScrollBar *hsb = sa->horizontalScrollBar();
        const bool hsbAlwaysOn = sa->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOn;
        if (hsb == sbar && hsbAlwaysOn)
            return false;
        const QScrollBar *vsb = sa->verticalScrollBar();
        const bool vsbAlwaysOn = sa->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOn;
        if (vsb == sbar && vsbAlwaysOn)
            return false;
    }

    auto styleAnimation = qobject_cast<dstyle::DScrollbarStyleAnimation *>(this->animation(sbar));
    if (!styleAnimation) {
        // styleAnimation -> updateTarget --sendEvent--> StyleAnimationUpdate -> repaint
        styleAnimation = new dstyle::DScrollbarStyleAnimation(dstyle::DScrollbarStyleAnimation::Deactivating, sbar);
        styleAnimation->setDeletePolicy(QAbstractAnimation::KeepWhenStopped);

        connect(
            styleAnimation, &dstyle::DStyleAnimation::destroyed, this, &ChameleonStyle::_q_removeAnimation, Qt::UniqueConnection);

        animations.insert(styleAnimation->target(), styleAnimation);

        // 滚动和滚动条大小变化时，重启动画改变显示和隐藏
        QObject::connect(sbar, &QAbstractSlider::valueChanged, styleAnimation, &dstyle::DScrollbarStyleAnimation::restart);
        QObject::connect(sbar, &QAbstractSlider::rangeChanged, styleAnimation, &dstyle::DScrollbarStyleAnimation::restart);
    }

    if (!styleAnimation)
        return false;

    QAbstractAnimation::State st = styleAnimation->state();
    bool isHoveredOrPressed = scrollBar->state & (QStyle::State_MouseOver | QStyle::State_Sunken);

    // 隐藏动画中鼠标hover上去或者按下时重启动画(计算时间)
    if (isHoveredOrPressed && st == QAbstractAnimation::Running) {
        styleAnimation->restart(true);
        return false;
    }

    if (st == QAbstractAnimation::Running) {
        p->setOpacity(styleAnimation->currentValue());
    }

    // 动画停止时不再绘制滚动条
    return st == QAbstractAnimation::Stopped;
}

// 当scrollbar透明时不关心鼠标消息。
void ChameleonStyle::transScrollbarMouseEvents(QObject *obj, bool on /*= true*/) const
{
    QScrollBar *sbar = qobject_cast<QScrollBar *>(obj);
    if (!sbar)
        return;

    sbar->setProperty("_d_dtk_slider_visible", on);
}

bool ChameleonStyle::eventFilter(QObject *watched, QEvent *event)
{
    QScrollBar *sbar = qobject_cast<QScrollBar *>(watched);
    if (!sbar)
        return false;

    QContextMenuEvent *cme = dynamic_cast<QContextMenuEvent *>(event);
    QMouseEvent *me = dynamic_cast<QMouseEvent *>(event);
    if (!cme && !me)
        return false;

    bool on = sbar->property("_d_dtk_slider_visible").toBool();
    // 有的应用会设置滚动条的 parent
    QWidget *pp = getSbarParentWidget(sbar);
    // 对于 QAbstractScrollArea 来说 item 一般在 viewport 上
    QAbstractScrollArea *itemView = qobject_cast<QAbstractScrollArea *>(pp);
    pp = itemView ? itemView->viewport() : pp;
    if (!pp)
        return false;

    // (`(!cme && !me)` is True) && (`cme` is False) ==> `me` is True, `me` can't be False.
    const QPoint viewportPos = pp->mapFromGlobal(cme ? cme->globalPos() : me->globalPos());
    QWidget *target = pp;
    QPoint localPos = viewportPos;
    if (qobject_cast<QScrollArea *>(itemView)) {
        if (target == pp->childAt(viewportPos)) {
            localPos = target->mapFrom(pp, viewportPos);
        }
    }

    // scrollbar right click
    if (cme) {
        if (target) {
            QContextMenuEvent menuEvent(cme->reason(), localPos, cme->globalPos(), cme->modifiers());
            return !on ? false : QApplication::sendEvent(target, &menuEvent);
        }
    } else {
        // 仅仅在滚动条显示时过滤鼠标(点击)事件
        if (!me || !on)
            return false;

        if (target) {
            QMouseEvent mevent = *me;
            mevent.setLocalPos(localPos);
            // 传递鼠标事件到后面的 widget
            return QApplication::sendEvent(target, &mevent);
        }
    }
    return false;
}



bool ChameleonStyle::drawTabBar(QPainter *painter, const QStyleOptionTab *tab, const QWidget *widget) const
{
    const QTabBar *m_tabbar = qobject_cast<const QTabBar *>(widget);

    if (!m_tabbar)
        return false;

    painter->fillRect(tab->rect, getColor(tab, QPalette::Window));
    painter->save();

    bool isTriangularMode = false;
    bool rtlHorTabs =
        (tab->direction == Qt::RightToLeft && (tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::RoundedSouth));
    bool selected = tab->state & State_Selected && tab->state & State_Enabled;
    bool lastTab =
        ((!rtlHorTabs && tab->position == QStyleOptionTab::End) || (rtlHorTabs && tab->position == QStyleOptionTab::Beginning));
    bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
    int tabOverlap = proxy()->pixelMetric(PM_TabBarTabOverlap, tab, widget);
    QRect rect = tab->rect.adjusted(0, 0, (onlyOne || lastTab) ? 0 : tabOverlap, 0);

    QRect r2(rect);
    int x1 = r2.left();
    int x2 = r2.right();
    int y1 = r2.top();
    int y2 = r2.bottom();

    QTransform rotMatrix;
    bool flip = false;
    painter->setPen(getColor(tab, DPalette::Shadow));

    switch (tab->shape) {
        case QTabBar::TriangularNorth:
            rect.adjust(0, 0, 0, -tabOverlap);
            isTriangularMode = true;
            break;
        case QTabBar::TriangularSouth:
            rect.adjust(0, tabOverlap, 0, 0);
            isTriangularMode = true;
            break;
        case QTabBar::TriangularEast:
            rect.adjust(tabOverlap, 0, 0, 0);
            isTriangularMode = true;
            break;
        case QTabBar::TriangularWest:
            rect.adjust(0, 0, -tabOverlap, 0);
            isTriangularMode = true;
            break;
        case QTabBar::RoundedNorth:
            break;
        case QTabBar::RoundedSouth:
            rotMatrix.rotate(180);
            rotMatrix.translate(0, -rect.height() + 1);
            rotMatrix.scale(-1, 1);
            painter->setTransform(rotMatrix, true);
            break;
        case QTabBar::RoundedWest:
            rotMatrix.rotate(180 + 90);
            rotMatrix.scale(-1, 1);
            flip = true;
            painter->setTransform(rotMatrix, true);
            break;
        case QTabBar::RoundedEast:
            rotMatrix.rotate(90);
            rotMatrix.translate(0, -rect.width() + 1);
            flip = true;
            painter->setTransform(rotMatrix, true);
            break;
    }

    if (flip) {
        QRect tmp = rect;
        rect = QRect(tmp.y(), tmp.x(), tmp.height(), tmp.width());
        int temp = x1;
        x1 = y1;
        y1 = temp;
        temp = x2;
        x2 = y2;
        y2 = temp;
    }

    QColor lineColor = !isTriangularMode || selected ? Qt::transparent : getColor(tab, QPalette::Light);
    QColor tabFrameColor = selected ? getColor(tab, QPalette::Window) : getColor(tab, QPalette::Button);

    if (!(tab->features & QStyleOptionTab::HasFrame))
        tabFrameColor = getColor(tab, QPalette::Shadow);

    if (!isTriangularMode)
        tabFrameColor = selected ? getColor(tab, QPalette::Highlight) : getColor(tab, QPalette::Button);

    QPen outlinePen(lineColor, proxy()->pixelMetric(PM_DefaultFrameWidth, tab, widget));
    QRect drawRect = rect;
    painter->setPen(outlinePen);
    painter->setBrush(tabFrameColor);
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (!isTriangularMode) {
        int buttonRadius = DStyle::pixelMetric(PM_FrameRadius, tab, widget);
        int buttonBorder = DStyle::pixelMetric(PM_FrameMargins, tab, widget);

        painter->drawRoundedRect(
            drawRect.adjusted(buttonBorder, buttonBorder, -buttonBorder, -buttonBorder), buttonRadius, buttonRadius);
    } else {
        painter->drawRect(drawRect);
    }

    painter->restore();
    return true;
}

bool ChameleonStyle::drawTabBarLabel(QPainter *painter, const QStyleOptionTab *tab, const QWidget *widget) const
{
    if (!widget)
        return false;

    bool type_check = false;
    bool selected = tab->state & State_Selected && tab->state & State_Enabled;

    if (widget)
        type_check = widget->property("_d_dtk_tabbartab_type").toBool();

    bool visible_close_button = selected;

    if (visible_close_button) {
        if (const DTabBar *tb = qobject_cast<const DTabBar *>(widget)) {
            visible_close_button = tb->tabsClosable();
        } else {
            visible_close_button = false;
        }
    }

    QStyleOptionTab newTab = *tab;

    if (selected) {
        QPalette::ColorRole role = type_check ? QPalette::Highlight : QPalette::HighlightedText;
        newTab.palette.setBrush(QPalette::WindowText, adjustColor(getColor(tab, role), 0, 0, 0, 0, 0, 0, 50));

        // 拖拽的tab不需要绘制渐变到透明，因为没有关闭按钮。拖拽标签时是生成图片 QPaintDevice 是 QPixMap
        bool is_moving_tab = painter->device()->devType() != QInternal::Widget;
        if (visible_close_button && !is_moving_tab) {
            QRect tr = proxy()->subElementRect(SE_TabBarTabText, tab, widget);
            QRect text_rect;
            if (const DTabBar *tabbar = qobject_cast<const DTabBar *>(widget)) {
                int alignment = tabbar->property("_d_dtk_tabbar_alignment").toInt();
                text_rect = tab->fontMetrics.boundingRect(tr, alignment | Qt::TextShowMnemonic, tab->text);
            } else {
                text_rect = tab->fontMetrics.boundingRect(tr, Qt::AlignCenter | Qt::TextShowMnemonic, tab->text);
            }
            int close_button_width = proxy()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, tab, widget);

            // 防止在平板中错误的对文字渐变
            if (DGuiApplicationHelper::isTabletEnvironment())
                close_button_width = TabBar_TabButtonSize;

            QBrush brush;

            //选中状态下文字颜色
            if (type_check) {
                brush = getColor(tab, QPalette::Highlight);
            } else {
                brush = newTab.palette.windowText();
            }
            // 小心除以 0
            qreal stop =
                text_rect.width() > 0 ? qreal(tr.right() - close_button_width - text_rect.x() - 5) / text_rect.width() : 1.0;
            if (stop < 1 && stop > 0 && tr.right() - close_button_width < text_rect.right()) {
                QLinearGradient lg(0, 0, 1, 0);
                QGradientStops stops;
                qreal offset = 5.0 / text_rect.width();

                // 接近关闭按钮部分的颜色渐变到透明
                stops << QGradientStop{0, brush.color()};
                stops << QGradientStop{qMax(0.0, stop - offset), brush.color()};
                stops << QGradientStop{stop, QColor(brush.color().red(), brush.color().green(), brush.color().blue(), 100)};

                // 保证文字超出最大可显示区域的部分为透明
                if (text_rect.right() > tr.right())
                    stops << QGradientStop{1 - (text_rect.right() - tr.right()) / qreal(text_rect.width()), Qt::transparent};

                stops << QGradientStop{1, Qt::transparent};

                lg.setCoordinateMode(QLinearGradient::ObjectBoundingMode);
                lg.setStops(stops);
                newTab.palette.setBrush(QPalette::WindowText, lg);
            } else {
                newTab.palette.setBrush(QPalette::WindowText, brush);
            }
        }

        // 禁止QCommonStyle中绘制默认的焦点颜色
        newTab.state &= ~QStyle::State_HasFocus;

        if (tab->state & QStyle::State_HasFocus) {
            QStyleOptionFocusRect fropt;
            fropt.QStyleOption::operator=(*tab);
            proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
        }
    } else {
        if (type_check) {
            newTab.palette.setBrush(QPalette::WindowText, QColor("#798190"));
        }

        //靠近边缘的文字渐变
        if (const DTabBar *tab = qobject_cast<const DTabBar *>(widget)) {
            if (!tab->expanding()) {
                QRect tr = proxy()->subElementRect(SE_TabBarTabText, &newTab, widget);
                int alignment = tab->property("_d_dtk_tabbar_alignment").toInt();
                QRect text_rect = newTab.fontMetrics.boundingRect(tr, alignment | Qt::TextShowMnemonic, newTab.text);
                QRect tabbar_rect = widget->findChild<QTabBar *>()->rect();

                bool vertTabs = verticalTabs(newTab.shape);
                int stopx = tabbar_rect.x() + (vertTabs ? tabbar_rect.height() : tabbar_rect.width());
                int tabX = (vertTabs ? newTab.rect.y() : text_rect.x()) + (vertTabs ? tabbar_rect.y() : tabbar_rect.x());
                int tabWidth = tabX + text_rect.width();

                if (text_rect.width() > 0 && tabX < stopx && stopx < tabWidth) {
                    const QBrush &brush = newTab.palette.windowText();
                    QLinearGradient lg(0, 0, 1, 0);
                    QGradientStops stops;
                    qreal stop = static_cast<qreal>(tabWidth - stopx) / text_rect.width();

                    stops << QGradientStop{0, brush.color()};
                    stops << QGradientStop{qMax(0.0, 1 - stop - 0.2), brush.color()};
                    stops << QGradientStop{qMax(0.0, 1 - stop), Qt::transparent};

                    lg.setCoordinateMode(QLinearGradient::ObjectBoundingMode);
                    lg.setStops(stops);
                    newTab.palette.setBrush(QPalette::WindowText, lg);
                }
            }
        }
    }

    if (const DTabBar *tb = qobject_cast<const DTabBar *>(widget)) {
        // Qt源码
        QRect tr = tab->rect;
        bool verticalTabs = tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::RoundedWest ||
                            tab->shape == QTabBar::TriangularEast || tab->shape == QTabBar::TriangularWest;

        int alignment = tb->property("_d_dtk_tabbar_alignment").toInt() | Qt::TextShowMnemonic;
        if (!proxy()->styleHint(SH_UnderlineShortcut, &newTab, widget))
            alignment |= Qt::TextHideMnemonic;

        if (verticalTabs) {
            painter->save();
            int newX, newY, newRot;
            if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                newX = tr.width() + tr.x();
                newY = tr.y();
                newRot = 90;
            } else {
                newX = tr.x();
                newY = tr.y() + tr.height();
                newRot = -90;
            }
            QTransform m = QTransform::fromTranslate(newX, newY);
            m.rotate(newRot);
            painter->setTransform(m, true);
        }
        QRect iconRect;

        tabLayout(tab, widget, &tr, &iconRect);

        tr = proxy()->subElementRect(SE_TabBarTabText, &newTab, widget);

        if (!tab->icon.isNull()) {
            QPixmap tabIcon = tab->icon.pixmap(widget ? widget->window()->windowHandle() : 0,
                                               tab->iconSize,
                                               (tab->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                                               (tab->state & State_Selected) ? QIcon::On : QIcon::Off);

            painter->drawPixmap(iconRect, tabIcon);
        }

        proxy()->drawItemText(painter,
                              tr.adjusted(1, 0, 0, 0),
                              alignment,
                              newTab.palette,
                              newTab.state & State_Enabled,
                              newTab.text,
                              QPalette::WindowText);
        if (verticalTabs)
            painter->restore();
    } else {
        QCommonStyle::drawControl(CE_TabBarTabLabel, &newTab, painter, widget);
    }

    return true;
}

void ChameleonStyle::tabLayout(const QStyleOptionTab *opt, const QWidget *widget, QRect *textRect, QRect *iconRect) const
{
    QRect tr = opt->rect;
    bool verticalTabs = opt->shape == QTabBar::RoundedEast || opt->shape == QTabBar::RoundedWest ||
                        opt->shape == QTabBar::TriangularEast || opt->shape == QTabBar::TriangularWest;
    if (verticalTabs)
        tr.setRect(0, 0, tr.height(), tr.width());  // 0, 0 as we will have a translate transform

    int verticalShift = proxy()->pixelMetric(QStyle::PM_TabBarTabShiftVertical, opt, widget);
    int horizontalShift = proxy()->pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, opt, widget);
    int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, opt, widget) / 2;
    int vpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabVSpace, opt, widget) / 2;
    if (opt->shape == QTabBar::RoundedSouth || opt->shape == QTabBar::TriangularSouth)
        verticalShift = -verticalShift;
    tr.adjust(hpadding, verticalShift - vpadding, horizontalShift - hpadding, vpadding);
    bool selected = opt->state & QStyle::State_Selected;
    if (selected) {
        tr.setTop(tr.top() - verticalShift);
        tr.setRight(tr.right() - horizontalShift);
    }

    // left widget
    if (!opt->leftButtonSize.isEmpty()) {
        tr.setLeft(tr.left() + 4 + (verticalTabs ? opt->leftButtonSize.height() : opt->leftButtonSize.width()));
    }
    // right widget
    if (!opt->rightButtonSize.isEmpty()) {
        tr.setRight(tr.right() - 4 - (verticalTabs ? opt->rightButtonSize.height() : opt->rightButtonSize.width()));
    }

    // icon
    if (!opt->icon.isNull()) {
        QSize iconSize = opt->iconSize;
        if (!iconSize.isValid()) {
            int iconExtent = proxy()->pixelMetric(QStyle::PM_SmallIconSize);
            iconSize = QSize(iconExtent, iconExtent);
        }
        QSize tabIconSize = opt->icon.actualSize(iconSize,
                                                 (opt->state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                                                 (opt->state & QStyle::State_Selected) ? QIcon::On : QIcon::Off);
        // High-dpi icons do not need adjustment; make sure tabIconSize is not larger than iconSize
        tabIconSize = QSize(qMin(tabIconSize.width(), iconSize.width()), qMin(tabIconSize.height(), iconSize.height()));

        // 由于Qt的历史原因 center 返回的坐标总是比矩形的真是中心坐标少一个像素 导致绘制图标时竖直方向无法对齐
        *iconRect = QRect(tr.left(), tr.center().y() - tabIconSize.height() / 2 + 1, tabIconSize.width(), tabIconSize.height());
        if (!verticalTabs)
            *iconRect = proxy()->visualRect(opt->direction, opt->rect, *iconRect);
        tr.setLeft(tr.left() + tabIconSize.width() + 4);
    }

    if (!verticalTabs)
        tr = proxy()->visualRect(opt->direction, opt->rect, tr);

    *textRect = tr;
}

bool ChameleonStyle::drawTableViewItem(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    if (!qobject_cast<const QTableView *>(w))
        return false;

    const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
    if (!vopt)
        return false;

    int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
    QRect select_rect = opt->rect;

    // 设置item的背景颜色
    p->setPen(Qt::NoPen);
    if (vopt->state & QStyle::State_Selected) {
        if (!vopt->showDecorationSelected) {
            select_rect = proxy()->subElementRect(QStyle::SE_ItemViewItemText, opt, w);
        } else {
            select_rect -= frameExtentMargins();
        }
        p->setBrush(getColor(opt, QPalette::Highlight));
    } else {
        p->setBrush(vopt->backgroundBrush);
    }

    // 绘制背景，选中的item圆角由属性来控制
    if (w->property("_d_dtk_enable_tableviewitem_radius").toBool()) {
        p->setRenderHint(QPainter::Antialiasing);
        p->drawRoundedRect(select_rect, frame_radius, frame_radius);
    } else {
        // 所有的item都是非圆角
        const_cast<QStyleOptionViewItem *>(vopt)->palette.setBrush(
            vopt->palette.currentColorGroup(), QPalette::Highlight, getColor(opt, QPalette::Highlight));
        DStyle::drawPrimitive(pe, vopt, p, w);
    }

    return true;
}

bool ChameleonStyle::drawTabBarCloseButton(QPainter *painter, const QStyleOption *tab, const QWidget *widget) const
{
    if (!widget)
        return false;
    const QTabBar *tb = qobject_cast<QTabBar *>(widget->parent());

    if (!tb) {
        if (const QWidget *w = dynamic_cast<const QWidget *>(painter->device()))
            widget = w;

        tb = qobject_cast<QTabBar *>(widget->parent());
    }

    if (Q_UNLIKELY(!tb))
        return false;

    int index = -1;

    for (int i = 0; i < tb->count(); ++i) {
        if (Q_LIKELY(tb->tabButton(i, QTabBar::LeftSide) != widget && tb->tabButton(i, QTabBar::RightSide) != widget)) {
            continue;
        }

        index = i;
        break;
    }

    if (Q_UNLIKELY(index < 0))
        return true;

    QStyleOptionButton buttonOpt;
    buttonOpt.rect = tab->rect;
    buttonOpt.state = tab->state;
    QColor iconColor = getColor(&buttonOpt, QPalette::ButtonText);
    painter->setPen(QPen(iconColor, 2));
    painter->setBrush(getColor(&buttonOpt, QPalette::Button));
    DDrawUtils::drawCloseButton(painter, buttonOpt.rect);

    return true;
}

bool ChameleonStyle::drawTabBarScrollButton(QPainter *painter, const QStyleOption *opt, const QWidget *buttonWidget) const
{
    const QToolButton *tabButton = qobject_cast<const QToolButton *>(buttonWidget);

    if (!tabButton)
        return false;

    if (tabButton->arrowType() == Qt::NoArrow || !tabButton->icon().isNull())
        return false;

    bool isTriangularMode = false;
    if (QTabBar *tabBar = qobject_cast<QTabBar *>(buttonWidget->parent())) {
        switch (tabBar->shape()) {
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularSouth:
            case QTabBar::TriangularEast:
            case QTabBar::TriangularWest:
                isTriangularMode = true;
                break;
            default:
                break;
        }
    }

    QStyleOptionToolButton toolButton(*qstyleoption_cast<const QStyleOptionToolButton *>(opt));
    int frameMargin = DStyle::pixelMetric(PM_FrameMargins);
    toolButton.rect -= QMargins(frameMargin, frameMargin, frameMargin, frameMargin);

    QPoint center = toolButton.rect.center();
    qreal sizeRatio = isTriangularMode ? (4.0 / 7.0) : 1.0;
    int minBoundWidth = qMin(toolButton.rect.width(), toolButton.rect.height());
    toolButton.rect.setWidth(qRound(minBoundWidth * sizeRatio));
    toolButton.rect.setHeight(qRound(minBoundWidth * sizeRatio));
    toolButton.rect.moveCenter(center);
    DDrawUtils::Corners corner = static_cast<DDrawUtils::Corners>(0xff);

    int radius = isTriangularMode ? toolButton.rect.width() / 2 : DStyle::pixelMetric(PM_FrameRadius);
    QLinearGradient lg(QPointF(0, opt->rect.top()), QPointF(0, opt->rect.bottom()));
    lg.setColorAt(0, getColor(opt, QPalette::Light));
    lg.setColorAt(1, getColor(opt, QPalette::Dark));

    painter->setPen(QPen(getColor(opt, DPalette::FrameBorder, buttonWidget), Metrics::Painter_PenWidth));
    painter->setBrush(lg);
    painter->setRenderHint(QPainter::Antialiasing);
    drawShadow(painter, toolButton.rect + frameExtentMargins() * 1.5, getColor(&toolButton, QPalette::Shadow));
    DDrawUtils::drawRoundedRect(painter, toolButton.rect, radius, radius, corner);

    QPoint originCenter = toolButton.rect.center();
    qreal buttonSizeRatio = isTriangularMode ? 3.0 / 4.0 : 1.0 / 2.0;
    toolButton.rect.setWidth(qRound(toolButton.rect.width() * buttonSizeRatio));
    toolButton.rect.setHeight(qRound(toolButton.rect.height() * buttonSizeRatio));
    toolButton.rect.moveCenter(originCenter);
    proxy()->drawControl(CE_ToolButtonLabel, &toolButton, painter, buttonWidget);

    return true;
}

bool ChameleonStyle::drawComboBox(QPainter *painter, const QStyleOptionComboBox *comboBox, const QWidget *widget) const
{
    QRect rect(comboBox->rect);
    QStyleOptionComboBox comboBoxCopy = *comboBox;
    QRect downArrowRect = proxy()->subControlRect(CC_ComboBox, &comboBoxCopy, SC_ComboBoxArrow, widget);

    if (comboBox->frame && comboBox->subControls & SC_ComboBoxFrame) {
        int frameRadius = DStyle::pixelMetric(PM_FrameRadius);
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);

        if (comboBox->editable) {
            QBrush brush = getThemTypeColor(QColor(0, 0, 0, 255 * 0.08), QColor(255, 255, 255, 255 * 0.15));
            if (widget->testAttribute(Qt::WA_SetPalette)) {
                brush = comboBox->palette.button();
            } else if (const QComboBox *combobox = qobject_cast<const QComboBox *>(widget)) {
                if (auto lineEdit = combobox->lineEdit()) {
                    if (lineEdit->testAttribute(Qt::WA_SetPalette)) {
                        brush = lineEdit->palette().button();
                    }
                }
            }
            painter->setBrush(brush);
        } else {
            painter->setBrush(Qt::transparent);
        }

        DDrawUtils::drawRoundedRect(painter,
                                    comboBoxCopy.rect,
                                    frameRadius,
                                    frameRadius,
                                    DDrawUtils::Corner::TopLeftCorner | DDrawUtils::Corner::TopRightCorner |
                                        DDrawUtils::Corner::BottomLeftCorner | DDrawUtils::Corner::BottomRightCorner);
    }

    QStyleOptionButton buttonOption;
    buttonOption.QStyleOption::operator=(*comboBox);
    if (comboBox->editable) {
        buttonOption.rect = rect;
        buttonOption.state = (comboBox->state & (State_Enabled | State_MouseOver | State_HasFocus)) |
                             State_KeyboardFocusChange;  // Always show hig

        if (comboBox->state & State_Sunken) {
            buttonOption.state |= State_Sunken;
            buttonOption.state &= ~State_MouseOver;
        }

        if (comboBox->direction == Qt::LeftToRight)
            buttonOption.rect.setLeft(downArrowRect.left());
        else
            buttonOption.rect.setRight(downArrowRect.right());

        downArrowRect.moveCenter(buttonOption.rect.center());
        proxy()->drawPrimitive(PE_PanelButtonCommand, &buttonOption, painter, widget);
    } else {
        buttonOption.rect = rect;
        buttonOption.state = comboBox->state & (State_Enabled | State_MouseOver | State_HasFocus | State_KeyboardFocusChange);

        if (comboBox->state & State_Sunken) {
            buttonOption.state |= State_Sunken;
            buttonOption.state &= ~State_MouseOver;
        }

        downArrowRect.moveCenter(buttonOption.rect.center());

        if (comboBox->direction == Qt::LeftToRight) {
            downArrowRect.moveRight(buttonOption.rect.right());
        } else {
            downArrowRect.moveLeft(buttonOption.rect.left());
        }

        proxy()->drawPrimitive(PE_PanelButtonCommand, &buttonOption, painter, widget);
    }

    if (comboBox->subControls & SC_ComboBoxArrow) {
        QStyleOption arrowOpt = *comboBox;
        arrowOpt.rect = downArrowRect - frameExtentMargins();

        if (comboBox->editable) {
            arrowOpt.rect.setSize(QSize(qRound(buttonOption.rect.width() / 3.0), qRound(buttonOption.rect.height() / 3.0)));
            arrowOpt.rect.moveCenter(buttonOption.rect.center());
        } else {
            QPoint center = arrowOpt.rect.center();
            arrowOpt.rect.setSize(QSize(qRound(arrowOpt.rect.height() / 2.4), qRound(arrowOpt.rect.height() / 2.4)));
            arrowOpt.rect.moveCenter(center);
            int radius = DStyle::pixelMetric(PM_FrameRadius);
            arrowOpt.rect = arrowOpt.rect.adjusted(-radius, 0, -radius, 0);
        }

        painter->setPen(getColor(comboBox, DPalette::ButtonText));
        proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
    }

    if (comboBox->state.testFlag(QStyle::State_HasFocus)) {
        proxy()->drawPrimitive(PE_FrameFocusRect, comboBox, painter, widget);
    }

    return true;
}

bool ChameleonStyle::drawComboBoxLabel(QPainter *painter, const QStyleOptionComboBox *cb, const QWidget *widget) const
{
    const bool hasText(!cb->currentText.isEmpty());
    const bool hasIcon(!cb->currentIcon.isNull());
    //    const bool enabled(cb->state & QStyle::State_Enabled);
    const bool sunken(cb->state & (QStyle::State_On | QStyle::State_Sunken));
    //    const bool mouseOver(cb->state & QStyle::State_MouseOver);
    //    const bool hasFocus(cb->state & QStyle::State_HasFocus);
    const bool flat(!cb->frame);
    const bool editable(cb->editable);

    QRect contentsRect(cb->rect);
    if (sunken && !flat)
        contentsRect.translate(1, 1);
    contentsRect.adjust(
        Metrics::Layout_ChildMarginWidth, 0, -Metrics::Layout_ChildMarginWidth - DStyle::pixelMetric(PM_FrameRadius), 0);
    QSize iconSize;
    if (hasIcon) {
        iconSize = cb->iconSize;
        if (!iconSize.isValid() && widget) {
            const int metric(widget->style()->pixelMetric(QStyle::PM_SmallIconSize, cb, widget));
            iconSize = QSize(metric, metric);
        }
    }

    int textFlags(Qt::AlignVCenter | Qt::AlignLeft);
    const QSize textSize(cb->fontMetrics.size(textFlags, cb->currentText));

    if (styleHint(SH_UnderlineShortcut, cb, widget))
        textFlags |= Qt::TextShowMnemonic;
    else
        textFlags |= Qt::TextHideMnemonic;

    QRect iconRect;
    QRect textRect;
    QRect downArrowRect = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxArrow, widget);

    if (hasText && !hasIcon) {
        textRect = contentsRect;
        int leftMargin = Metrics::ComboBox_ContentLeftMargin;
        textRect.adjust(leftMargin, 0, 0, 0);
        textRect.setWidth(textRect.width() - downArrowRect.width());
    } else {
        const int contentsWidth(iconSize.width() + textSize.width() + Metrics::Button_ItemSpacing);
        const int contentLeftPadding = flat ? (contentsRect.width() - contentsWidth) / 2 : Metrics::ComboBox_ContentLeftMargin;
        iconRect = QRect(QPoint(contentsRect.left() + contentLeftPadding,
                                contentsRect.top() + (contentsRect.height() - iconSize.height()) / 2),
                         iconSize);
        const int availableTextWidth =
            contentsRect.width() - contentLeftPadding - iconSize.width() - Metrics::Icon_Margins - downArrowRect.width();
        textRect = QRect(QPoint(iconRect.right() + Metrics::Icon_Margins + 1,
                                contentsRect.top() + (contentsRect.height() - textSize.height()) / 2),
                         QSize(availableTextWidth, textSize.height()));
    }

    // handle right to left
    if (iconRect.isValid())
        iconRect = visualRect(cb->direction, cb->rect, iconRect);
    if (textRect.isValid())
        textRect = visualRect(cb->direction, cb->rect, textRect);

    // render icon
    if (hasIcon && iconRect.isValid()) {
        // icon state and mode
        cb->currentIcon.paint(painter, iconRect, Qt::AlignLeft);
    }

    // render text
    if (hasText && textRect.isValid() && !editable) {
        painter->setPen(getColor(cb, QPalette::ButtonText));
        QString text = painter->fontMetrics().elidedText(cb->currentText, Qt::ElideRight, textRect.width());
        textRect.setWidth(textRect.width() + downArrowRect.width());
        painter->drawText(textRect, textFlags, text);
    }

    return true;
}

void ChameleonStyle::drawSliderHandle(const QStyleOptionComplex *opt, QRectF &rectHandle, QPainter *p, const QWidget *w) const
{
    if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
        const DSlider *dslider = qobject_cast<const DSlider *>(w);
        QSlider::TickPosition tickPosition = slider->tickPosition;

        if (dslider)
            tickPosition = dslider->tickPosition();

        if (tickPosition == QSlider::NoTicks) {
            p->drawRoundedRect(
                rectHandle, DStyle::pixelMetric(DStyle::PM_FrameRadius), DStyle::pixelMetric(DStyle::PM_FrameRadius));
        } else {
            qreal radius = DStyle::pixelMetric(DStyle::PM_FrameRadius);
            QRectF rectRoundedPart(0, 0, 0, 0);

            if (slider->orientation == Qt::Horizontal) {
                if (tickPosition == QSlider::TicksAbove) {  //尖角朝上
                    rectRoundedPart = QRectF(rectHandle.left(), rectHandle.bottom() - 2 * radius, rectHandle.width(), 2 * radius);
                    QPointF polygon[5] = {QPointF(rectHandle.left(), rectHandle.bottom() - radius),
                                          QPointF(rectHandle.left(), rectHandle.top() + radius),
                                          QPointF(rectHandle.center().x(), rectHandle.top()),
                                          QPointF(rectHandle.right(), rectHandle.top() + radius),
                                          QPointF(rectHandle.right(), rectHandle.bottom() - radius)};
                    p->drawPolygon(polygon, 5);
                } else {  //尖角朝下
                    rectRoundedPart = QRectF(rectHandle.left(), rectHandle.top(), rectHandle.width(), 2 * radius);
                    QPointF polygon[5] = {QPointF(rectHandle.left(), rectHandle.top() + radius),
                                          QPointF(rectHandle.left(), rectHandle.bottom() - radius),
                                          QPointF(rectHandle.center().x(), rectHandle.bottom()),
                                          QPointF(rectHandle.right(), rectHandle.bottom() - radius),
                                          QPointF(rectHandle.right(), rectHandle.top() + radius)};
                    p->drawPolygon(polygon, 5);
                }
            } else {
                if (tickPosition == QSlider::TicksLeft) {  //尖角朝左
                    rectRoundedPart = QRectF(rectHandle.right() - 2 * radius, rectHandle.top(), 2 * radius, rectHandle.height());
                    QPointF polygon[5] = {QPointF(rectHandle.right() - radius, rectHandle.top()),
                                          QPointF(rectHandle.left() + radius, rectHandle.top()),
                                          QPointF(rectHandle.left(), rectHandle.center().y()),
                                          QPointF(rectHandle.left() + radius, rectHandle.bottom()),
                                          QPointF(rectHandle.right() - radius, rectHandle.bottom())};
                    p->drawPolygon(polygon, 5);
                } else {  //尖角朝右
                    rectRoundedPart = QRectF(rectHandle.left(), rectHandle.top(), 2 * radius, rectHandle.height());
                    QPointF polygon[5] = {QPointF(rectHandle.left() + radius, rectHandle.top()),
                                          QPointF(rectHandle.right() - radius, rectHandle.top()),
                                          QPointF(rectHandle.right(), rectHandle.center().y()),
                                          QPointF(rectHandle.right() - radius, rectHandle.bottom()),
                                          QPointF(rectHandle.left() + radius, rectHandle.bottom())};
                    p->drawPolygon(polygon, 5);
                }
            }
            p->drawRoundedRect(
                rectRoundedPart, DStyle::pixelMetric(DStyle::PM_FrameRadius), DStyle::pixelMetric(DStyle::PM_FrameRadius));
        }
    }
}

void ChameleonStyle::drawSliderHandleFocus(const QStyleOptionComplex *opt,
                                           QRectF &rectHandle,
                                           QPainter *p,
                                           const QWidget *w) const
{
    if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
        const DSlider *dslider = qobject_cast<const DSlider *>(w);
        QSlider::TickPosition tickPosition = slider->tickPosition;

        int lineOffset = DStyle::pixelMetric(PM_FocusBorderWidth) / 2;
        int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);
        int marginRect = DStyle::pixelMetric(PM_FocusBorderSpacing) + lineOffset;

        if (dslider)
            tickPosition = dslider->tickPosition();

        if (tickPosition == QSlider::NoTicks) {
            p->drawRoundedRect(rectHandle.adjusted(-marginRect, -marginRect, marginRect, marginRect),
                               DStyle::pixelMetric(DStyle::PM_FrameRadius) + marginRect,
                               DStyle::pixelMetric(DStyle::PM_FrameRadius) + marginRect);
        } else {
            qreal radius = DStyle::pixelMetric(DStyle::PM_FrameRadius);
            QPainterPath focusPath;

            if (slider->orientation == Qt::Horizontal) {
                if (tickPosition == QSlider::TicksAbove) {  //尖角朝上
                    focusPath.moveTo(QPointF(rectHandle.left() - marginRect, rectHandle.bottom() - radius));
                    focusPath.lineTo(QPointF(rectHandle.left() - marginRect, rectHandle.top() + radius - lineOffset));
                    focusPath.lineTo(QPointF(rectHandle.center().x(), rectHandle.top() - margin));
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.top() + radius - lineOffset));
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.bottom() - radius));
                    focusPath.arcTo(QRectF(rectHandle.right() - radius - radius - marginRect,
                                           rectHandle.bottom() - radius - radius - marginRect,
                                           2 * (radius + marginRect),
                                           2 * (radius + marginRect)),
                                    -0,
                                    -90);
                    focusPath.lineTo(QPointF(rectHandle.left() + radius, rectHandle.bottom() + marginRect));
                    focusPath.arcTo(QRectF(rectHandle.left() - marginRect,
                                           rectHandle.bottom() - radius - radius - marginRect,
                                           2 * (radius + marginRect),
                                           2 * (radius + marginRect)),
                                    -90,
                                    -90);
                } else {  //尖角朝下
                    focusPath.moveTo(QPointF(rectHandle.left() - marginRect, rectHandle.top() + radius));
                    focusPath.lineTo(QPointF(rectHandle.left() - marginRect, rectHandle.bottom() - radius + lineOffset));
                    focusPath.lineTo(QPointF(rectHandle.center().x(), rectHandle.bottom() + margin));
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.bottom() - radius + lineOffset));
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.top() + radius));
                    focusPath.arcTo(QRectF(rectHandle.right() - radius - radius - marginRect,
                                           rectHandle.top() - marginRect,
                                           2 * (radius + marginRect),
                                           2 * (radius + marginRect)),
                                    0,
                                    90);
                    focusPath.lineTo(QPointF(rectHandle.left() + radius, rectHandle.top() - marginRect));
                    focusPath.arcTo(QRectF(rectHandle.left() - marginRect,
                                           rectHandle.top() - marginRect,
                                           2 * (radius + marginRect),
                                           2 * (radius + marginRect)),
                                    90,
                                    90);
                }
            } else {
                if (tickPosition == QSlider::TicksLeft) {  //尖角朝左
                    focusPath.moveTo(QPointF(rectHandle.right() - radius, rectHandle.top() - marginRect));
                    focusPath.lineTo(QPointF(rectHandle.left() + radius - lineOffset, rectHandle.top() - marginRect));
                    focusPath.lineTo(QPointF(rectHandle.left() - margin, rectHandle.center().y()));
                    focusPath.lineTo(QPointF(rectHandle.left() + radius - lineOffset, rectHandle.bottom() + marginRect));
                    focusPath.lineTo(QPointF(rectHandle.right() - radius, rectHandle.bottom() + marginRect));
                    focusPath.arcTo(QRectF(rectHandle.right() - radius - radius - marginRect,
                                           rectHandle.bottom() - radius - radius - marginRect,
                                           2 * (radius + marginRect),
                                           2 * (radius + marginRect)),
                                    -90,
                                    90);
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.top() + radius));
                    focusPath.arcTo(QRectF(rectHandle.right() - radius - radius - marginRect,
                                           rectHandle.top() - marginRect,
                                           2 * (radius + marginRect),
                                           2 * (radius + marginRect)),
                                    0,
                                    90);
                } else {  //尖角朝右
                    focusPath.moveTo(QPointF(rectHandle.left() + radius, rectHandle.top() - marginRect));
                    focusPath.lineTo(QPointF(rectHandle.right() - radius + lineOffset, rectHandle.top() - marginRect));
                    focusPath.lineTo(QPointF(rectHandle.right() + margin, rectHandle.center().y()));
                    focusPath.lineTo(QPointF(rectHandle.right() - radius + lineOffset, rectHandle.bottom() + marginRect));
                    focusPath.lineTo(QPointF(rectHandle.left() + radius, rectHandle.bottom() + marginRect));
                    focusPath.arcTo(QRectF(rectHandle.left() - marginRect,
                                           rectHandle.bottom() - radius - radius - marginRect,
                                           2 * (radius + marginRect),
                                           2 * (radius + marginRect)),
                                    -90,
                                    -90);
                    focusPath.lineTo(QPointF(rectHandle.left() - marginRect, rectHandle.top() + radius));
                    focusPath.arcTo(QRectF(rectHandle.left() - marginRect,
                                           rectHandle.top() - marginRect,
                                           2 * (radius + marginRect),
                                           2 * (radius + marginRect)),
                                    180,
                                    -90);
                }
            }
            p->drawPath(focusPath);
        }
    }
}

void ChameleonStyle::drawIcon(const QStyleOption *opt, QPainter *p, QRect &rect, const QIcon &icon, bool checked) const
{
    bool enabled = opt->state & State_Enabled;
    bool selected = opt->state & State_Selected && enabled;
    QIcon::Mode mode = !enabled ? QIcon::Disabled : (selected ? QIcon::Selected : QIcon::Normal);

    if (mode == QIcon::Normal && opt->state & State_On)
        mode = QIcon::Selected;
    icon.paint(p, rect, Qt::AlignCenter, mode, checked ? QIcon::On : QIcon::Off);
}

#ifndef QT_NO_ANIMATION
dstyle::DStyleAnimation *ChameleonStyle::animation(const QObject *target) const
{
    return animations.value(target, nullptr);
}

void ChameleonStyle::startAnimation(dstyle::DStyleAnimation *animation, int delay) const
{
    connect(animation, &dstyle::DStyleAnimation::destroyed, this, &ChameleonStyle::_q_removeAnimation, Qt::UniqueConnection);

    animations.insert(animation->target(), animation);

    if (delay > 0) {
        QTimer::singleShot(delay, animation, SLOT(start()));
    } else {
        animation->start();
    }
}

void ChameleonStyle::_q_removeAnimation()
{
    QObject *animation = sender();
    if (animation)
        animations.remove(animation->parent());
}
#endif

bool ChameleonStyle::drawMenuBarItem(const QStyleOptionMenuItem *option,
                                     QRect &rect,
                                     QPainter *painter,
                                     const QWidget *widget) const
{
    bool enabled(option->state & QStyle::State_Enabled);
    bool mouseOver((option->state & QStyle::State_MouseOver) && enabled);
    bool sunken((option->state & QStyle::State_Sunken) && enabled);

    if (mouseOver || sunken) {
        QBrush background(getColor(option, QPalette::Highlight));
        qreal radius = DStyle::pixelMetric(DStyle::PM_FrameRadius);

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(background);
        painter->drawRoundedRect(rect, radius, radius);
    }

    if (option) {
        int alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;

        if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
            alignment |= Qt::TextHideMnemonic;

        int iconExtent = proxy()->pixelMetric(PM_SmallIconSize);
        QPixmap pix = option->icon.pixmap(widget ? widget->window()->windowHandle() : nullptr,
                                          QSize(iconExtent, iconExtent),
                                          (enabled) ? (mouseOver ? QIcon::Active : QIcon::Normal) : QIcon::Disabled);

        if (!pix.isNull()) {
            proxy()->drawItemPixmap(painter, option->rect, alignment, pix);
        } else {
            QStyleOptionMenuItem itemOption = *option;

            if (mouseOver || sunken)
                itemOption.palette.setBrush(QPalette::ButtonText, itemOption.palette.highlightedText());

            proxy()->drawItemText(
                painter, itemOption.rect, alignment, itemOption.palette, enabled, itemOption.text, QPalette::ButtonText);
        }
    }

    return true;
}

void ChameleonStyle::drawMenuItemBackground(const QStyleOption *option,
                                            QPainter *painter,
                                            QStyleOptionMenuItem::MenuItemType type) const
{
    QBrush color;
    bool selected = (option->state & QStyle::State_Enabled) && option->state & QStyle::State_Selected;
    if (selected && (DGuiApplicationHelper::isTabletEnvironment() || !DGuiApplicationHelper::isSpecialEffectsEnvironment())) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(getColor(option, QPalette::Highlight));
        painter->drawRect(option->rect);
        return;
    }

    // 清理旧的阴影
    if (option->styleObject) {
        const QRect shadow = option->styleObject->property("_d_menu_shadow_rect").toRect();
        const QRect shadow_base = option->styleObject->property("_d_menu_shadow_base_rect").toRect();

        // 如果当前菜单项时已选中的，并且shadow_base不等于当前区域，此时应当清理阴影区域
        // 如果当前要绘制的item是触发阴影绘制的那一项，那么，此时应当清空阴影区域
        if ((selected && shadow_base != option->rect) || (!selected && shadow_base == option->rect) ||
            (!selected && shadow_base.width() != option->rect.width())) {
            // 清空阴影区域
            option->styleObject->setProperty("_d_menu_shadow_rect", QVariant());
            option->styleObject->setProperty("_d_menu_shadow_base_rect", QVariant());

            // 确保阴影区域能重绘
            if (QWidget *w = qobject_cast<QWidget *>(option->styleObject)) {
                w->update(shadow);
            }
        }
    }

    if (selected) {
        color = option->palette.highlight();

        // draw shadow
        if (type == QStyleOptionMenuItem::Normal) {
            if (option->styleObject) {
                QRect shadow(0, 0, option->rect.width(), 7);
                shadow.moveTop(option->rect.bottom() + 1);
                option->styleObject->setProperty("_d_menu_shadow_rect", shadow);
                option->styleObject->setProperty("_d_menu_shadow_base_rect", option->rect);

                // 确保阴影区域能重绘
                if (QWidget *w = qobject_cast<QWidget *>(option->styleObject)) {
                    w->update(shadow);
                }
            }
        }

        painter->fillRect(option->rect, color);
    } else {
        color = option->palette.window().color();

        if (color.color().isValid()) {
            QColor c = color.color();

            // 未开启窗口混成时不应该设置背景色的alpha通道，应当显示纯色背景(设置StyleSheet时，不加载此设置，防止alpha通道等对其造成影响)
            if (DWindowManagerHelper::instance()->hasComposite() && qobject_cast<QWidget *>(option->styleObject) &&
                !qobject_cast<QWidget *>(option->styleObject)->testAttribute(Qt::WA_StyleSheet)) {
                if (DGuiApplicationHelper::toColorType(c) == DGuiApplicationHelper::LightType) {
                    c = Qt::white;
                    c.setAlphaF(0.4);
                } else {
                    //在深色背景下,为了处理QComboBox的ListView的背景色和menuItem的背景色不一致的问题,加一个判断
                    if (qobject_cast<QMenu *>(option->styleObject))
                        c = DGuiApplicationHelper::adjustColor(c, 0, 0, -10, 0, 0, 0, 0);
                    c.setAlphaF(0.8);
                }
            }

            color = c;
        }

        painter->fillRect(option->rect, color);

        if (type == QStyleOptionMenuItem::Separator) {
            QColor colorSeparator;
            DGuiApplicationHelper *guiAppHelp = DGuiApplicationHelper::instance();
            if (guiAppHelp->themeType() == DGuiApplicationHelper::ColorType::DarkType)
                colorSeparator = QColor(255, 255, 255, 255 * 0.05);
            else
                colorSeparator = QColor(0, 0, 0, 255 * 0.1);
            painter->fillRect(option->rect, colorSeparator);
        }

        if (!option->styleObject)
            return;

        // 为上一个item绘制阴影
        const QRect shadow = option->styleObject->property("_d_menu_shadow_rect").toRect();

        // 判断阴影rect是否在自己的区域
        if (!option->rect.contains(shadow.center()))
            return;

        static QColor shadow_color;
        static QPixmap shadow_pixmap;

        if (shadow_color != option->palette.color(QPalette::Active, QPalette::Highlight)) {
            shadow_color = option->palette.color(QPalette::Active, QPalette::Highlight);
            QImage image(":/chameleon/menu_shadow.svg");
            QPainter pa(&image);
            pa.setCompositionMode(QPainter::CompositionMode_SourceIn);
            pa.fillRect(image.rect(), shadow_color);
            shadow_pixmap = QPixmap::fromImage(image);
        }

        if (!shadow_pixmap.isNull()) {
            if (QMenu *menu = qobject_cast<QMenu *>(option->styleObject)) {
                if (!menu->geometry().contains(QCursor::pos()))
                    return;
            }
            painter->drawPixmap(shadow, shadow_pixmap);
        }
    }
}

void ChameleonStyle::drawMenuItemRedPoint(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    if (!(option->state & QStyle::State_Enabled))
        return;

    QAction *action = nullptr;
    if (const QMenu *menu = qobject_cast<const QMenu *>(widget)) {
        action = menu->actionAt(option->rect.center());
    }
    if (!action)
        return;

    QObject *obj = action;
    if (option->menuItemType == QStyleOptionMenuItem::SubMenu) {
        obj = action->menu();
    }

    if (!obj->property("_d_menu_item_redpoint").toBool())
        return;

    int h = 6, w = 6;  // red point size 6x6
#ifdef ENABLE_RED_POINT_TEXT
    QString text = obj->property("_d_menu_item_info").toString();
    QFont f = option->font;
    f.setPointSize(8);
    if (!text.isEmpty()) {
        QFontMetrics fontMetrics(f);
        h = fontMetrics.height();
        w = fontMetrics.horizontalAdvance(text) + Menu_ItemVTextMargin * 2;  // margin left/right 4
    }
#endif
    QPainterPath path;
    QRectF rcf(option->rect);

    rcf.setWidth(w);
    rcf.setHeight(h);
    rcf.moveTop(option->rect.top() + (option->rect.height() - h) / 2.0);  // vcenter
    rcf.moveLeft(option->rect.right() - w - 24);                          // 离右侧24像素

#ifdef ENABLE_RED_POINT_TEXT
    if (!text.isEmpty()) {
        path.addRoundRect(rcf, 50, 99);  // 0 is angled corners, 99 is maximum roundedness.
    } else
#endif
    {
        path.addEllipse(rcf);
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(path, QColor("#FF3B30"));  // color from ui.ds
    QPen oldpen = painter->pen();
    painter->setPen(QColor("#14000000"));  // 外描边，边边颜色为8%的黑 hex(255*8/100)
    painter->drawEllipse(rcf);
    painter->setPen(oldpen);

#ifdef ENABLE_RED_POINT_TEXT
    if (!text.isEmpty()) {
        painter->setPen(Qt::white);  // 文字白色
        painter->setFont(f);
        painter->drawText(QPointF(rcf.left() + 4, rcf.bottom() - 4), text.left(3));
    }
#endif
    painter->restore();
}

bool ChameleonStyle::drawMenuItem(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    if (const QStyleOptionMenuItem *menuItem = option) {
        //绘制背景
        QRect menuRect = menuItem->rect;
        bool enabled = menuItem->state & State_Enabled;
        bool selected = menuItem->state & State_Selected && enabled;
        bool checkable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
        bool checked = menuItem->checked;
        bool sunken = menuItem->state & State_Sunken;

        //绘制背景
        drawMenuItemBackground(option, painter, menuItem->menuItemType);

        //绘制分段
        if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
            if (!menuItem->text.isEmpty()) {
                painter->setFont(menuItem->font);
                painter->setPen(Qt::NoPen);
                painter->setBrush(Qt::NoBrush);
                proxy()->drawItemText(painter,
                                      menuRect,
                                      menuItem->direction == Qt::LeftToRight ? (Qt::AlignLeft | Qt::AlignVCenter) :
                                                                               (Qt::AlignRight | Qt::AlignVCenter),
                                      menuItem->palette,
                                      menuItem->state & State_Enabled,
                                      menuItem->text,
                                      QPalette::WindowText);
            }

            return true;
        }

        //绘制选择框
        bool ignoreCheckMark = false;

        int frameRadius =
            DStyle::pixelMetric(PM_FrameRadius);  //打钩矩形的左侧距离item的左边缘； 也是 打钩矩形的右侧距离 图文内容的左边缘
        int smallIconSize = proxy()->pixelMetric(PM_ButtonIconSize, option, widget);  //打钩的宽度
        int realMargins = smallIconSize + 2 * frameRadius;  //左侧固定预留的margin，无论是否能够打钩都要预留

        if (!ignoreCheckMark) {
            /*checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);*/
            QRect checkRect(menuItem->rect);

            if (checkable) {
                checkRect.setLeft(frameRadius);
                checkRect.setWidth(smallIconSize);
                checkRect.setHeight(smallIconSize);
                checkRect.moveCenter(QPoint(checkRect.left() + smallIconSize / 2, menuItem->rect.center().y()));
                painter->setRenderHint(QPainter::Antialiasing);

                if (selected)
                    painter->setPen(getColor(option, QPalette::HighlightedText));
                else
                    painter->setPen(getColor(option, QPalette::BrightText));

                if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {  //单选框
                    if (checked || sunken) {
                        QIcon markIcon = DStyle::standardIcon(SP_MarkElement, option, widget);
                        markIcon.paint(painter, checkRect);
                    }
                } else if (checked) {  //复选框
                    QIcon markIcon = DStyle::standardIcon(SP_MarkElement, option, widget);
                    markIcon.paint(painter, checkRect);
                } else {
                }
            }
        } else {  // ignore checkmark //用于combobox
        }

        if (selected) {
            painter->setPen(getColor(option, QPalette::HighlightedText));
        } else {
            if ((option->state & QStyle::State_Enabled)) {
                painter->setPen(getColor(option, QPalette::BrightText));
            } else {
                QColor color = option->palette.color(QPalette::Active, QPalette::BrightText);
                color = DStyle::adjustColor(color, 0, 0, 0, 0, 0, 0, -60);
                painter->setPen(color);
            }
        }

        QSize iconSize(0, 0);
        // 绘制图标
        if (!menuItem->icon.isNull()) {
            /*= visualRect(opt->direction, menuItem->rect,QRect(menuItem->rect.x() + checkColHOffset, menuItem->rect.y(),checkcol,
             * menuitem->rect.height()));*/
            iconSize.setWidth(smallIconSize);
            iconSize.setHeight(smallIconSize);

#if QT_CONFIG(combobox)
            if (const QComboBox *combo = qobject_cast<const QComboBox *>(widget))
                iconSize = combo->iconSize();
#endif
            //计算icon的绘制区域(icon需要居中显示)
            QRect pmr(
                menuRect.x() + realMargins, menuRect.center().y() - iconSize.height() / 2, iconSize.width(), iconSize.height());
            drawIcon(option, painter, pmr, option->icon, checked);
        }

        // 绘制文本
        int x, y, w, h;
        menuRect.getRect(&x, &y, &w, &h);
        int tab = menuItem->tabWidth;
        int xpos = menuRect.x();  // 1.只有文本  2.只有图片加文本  ，xpos为文本的起始坐标

        if (iconSize.width() > 0) {
            xpos += realMargins + frameRadius + iconSize.width();
        } else {
            xpos += realMargins;
        }

        QRect textRect(xpos, y + Menu_ItemHTextMargin, w - xpos - tab, h - 2 * Menu_ItemVTextMargin);
        QRect vTextRect = textRect /*visualRect(option->direction, menuRect, textRect)*/;  // 区分左右方向
        QStringRef textRef(&menuItem->text);

        painter->setBrush(Qt::NoBrush);

        if (!textRef.isEmpty()) {
            int tabIndex = textRef.indexOf(QLatin1Char('\t'));
            int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;

            if (!styleHint(SH_UnderlineShortcut, menuItem, widget))
                text_flags |= Qt::TextHideMnemonic;

            text_flags |= Qt::AlignLeft;

            if (tabIndex >= 0) {
                QPoint vShortcutStartPoint = textRect.topRight();  //快捷键设置显示
                vShortcutStartPoint.setX(vShortcutStartPoint.x() - Menu_PanelRightPadding - realMargins);
                QRect vShortcutRect = QRect(vShortcutStartPoint, QPoint(menuRect.right(), textRect.bottom()));
                /* = visualRect(option->direction,menuRect,QRect(vShortcutStartPoint, QPoint(menuRect.right(),
                 * textRect.bottom())))*/
                ;
                const QString textToDraw = textRef.mid(tabIndex + 1).toString();

                painter->drawText(vShortcutRect, text_flags, textToDraw);
                textRef = textRef.left(tabIndex);
            }

            QFont font = menuItem->font;
            font.setPointSizeF(QFontInfo(menuItem->font).pointSizeF());
            painter->setFont(font);
            const QString textToDraw = textRef.left(tabIndex).toString();

            painter->setBrush(Qt::NoBrush);
            painter->drawText(vTextRect, text_flags, textToDraw);

            drawMenuItemRedPoint(option, painter, widget);
        }

        // 绘制箭头
        if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {  // draw sub menu arrow
            int dim = (menuRect.height() - 4) / 2;
            int xpos = menuRect.left() + menuRect.width() - 3 - dim;
            QStyleOptionMenuItem newMI = *menuItem;
            xpos += realMargins + iconSize.width() + frameRadius;
            QPoint topLeft(menuItem->rect.right() - frameRadius - smallIconSize / 2,
                           menuItem->rect.center().y() - smallIconSize / 3);  //箭头rect: Size(smallIconSize, smallIconSize*2/3)
            QPoint RightButtom(topLeft.x() + smallIconSize / 2, menuItem->rect.center().y() + smallIconSize / 3);
            QRect rectArrow(topLeft, RightButtom);
            newMI.rect = rectArrow;

            newMI.state = !enabled ? State_None : State_Enabled;
            if (selected)
                newMI.palette.setColor(QPalette::WindowText, newMI.palette.highlightedText().color());

            QIcon markIcon = DStyle::standardIcon(SP_ArrowEnter, &newMI, widget);
            markIcon.paint(painter, newMI.rect);
        }
    }

    return true;
}




bool ChameleonStyle::drawSpinBox(const QStyleOptionSpinBox *opt, QPainter *painter, const QWidget *widget) const
{
    qreal borderRadius = DStyle::pixelMetric(DStyle::PM_FrameRadius);

    if (opt->frame && (opt->subControls & SC_SpinBoxFrame)) {
        QRect frameRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxFrame, widget);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(opt->palette.button());
        painter->drawRoundedRect(frameRect, borderRadius, borderRadius);
    }

    if (opt->subControls & SC_SpinBoxUp) {
        bool upIsActive = opt->activeSubControls == SC_SpinBoxUp;
        bool upIsEnabled = opt->stepEnabled & QAbstractSpinBox::StepUpEnabled && opt->state.testFlag(State_Enabled);
        QRect subRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, widget);
        QStyleOptionButton buttonOpt;
        buttonOpt.rect = subRect;
        buttonOpt.palette = opt->palette;
        updateSpinBoxButtonState(opt, buttonOpt, upIsActive, upIsEnabled);
        bool isPlusMinus = opt->buttonSymbols & QAbstractSpinBox::PlusMinus;

        if (widget && widget->property("_d_dtk_spinBox").toBool()) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(buttonOpt.palette.color(QPalette::Button));
            DDrawUtils::drawRoundedRect(
                painter, subRect.adjusted(-2, -1, 0, 0), borderRadius, borderRadius, DDrawUtils::TopRightCorner);

            painter->setPen(getColor(opt, DPalette::FrameBorder, widget));
            painter->drawLine(subRect.x() - 2, subRect.y() - 1, subRect.x() - 2, subRect.y() - 1 + subRect.height());
        } else {
            proxy()->drawControl(CE_PushButton, &buttonOpt, painter, widget);
        }

        if (isPlusMinus) {
            //绘制 "+" 符号
            buttonOpt.rect -= frameExtentMargins();
            QRectF plusRect = proxy()->subElementRect(SE_PushButtonContents, &buttonOpt, widget);
            QRectF cent_rect(0, 0, plusRect.width() / 2, plusRect.width() / 2);
            cent_rect.moveCenter(plusRect.center());

            if (opt->activeSubControls == SC_SpinBoxUp) {
                DDrawUtils::drawPlus(painter, cent_rect, getColor(opt, QPalette::ButtonText), 1);
            } else {
                DDrawUtils::drawPlus(painter,
                                     cent_rect,
                                     DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ?
                                         QColor("#536076") :
                                         QColor("#c5cfe0"),
                                     1);
            }
        } else {
            QStyleOptionButton arrowDrawBtn = buttonOpt;

            if (widget && widget->property("_d_dtk_spinBox").toBool()) {
                if (opt->activeSubControls == SC_SpinBoxDown) {
                    painter->setPen(buttonOpt.palette.color(QPalette::ButtonText));
                } else {
                    painter->setPen(getColor(opt, QPalette::ButtonText));
                }

                arrowDrawBtn.rect = subRect;
                arrowDrawBtn.rect.setWidth(subRect.width() / 2);
                arrowDrawBtn.rect.setHeight(subRect.height() / 1.3);
                arrowDrawBtn.rect.moveCenter(subRect.center());
                proxy()->drawPrimitive(PE_IndicatorArrowUp, &arrowDrawBtn, painter, widget);

            } else {
                arrowDrawBtn.rect = subRect;
                arrowDrawBtn.rect.setSize(subRect.size() / 3);
                arrowDrawBtn.rect.moveCenter(subRect.center());
                proxy()->drawPrimitive(PE_IndicatorArrowUp, &arrowDrawBtn, painter, widget);
            }
        }
    }

    if (opt->subControls & SC_SpinBoxDown) {
        bool downIsActive = opt->activeSubControls == SC_SpinBoxDown;
        bool downIsEnabled = opt->stepEnabled & QAbstractSpinBox::StepDownEnabled && opt->state.testFlag(State_Enabled);
        QRect subRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, widget);
        QStyleOptionButton buttonOpt;
        buttonOpt.rect = subRect;
        buttonOpt.palette = opt->palette;
        updateSpinBoxButtonState(opt, buttonOpt, downIsActive, downIsEnabled);
        bool isPlusMinus = opt->buttonSymbols & QAbstractSpinBox::PlusMinus;

        if (widget && widget->property("_d_dtk_spinBox").toBool()) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(buttonOpt.palette.color(QPalette::Button));
            DDrawUtils::drawRoundedRect(
                painter, subRect.adjusted(-2, 0, 0, +1), borderRadius, borderRadius, DDrawUtils::BottomRightCorner);
            painter->setPen(getColor(opt, DPalette::FrameBorder, widget));
            painter->drawLine(subRect.x() - 2, subRect.y(), subRect.x() + subRect.width(), subRect.y());
            painter->drawLine(subRect.x() - 2, subRect.y(), subRect.x() - 2, subRect.y() + subRect.height() + 1);
        } else {
            proxy()->drawControl(CE_PushButton, &buttonOpt, painter, widget);
        }

        if (isPlusMinus) {
            buttonOpt.rect -= frameExtentMargins();
            QRectF subtractRect = proxy()->subElementRect(SE_PushButtonContents, &buttonOpt, widget);
            QRectF cent_rect(0, 0, subtractRect.width() / 2, subtractRect.height() / 2);
            cent_rect.moveCenter(subtractRect.center());

            if (opt->activeSubControls == SC_SpinBoxDown) {
                DDrawUtils::drawSubtract(painter, cent_rect, getColor(opt, QPalette::ButtonText), 1);
            } else {
                DDrawUtils::drawSubtract(painter,
                                         cent_rect,
                                         DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ?
                                             QColor("#536076") :
                                             QColor("#c5cfe0"),
                                         1);
            }
        } else {
            QStyleOptionButton arrowDrawBtn = buttonOpt;

            if (widget && widget->property("_d_dtk_spinBox").toBool()) {
                if (opt->activeSubControls == SC_SpinBoxDown) {
                    painter->setPen(getColor(opt, QPalette::ButtonText));
                } else {
                    painter->setPen(buttonOpt.palette.color(QPalette::ButtonText));
                }

                arrowDrawBtn.rect = subRect;
                arrowDrawBtn.rect.setWidth(subRect.width() / 2);
                arrowDrawBtn.rect.setHeight(subRect.height() / 1.3);
                arrowDrawBtn.rect.moveCenter(subRect.center());
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowDrawBtn, painter, widget);
            } else {
                //更换了一种绘制箭头方式，使36 *36情况下箭头无瑕疵
                arrowDrawBtn.rect = subRect;
                arrowDrawBtn.rect.setSize(subRect.size() / 3);
                arrowDrawBtn.rect.moveCenter(subRect.center());
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowDrawBtn, painter, widget);
            }
        }
    }

    return true;
}

void ChameleonStyle::updateSpinBoxButtonState(const QStyleOptionSpinBox *opt,
                                              QStyleOptionButton &buttonOpt,
                                              bool isActive,
                                              bool isEnabled) const
{
    State buttonState = opt->state;

    if (!isActive) {
        buttonState &= ~State_MouseOver;
        buttonState &= ~State_Sunken;
        buttonState &= ~State_Active;
        buttonState &= ~State_HasFocus;
    }

    if (!isEnabled) {
        buttonState &= ~State_Enabled;
        buttonState &= ~State_MouseOver;
        buttonState &= ~State_Sunken;
        buttonState &= ~State_On;
    }

    if (buttonState & State_Sunken || buttonState & State_MouseOver || buttonState & State_Active) {
        buttonState &= ~State_HasFocus;
    }

    buttonOpt.state = buttonState;
}

void ChameleonStyle::resetAttribute(QWidget *w, bool polish)
{
    if (!w)
        return;

    bool enableHover = w->testAttribute(Qt::WA_Hover);
    if (qobject_cast<QAbstractButton *>(w) || qobject_cast<QComboBox *>(w) || qobject_cast<QScrollBar *>(w) ||
        qobject_cast<QCheckBox *>(w) || qobject_cast<QAbstractSpinBox *>(w) || qobject_cast<QTabBar *>(w) ||
        qobject_cast<QCheckBox *>(w)) {
        enableHover = polish;
    }

    if (auto view = qobject_cast<QAbstractItemView *>(w)) {
        enableHover = polish;
        w = view->viewport();
    }

    // TODO: 是平板环境统一设置所有的控件的 WA_Hover 为 false，
    // 不过在插入鼠标时还是有问题，现阶段先不考虑，以后 Qt 如果优化了如有更好的方案这里再改掉。
    if (DGuiApplicationHelper::isTabletEnvironment())
        enableHover = false;

    w->setAttribute(Qt::WA_Hover, enableHover);

    if (auto scrollbar = qobject_cast<QScrollBar *>(w)) {
        if (polish) {
            scrollbar->installEventFilter(this);
        } else {
            scrollbar->removeEventFilter(this);
        }
        scrollbar->setAttribute(Qt::WA_OpaquePaintEvent, !polish);
    }
}





void ChameleonStyle::drawShadow(QPainter *p, const QRect &rect, const QColor &color) const
{
    int frame_radius = DStyle::pixelMetric(PM_FrameRadius);
    int shadow_xoffset = DStyle::pixelMetric(PM_ShadowHOffset);
    int shadow_yoffset = DStyle::pixelMetric(PM_ShadowVOffset);

    QRect shadow = rect;
    QPoint pointOffset(rect.center().x() + shadow_xoffset, rect.center().y() + shadow_yoffset);
    shadow.moveCenter(pointOffset);

    p->setBrush(color);
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing);
    p->drawRoundedRect(shadow, frame_radius, frame_radius);
}

void ChameleonStyle::drawBorder(QPainter *p, const QStyleOption *opt, const QWidget *w) const
{
    const QColor &focus_color = getColor(opt, QPalette::Highlight);

    if (!focus_color.isValid() || focus_color.alpha() == 0)
        return;

    int frame_radius = DStyle::pixelMetric(PM_FrameRadius);
    int margins = DStyle::pixelMetric(PM_FrameMargins);

    QRect border = opt->rect.adjusted(margins, margins, -margins, -margins);
    p->setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(focus_color);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    if (w && w->property("_d_dtk_tabbartab_type").toBool()) {
        p->drawRect(opt->rect.adjusted(-1, 1, 1, -1));
        return;
    }

    if (w && w->parentWidget() && w->parentWidget()->property("_d_dtk_spinBox").toBool()) {
        DDrawUtils::drawRoundedRect(p,
                                    opt->rect.adjusted(1, 1, -1, -1),
                                    frame_radius,
                                    frame_radius,
                                    DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
        return;
    }

    bool table = qobject_cast<const QTableView *>(w) && !w->property("_d_dtk_enable_tableviewitem_radius").toBool();
    // QCalendarWidget的QTableView焦点状态与QTableView不同
    bool calendar = w && (w->objectName() == "qt_calendar_calendarview");
    // DButtonBoxButton在不同位置焦点不同
    const DButtonBoxButton *buttonBoxButton = qobject_cast<const DButtonBoxButton *>(w);
    const DStyleOptionButtonBoxButton *btnopt = qstyleoption_cast<const DStyleOptionButtonBoxButton *>(opt);

    pen.setWidth(2);
    int offset = 1;
    QRect rect = border.adjusted(offset, offset, -offset, -offset);

    // 先画内框（黑or白）
    pen.setColor(getColor(opt, QPalette::Base));
    p->setPen(pen);
    if (calendar) {
        pen.setWidth(3);
        p->setPen(pen);
        offset = 2;
        drawCalenderEllipse(p, opt->rect, offset);
    } else if (table) {
        p->drawRect(rect);
    } else if (buttonBoxButton && btnopt) {
        drawButtonBoxButton(btnopt, p, rect, frame_radius);
    } else {
        p->drawRoundedRect(rect, frame_radius, frame_radius);
    }

    // 再画外框（活动色即焦点）
    pen.setColor(focus_color);
    p->setPen(pen);
    if (calendar) {
        pen.setWidth(2);
        p->setPen(pen);
        offset = 1;
        drawCalenderEllipse(p, opt->rect, offset);
    } else if (table) {
        p->drawRect(border);
    } else if (buttonBoxButton && btnopt) {
        drawButtonBoxButton(btnopt, p, border, frame_radius);
    } else {
        p->drawRoundedRect(border, frame_radius + margins, frame_radius + margins);
    }
}

void ChameleonStyle::drawCalenderEllipse(QPainter *p, const QRect &rect, int offset) const
{
    QRect ellipese = rect;
    ellipese.setWidth(ellipese.height());
    ellipese.moveCenter(rect.center());
    p->drawEllipse(ellipese.adjusted(offset, offset, -offset, -offset));
}

void ChameleonStyle::drawButtonBoxButton(const DStyleOptionButtonBoxButton *btnopt,
                                         QPainter *p,
                                         const QRect &rect,
                                         int radius) const
{
    if (btnopt->position == DStyleOptionButtonBoxButton::Beginning) {
        // Begin
        if (btnopt->orientation == Qt::Horizontal) {
            DDrawUtils::drawRoundedRect(p, rect, radius, radius, DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
        } else {
            DDrawUtils::drawRoundedRect(p, rect, radius, radius, DDrawUtils::TopLeftCorner | DDrawUtils::TopRightCorner);
        }
    } else if (btnopt->position == DStyleOptionButtonBoxButton::End) {
        // End
        if (btnopt->orientation == Qt::Horizontal) {
            DDrawUtils::drawRoundedRect(p, rect, radius, radius, DDrawUtils::TopRightCorner | DDrawUtils::BottomRightCorner);
        } else {
            DDrawUtils::drawRoundedRect(p, rect, radius, radius, DDrawUtils::BottomLeftCorner | DDrawUtils::BottomRightCorner);
        }
    } else if (btnopt->position == DStyleOptionButtonBoxButton::Middle) {
        // Middle
        p->drawRect(rect);
    } else if (btnopt->position == DStyleOptionButtonBoxButton::OnlyOne) {
        // OnlyOne
        p->drawRoundedRect(rect, radius, radius);
    }
}

bool ChameleonStyle::isNoticks(const QStyleOptionSlider *slider, QPainter *p, const QWidget *w) const
{
    Q_UNUSED(p)
    if (const DSlider *dslider = qobject_cast<const DSlider *>(w)) {
        QSlider::TickPosition tickPosition = slider->tickPosition;

        if (dslider)
            tickPosition = dslider->tickPosition();

        return tickPosition == QSlider::NoTicks;
    }
    return false;
}


QColor ChameleonStyle::getColor(const QStyleOption *option, QPalette::ColorRole role) const
{
    return DStyle::generatedBrush(option, option->palette.brush(role), option->palette.currentColorGroup(), role).color();
}

QColor ChameleonStyle::getColor(const QStyleOption *option, DPalette::ColorType type, const QWidget *widget) const
{
    const DPalette &pa = DPaletteHelper::instance()->palette(widget, option->palette);

    return DStyle::generatedBrush(option, pa.brush(type), pa.currentColorGroup(), type).color();
}

QBrush ChameleonStyle::getBrush(const QStyleOption *option, DPalette::ColorRole type) const
{
    QWidget *widget = qobject_cast<QWidget *>(option->styleObject);
    if (widget && !widget->testAttribute(Qt::WA_Hover) && DGuiApplicationHelper::isTabletEnvironment()) {
        return QBrush(Qt::NoBrush);
    }
    return QBrush(getColor(option, type));
}

QMargins ChameleonStyle::frameExtentMargins() const
{
    int margins = DStyle::pixelMetric(PM_FrameMargins);

    return QMargins(margins, margins, margins, margins);
}

QRect ChameleonStyle::drawButtonDownArrow(const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
	//getThemTypeColor(QColor lightColor, QColor darkColor);
    const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt);

    if (!btn)
        return QRect(-1, -1, -1, -1);

    QRect rectOpt = btn->rect;  //实际绘画箭头所占的小矩形
    int arrowWidth = DStyle::pixelMetric(PM_MenuButtonIndicator, opt, w);
    int arrowHeight = arrowWidth;
    QRect rectArrow(0, 0, arrowWidth, arrowHeight);
    rectArrow.moveCenter(rectOpt.center());

    QStyleOptionButton newBtn = *btn;  //绘画箭头的大矩形(不要竖线)
    QRect &newRect = newBtn.rect;
    newRect.setHeight(rectOpt.height());
    newRect.setWidth(arrowWidth);
    newRect.moveCenter(rectOpt.center());

    if (btn->direction == Qt::LeftToRight) {
        rectArrow.moveRight(rectOpt.right());
        newRect.moveRight(rectOpt.right());
    } else {
        rectArrow.moveLeft(rectOpt.left());
        newRect.moveLeft(rectOpt.left());
    }

    if (p == nullptr || w == nullptr)
        return newRect;

    QStyleOptionButton arrowDrawBtn = newBtn;
    arrowDrawBtn.rect = rectArrow;
    proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowDrawBtn, p, w);

    return newRect;
}

}  // namespace chameleon
