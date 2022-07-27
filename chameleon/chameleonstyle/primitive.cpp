#include "chameleonstyle.h"
#include "common.h"
#include "chameleontools.hpp"

#include <DApplicationHelper>
#include <DButtonBox>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>
#include <QComboBox>
#include <QLineEdit>
#include <DDateTimeEdit>
#include <DListView>
#include <DSpinBox>
#include <DTabBar>
#include <DTreeView>
#include <QAbstractItemView>
#include <QtGlobal>
#include <QtMath>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {
void ChameleonStyle::drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch (static_cast<int>(pe)) {
        case PE_PanelButtonCommand: {
            //        qDebug() << "### pushbutton state " << (int)opt->state;
            const QMargins &margins = frameExtentMargins();

            // checked
            if (opt->state & State_On) {
                p->setBrush(getColor(opt, QPalette::Highlight));
            } else {
                drawShadow(p, opt->rect - margins, getColor(opt, QPalette::Shadow));
                // 初始化button的渐变背景色
                QLinearGradient lg(QPointF(0, opt->rect.top()), QPointF(0, opt->rect.bottom()));
                lg.setColorAt(0, getColor(opt, QPalette::Light));
                lg.setColorAt(1, getColor(opt, QPalette::Dark));

                p->setBrush(lg);
            }

            p->setPen(Qt::NoPen);
            p->setRenderHint(QPainter::Antialiasing);
            int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
            p->drawRoundedRect(opt->rect - margins, frame_radius, frame_radius);

            // draw border，border应该是完全叠加到按钮的背景上
            p->setPen(QPen(getColor(opt, DPalette::FrameBorder, w), Metrics::Painter_PenWidth));
            p->setBrush(Qt::NoBrush);
            const QMarginsF border_margins(
                Metrics::Painter_PenWidth, Metrics::Painter_PenWidth, Metrics::Painter_PenWidth, Metrics::Painter_PenWidth);
            p->drawRoundedRect(QRectF(opt->rect) - margins - border_margins / 2.0, frame_radius, frame_radius);

            return;
        }
        case PE_FrameFocusRect: {
            //设计要求QTreeView选中整行，这里不绘制focus rect
            if (qobject_cast<const QTreeView *>(w)) {
                return;
            }

            // QListView不需要绘制focus rect
            if (qobject_cast<const QListView *>(w)) {
                return;
            }

            if (w && w->property("_d_dtk_noFocusRect").toBool())
                return;

            //设计要求DDateTimeEdit focus只绘制边缘
            if (qobject_cast<const QLineEdit *>(w) && qobject_cast<const DDateTimeEdit *>(w->parentWidget())) {
                return;
            }

            drawBorder(p, opt, w);
            return;
        }
        case PE_PanelItemViewItem: {
            if (w && w->objectName() == "qt_calendar_calendarview") {
                if (opt->state & QStyle::State_Selected) {
                    QRect textRect = opt->rect;
                    textRect.setWidth(textRect.height());
                    p->setBrush(opt->palette.brush(QPalette::Highlight));
                    textRect.moveCenter(opt->rect.center());
                    p->setPen(Qt::NoPen);
                    p->setRenderHint(QPainter::Antialiasing);
                    p->drawEllipse(textRect.adjusted(1, 1, -1, -1));
                }
                return;
            }

            // QTreeView的绘制复制了QCommonStyle的代码，添加了圆角的处理,hover的处理
            if (qobject_cast<const QTreeView *>(w)) {
                //如果QTreeView使用的不是默认代理
                // QStyledItemDelegate,则采取DStyle的默认绘制(备注:这里的QtCreator不会有hover效果和圆角)
                if (typeid(qobject_cast<const QTreeView *>(w)->itemDelegate()) != typeid(QStyledItemDelegate)) {
                    break;
                }

                if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                    QPalette::ColorGroup cg =
                        (w ? w->isEnabled() : (vopt->state & QStyle::State_Enabled)) ? QPalette::Normal : QPalette::Disabled;
                    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                        cg = QPalette::Inactive;

                    int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);

                    if (vopt->showDecorationSelected && (vopt->state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
                        p->setRenderHint(QPainter::Antialiasing, true);
                        p->setPen(Qt::NoPen);
                        p->setBrush(vopt->palette.brush(
                            cg, (vopt->state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Midlight));

                        if ((vopt->state & QStyle::State_Selected) && (vopt->state & QStyle::State_MouseOver)) {
                            p->setBrush(p->brush().color().lighter(120));
                        }

                        //只对最后一列的item绘制圆角
                        if (vopt->viewItemPosition == QStyleOptionViewItem::End ||
                            vopt->viewItemPosition == QStyleOptionViewItem::OnlyOne) {
                            p->drawRoundedRect(vopt->rect.adjusted(-frame_radius, 0, 0, 0), frame_radius, frame_radius);
                        } else if (vopt->viewItemPosition != QStyleOptionViewItem::Invalid) {
                            p->drawRoundedRect(
                                vopt->rect.adjusted(-frame_radius, 0, frame_radius, 0), frame_radius, frame_radius);
                        }
                    } else {
                        if (vopt->backgroundBrush.style() != Qt::NoBrush) {
                            QPointF oldBO = p->brushOrigin();
                            p->setBrushOrigin(vopt->rect.topLeft());
                            p->fillRect(vopt->rect, vopt->backgroundBrush);
                            p->setBrushOrigin(oldBO);
                        }

                        if (vopt->state & QStyle::State_Selected) {
                            QRect textRect = subElementRect(QStyle::SE_ItemViewItemText, opt, w);
                            p->fillRect(textRect, vopt->palette.brush(cg, QPalette::Highlight));
                        }
                    }
                }
                return;
            }

            if (drawTableViewItem(pe, opt, p, w))
                return;

            if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);

                if (vopt->state & QStyle::State_Selected) {
                    QRect select_rect = opt->rect;

                    if (!vopt->showDecorationSelected) {
                        select_rect = proxy()->subElementRect(QStyle::SE_ItemViewItemText, opt, w);
                    } else {
                        select_rect -= frameExtentMargins();
                    }

                    p->setPen(Qt::NoPen);
                    p->setBrush(getColor(opt, QPalette::Highlight));
                    p->setRenderHint(QPainter::Antialiasing);
                    p->drawRoundedRect(select_rect, frame_radius, frame_radius);
                    return;
                }
            }
            break;
        }
        case PE_PanelLineEdit: {
            if (w && w->parentWidget()) {
                // lineEdit作为子控件时不进行绘制
                if (qobject_cast<const QComboBox *>(w->parent()))
                    return;

                if (auto edit = qobject_cast<const QDateTimeEdit *>(w->parentWidget())) {
                    if (edit->calendarPopup())
                        return;
                }
            }

            if (auto fopt = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
                // Flat时不绘制输入框的背景
                if (fopt->features == QStyleOptionFrame::Flat) {
                    if (opt->state.testFlag(QStyle::State_HasFocus)) {
                        proxy()->drawPrimitive(PE_FrameFocusRect, opt, p, w);
                    }
                    return;
                }

                if (fopt->lineWidth > 0) {
                    proxy()->drawPrimitive(PE_FrameLineEdit, fopt, p, w);
                }
            }

            // 此处设置painter的行为不应该影响外面的绘制，会导致lineedit光标不对 Bug-20967。
            p->save();
            if (w && w->property("_d_dtk_lineedit_opacity").toBool())
                p->setBrush(getThemTypeColor(QColor(0, 0, 0, 255 * 0.08), QColor(255, 255, 255, 255 * 0.15)));
            else
                p->setBrush(opt->palette.button());
            p->setPen(Qt::NoPen);
            p->setRenderHints(QPainter::Antialiasing);
            int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);

            if ((w && qobject_cast<QAbstractSpinBox *>(w->parentWidget()))) {
                if (w->parentWidget()->property("_d_dtk_spinBox").toBool()) {
                    DDrawUtils::drawRoundedRect(
                        p, opt->rect, frame_radius, frame_radius, DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
                } else {
                    p->drawRoundedRect(opt->rect - frameExtentMargins(), frame_radius, frame_radius);
                }
            } else {
                p->drawRoundedRect(opt->rect, frame_radius, frame_radius);
            }

            if (opt->state.testFlag(QStyle::State_HasFocus)) {
                proxy()->drawPrimitive(PE_FrameFocusRect, opt, p, w);
            }

            p->restore();
            return;
        }
        case PE_FrameLineEdit: {
            // lineedit no frame
            return;
        }
        case PE_IndicatorRadioButton: {
            QRect standard = opt->rect;

            p->setRenderHint(QPainter::Antialiasing, true);

            if (opt->state & State_On) {  // Qt::Checked
                int padding = qCeil(standard.width() / 2.0 / 2.0);
                QPainterPath path;

                path.addEllipse(standard);
                path.addEllipse(standard.adjusted(padding, padding, -padding, -padding));

                p->fillPath(path, getColor(opt, DPalette::Highlight));

                // 内圈填充
                QPainterPath innerCirclePath;
                innerCirclePath.addEllipse(standard.adjusted(padding, padding, -padding, -padding));
                p->fillPath(innerCirclePath, getThemTypeColor(Qt::white, Qt::black));
            } else if (opt->state & State_Off) {
                p->setPen(QPen(getColor(opt, DPalette::WindowText), 1));
                p->drawEllipse(standard.adjusted(1, 1, -1, -1));

                // 内圈填充
                QPainterPath innerCirclePath;
                innerCirclePath.addEllipse(standard.adjusted(1, 1, -1, -1));
                p->fillPath(innerCirclePath, getThemTypeColor(Qt::transparent, QColor(0, 0, 0, qCeil(255 * 0.5))));
            }

            return;
        }
        case PE_IndicatorCheckBox: {
            QRectF standard = opt->rect;

            if (opt->state & State_NoChange) {  // Qt::PartiallyChecked
                DDrawUtils::drawBorder(p, standard, getColor(opt, DPalette::WindowText), 1, 2);

                // 内部矩形填充
                p->setBrush(getThemTypeColor(Qt::transparent, QColor(0, 0, 0, qCeil(255 * 0.5))));
                p->drawRoundedRect(standard.adjusted(1, 1, -1, -1), 2, 2);

                QRectF lineRect(0, 0, standard.width() / 2.0, 2);
                lineRect.moveCenter(standard.center());
                p->fillRect(lineRect, getColor(opt, DPalette::TextTitle, w));
            } else if (opt->state & State_On) {  // Qt::Checked
                // 填充整个矩形
                p->setPen(Qt::NoPen);
                p->setBrush(getThemTypeColor(Qt::transparent, Qt::black));
                p->drawRoundedRect(standard.adjusted(1, 1, -1, -1), 2, 2);

                p->setPen(getColor(opt, DPalette::Highlight));
                p->setBrush(Qt::NoBrush);

                QIcon icon = QIcon::fromTheme("checked");
                icon.paint(p, opt->rect.adjusted(-1, -1, 1, 1));
            } else {
                DDrawUtils::drawBorder(p, standard, getColor(opt, DPalette::WindowText), 1, 2);

                // 内部矩形填充
                p->setBrush(
                    getThemTypeColor(Qt::transparent, getThemTypeColor(Qt::transparent, QColor(0, 0, 0, qCeil(255 * 0.5)))));
                p->drawRoundedRect(standard.adjusted(1, 1, -1, -1), 2, 2);
            }

            return;
        }
        case PE_IndicatorTabClose: {
            QIcon icon = DStyle::standardIcon(SP_CloseButton, opt, w);
            if (DGuiApplicationHelper::isTabletEnvironment()) {
                // 在平板中点击区域为36px 显示区域22px
                QRect iconRect(0, 0, TabBar_TabButtonSize, TabBar_TabButtonSize);
                iconRect.moveCenter(opt->rect.center());
                icon.paint(p, iconRect);
                return;
            }
            icon.paint(p, opt->rect);
            return;
        }
        case PE_FrameTabWidget: {
            p->setPen(QPen(getColor(opt, QPalette::Dark), proxy()->pixelMetric(PM_DefaultFrameWidth, opt, w)));
            p->setBrush(getColor(opt, QPalette::Window));
            p->drawRect(opt->rect);
            return;
        }
        case PE_IndicatorItemViewItemCheck: {
            QRectF standard = opt->rect;
            p->setRenderHint(QPainter::Antialiasing, true);
            QIcon::Mode mode = opt->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;

            if (opt->state & State_NoChange) {  // Qt::PartiallyChecked
                DStyle::standardIcon(SP_IndicatorChecked, opt, w).paint(p, standard.toRect(), Qt::AlignCenter, mode);
            } else if (opt->state & State_On) {  // Qt::Checked
                p->setPen(getColor(opt, DPalette::Highlight));
                DStyle::standardIcon(SP_IndicatorChecked, opt, w).paint(p, standard.toRect(), Qt::AlignCenter, mode);
            } else if (opt->state & State_Off) {  // Qt::Unchecked
                if (w && w->property("_d_dtk_UncheckedItemIndicator").toBool()) {
                    DStyle::standardIcon(SP_IndicatorUnchecked, opt, w).paint(p, standard.toRect(), Qt::AlignCenter, mode);
                }
                if (w && !w->property("_d_dtk_UncheckedItemIndicator").isValid() &&
                    !qobject_cast<const Dtk::Widget::DListView *>(w)) {
                    DStyle::standardIcon(SP_IndicatorUnchecked, opt, w).paint(p, standard.toRect(), Qt::AlignCenter, mode);
                }
            }
            return;
        }
        case PE_PanelMenu: {
            if (opt->palette.window().color().isValid() && DWindowManagerHelper::instance()->hasBlurWindow()) {
                QColor color = opt->palette.window().color();
                color.setAlphaF(0.3);
                p->fillRect(opt->rect, color);
            } else {
                p->fillRect(opt->rect, opt->palette.window());
            }

            break;
        }
        case PE_Frame: {
            if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
                if (f->features & QStyleOptionFrame::Rounded) {
                    p->setRenderHint(QPainter::Antialiasing);
                    p->setBrush(p->background());

                    if (f->lineWidth <= 0) {
                        p->setPen(Qt::NoPen);
                    }

                    int radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
                    QRectF rect(opt->rect);
                    rect.adjust(f->lineWidth / 2.0, f->lineWidth / 2.0, -f->lineWidth / 2.0, -f->lineWidth / 2.0);
                    p->drawRoundedRect(rect, radius, radius);
                    return;
                }
            }
            break;
        }
        case PE_PanelTipLabel: {
            return;
        }
        case PE_FrameGroupBox: {
            if (auto group_opt = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
                if (group_opt->features & QStyleOptionFrame::Flat)
                    return;
            }
            DStyleOptionBackgroundGroup option;
            option.init(w);
            option.position = DStyleOptionBackgroundGroup::OnlyOne;
            static_cast<QStyleOption *>(&option)->operator=(*opt);
            DStyle::drawPrimitive(PE_ItemBackground, &option, p, w);
            return;
        }
        case PE_IndicatorArrowUp: {
            QIcon icon = DStyle::standardIcon(SP_ArrowUp, opt, w);
            icon.paint(p, opt->rect);
            return;
        }
        case PE_IndicatorArrowDown: {
            QIcon icon = DStyle::standardIcon(SP_ArrowDown, opt, w);
            QRect r = opt->rect;
            int size = qMin(r.width(), r.height());
            int xOffset = r.x() + (r.width() - size) / 2;
            int yOffset = r.y() + (r.height() - size) / 2;
            icon.paint(p, QRect(xOffset, yOffset, size, size));
            return;
        }
        case PE_IndicatorArrowRight: {
            QIcon icon = DStyle::standardIcon(SP_ArrowRight, opt, w);
            icon.paint(p, opt->rect);
            return;
        }
        case PE_IndicatorArrowLeft: {
            QIcon icon = DStyle::standardIcon(SP_ArrowLeft, opt, w);
            icon.paint(p, opt->rect);
            return;
        }
        case PE_FrameTabBarBase:
            return;
        case PE_IndicatorBranch: {
            // tree
            QRect rect = DStyle::subElementRect(SE_HeaderArrow, opt, w);
            int rect_width = rect.width();
            int rect_height = rect.height();
            rect.setWidth(rect_width > rect_height ? rect_width : rect_height);
            rect.setHeight(rect_width > rect_height ? rect_width : rect_height);
            rect.moveCenter(opt->rect.center());

            if (opt->state & State_Children) {
                if (!(opt->state & State_Open)) {
                    p->save();

                    //在选中时进行反白处理
                    if (opt->state & State_Selected) {
                        p->setPen(QColor(Qt::white));
                    }

                    DStyle::standardIcon(SP_ArrowRight, opt, w).paint(p, rect);
                    p->restore();
                    return;
                }

                p->save();

                //在选中时进行反白处理
                if (opt->state & State_Selected) {
                    p->setPen(QColor(Qt::white));
                }

                DStyle::standardIcon(SP_ArrowDown, opt, w).paint(p, rect);
                p->restore();
            }
            return;
        }
        case PE_PanelItemViewRow: {
            // 不绘制选中的item背景，已经绘制了一个圆了
            if (w && qobject_cast<QCalendarWidget *>(w->parentWidget())) {
                return;
            }
            //这里QTreeView的绘制复制了QCommonStyle的代码，添加了圆角的处理,hover的处理
            if (qobject_cast<const QTreeView *>(w)) {
                //如果QTreeView使用的不是默认代理
                // QStyledItemDelegate,则采取DStyle的默认绘制(备注:这里的QtCreator不会有hover效果和圆角)
                if (typeid(qobject_cast<const QTreeView *>(w)->itemDelegate()) != typeid(QStyledItemDelegate)) {
                    break;
                }

                if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                    QPalette::ColorGroup cg =
                        (w ? w->isEnabled() : (vopt->state & QStyle::State_Enabled)) ? QPalette::Normal : QPalette::Disabled;
                    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                        cg = QPalette::Inactive;

                    int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);

                    if ((vopt->state & (QStyle::State_Selected | QStyle::State_MouseOver)) &&
                        proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, opt, w)) {
                        p->setRenderHint(QPainter::Antialiasing, true);
                        p->setPen(Qt::NoPen);
                        p->setBrush(vopt->palette.brush(
                            cg, (vopt->state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Midlight));

                        if ((vopt->state & QStyle::State_Selected) && (vopt->state & QStyle::State_MouseOver)) {
                            p->setBrush(p->brush().color().lighter(120));
                        }

                        if (vopt->viewItemPosition != QStyleOptionViewItem::End) {
                            p->drawRoundedRect(vopt->rect.adjusted(0, 0, frame_radius, 0), frame_radius, frame_radius);
                        } else if (vopt->viewItemPosition != QStyleOptionViewItem::Invalid) {
                            p->drawRoundedRect(vopt->rect, frame_radius, frame_radius);
                        }
                    } else if (vopt->features & QStyleOptionViewItem::Alternate) {
                        p->fillRect(vopt->rect, vopt->palette.brush(cg, QPalette::AlternateBase));
                    }
                }
                return;
            }

            break;
        }
        case PE_FrameStatusBarItem: {
            return;
        }
        case PE_PanelStatusBar: {
            QColor bgcolor;
            QColor lineColor;
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
                bgcolor = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Base), 0, 0, -10, 0, 0, 0, 95);
                lineColor = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Button), 0, 0, 0, 0, 0, 0, 5);
            } else {
                bgcolor = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Base), 0, 0, 0, 0, 0, 0, 70);
                lineColor = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Button), 0, 0, 60, 0, 0, 0, 0);
            }
            p->setPen(lineColor);
            p->setBrush(bgcolor);

            p->drawLine(opt->rect.topLeft(), opt->rect.topRight());
            p->drawRect(opt->rect);

            return;
        }
        default:
            break;
    }

    DStyle::drawPrimitive(pe, opt, p, w);
}
}  // namespace chameleon
