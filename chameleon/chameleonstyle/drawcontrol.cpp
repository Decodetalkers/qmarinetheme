// for tables
#include "chameleonstyle.h"
#include "common.h"
#include "chameleontools.hpp"
#include <DApplication>
#include <DApplicationHelper>
#include <DButtonBox>
#include <DListView>
#include <DNativeSettings>
#include <DPlatformTheme>
#include <DPlatformWindowHandle>
#include <QTableView>
#include <private/qcombobox_p.h>
#include <private/qcommonstyle_p.h>

#include <qdrawutil.h>
#include <qpa/qplatformwindow.h>
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {
void ChameleonStyle::drawControl(QStyle::ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch (element) {
        case CE_RadioButton:
        case CE_CheckBox:
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                bool isRadio = (element == CE_RadioButton);
                QStyleOptionButton subopt = *btn;
                subopt.rect = subElementRect(isRadio ? SE_RadioButtonIndicator : SE_CheckBoxIndicator, btn, w);
                proxy()->drawPrimitive(isRadio ? PE_IndicatorRadioButton : PE_IndicatorCheckBox, &subopt, p, w);

                subopt.rect = subElementRect(SE_CheckBoxContents, btn, w);
                proxy()->drawControl(CE_CheckBoxLabel, &subopt, p, w);

                if (btn->state & State_HasFocus) {
                    QRect rect(subElementRect(isRadio ? SE_RadioButtonFocusRect : SE_CheckBoxFocusRect, btn, w));

                    if (isRadio) {
                        p->setPen(QPen(getColor(opt, DPalette::Highlight), DStyle::pixelMetric(PM_FocusBorderWidth)));
                        p->drawEllipse(rect.adjusted(1, 1, -1, -1));
                    } else {
                        DDrawUtils::drawBorder(p,
                                               rect,
                                               getColor(opt, DPalette::Highlight),
                                               DStyle::pixelMetric(PM_FocusBorderWidth),
                                               DStyle::pixelMetric(PM_FocusBorderSpacing) + 2);
                    }
                }
            }
            return;
        case CE_CheckBoxLabel:
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                int alignment = static_cast<int>(visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter));

                if (!proxy()->styleHint(SH_UnderlineShortcut, btn, w))
                    alignment |= Qt::TextHideMnemonic;
                QPixmap pix;
                QRect textRect = btn->rect;
                if (!btn->icon.isNull()) {
                    auto icon_mode_state = toIconModeState(opt);  // 与PushButton一致，转换成icon的mode和state.
                    pix = btn->icon.pixmap(
                        w ? w->window()->windowHandle() : nullptr, btn->iconSize, icon_mode_state.first, icon_mode_state.second);
                    proxy()->drawItemPixmap(p, btn->rect, alignment, pix);
                    if (btn->direction == Qt::RightToLeft)
                        textRect.setRight(textRect.right() - btn->iconSize.width() - 4);
                    else
                        textRect.setLeft(textRect.left() + btn->iconSize.width() + 4);
                }
                if (!btn->text.isEmpty()) {
                    proxy()->drawItemText(p,
                                          textRect,
                                          alignment | Qt::TextShowMnemonic,
                                          btn->palette,
                                          btn->state & State_Enabled,
                                          btn->text,
                                          QPalette::WindowText);
                }
            }
            return;
        case CE_ScrollBarSlider: {
            if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
                // 非特效不需要动画，只有显示和隐藏
                if (!DGuiApplicationHelper::isSpecialEffectsEnvironment()) {
                    bool isHoveredOrPressed = scrollBar->state & (QStyle::State_MouseOver | QStyle::State_Sunken);
                    transScrollbarMouseEvents(scrollBar->styleObject, !isHoveredOrPressed);
                    if (!isHoveredOrPressed)
                        return;
                }

                bool hidden = hideScrollBarByAnimation(scrollBar, p);
                // 不绘制则将鼠标消息转发到 parentwidget
                transScrollbarMouseEvents(scrollBar->styleObject, hidden);
                if (hidden)
                    return;

                p->save();
                p->setBrush(getColor(opt, QPalette::Highlight));
                p->setPen(Qt::NoPen);
                p->setRenderHint(QPainter::Antialiasing);
                QRectF rect = opt->rect;
                int realRadius = 0;
                QPoint scrollBarRectCenter;
                int spacing = DStyle::pixelMetric(DStyle::PM_FocusBorderSpacing);

                //用于判断滚动条是否被圆角区域遮住或特殊要求（滚动条上下不到顶、末端），使用方法是 在对应的控件的滚动条，
                //设置添加间隔，其中左、右和上、下每一个都是分开的，且没有遮挡长度的限制;
                //若是超出滚动条的总长度，则表现无变化（不作处理）
                // eg： scrollbar->setProperty("_d_slider_leftOrRight_spacing", 100);
                bool okLeft = false;
                bool okRight = false;
                bool okUp = false;
                bool okDown = false;
                int spacLeft = 0;
                int spacRight = 0;
                int spacUp = 0;
                int spacDown = 0;

                if (opt->styleObject->property("_d_slider_spaceLeft").isValid())
                    spacLeft = opt->styleObject->property("_d_slider_spaceLeft").toInt(&okLeft);
                if (opt->styleObject->property("_d_slider_spaceRight").isValid())
                    spacRight = opt->styleObject->property("_d_slider_spaceRight").toInt(&okRight);
                if (opt->styleObject->property("_d_slider_spaceUp").isValid())
                    spacUp = opt->styleObject->property("_d_slider_spaceUp").toInt(&okUp);
                if (opt->styleObject->property("_d_slider_spaceDown").isValid())
                    spacDown = opt->styleObject->property("_d_slider_spaceDown").toInt(&okDown);

                if (opt->state & QStyle::State_Horizontal) {
                    rect.setHeight(rect.height() / 2);
                    if ((okLeft && spacLeft > 0) || (okRight && spacRight > 0)) {
                        if ((2 * spacing + spacLeft + spacRight) < rect.width()) {
                            rect = rect.adjusted(spacing + spacLeft, 0, -spacing - spacRight, 0);
                        }
                    } else {
                        rect = rect.adjusted(spacing, 0, -spacing, 0);
                    }

                    if (!(opt->state & QStyle::State_MouseOver))
                        rect.setHeight(rect.height() - 2);

                    realRadius = rect.height() / 2.0;

                    scrollBarRectCenter.setX(scrollBar->rect.x() + scrollBar->rect.width() / 2);
                    scrollBarRectCenter.setY((scrollBar->rect.y() + scrollBar->rect.height()) / 2);
                    rect.moveCenter(scrollBarRectCenter);
                    rect.moveBottom(scrollBar->rect.bottom() - 2);
                } else {
                    rect.setWidth(rect.width() / 2);
                    if ((okUp && spacUp > 0) || (okDown && spacDown > 0)) {
                        if ((2 * spacing + spacUp + spacDown) < rect.height()) {
                            rect = rect.adjusted(0, spacing + spacUp, 0, -spacing - spacDown);
                        }
                    } else {
                        rect = rect.adjusted(0, spacing, 0, -spacing);
                    }

                    if (!(opt->state & QStyle::State_MouseOver))
                        rect.setWidth(rect.width() - 2);

                    realRadius = rect.width() / 2.0;

                    scrollBarRectCenter.setX((scrollBar->rect.x() + scrollBar->rect.width()) / 2);
                    scrollBarRectCenter.setY(scrollBar->rect.y() + scrollBar->rect.height() / 2);
                    rect.moveCenter(scrollBarRectCenter);
                    rect.moveRight(scrollBar->rect.right() - 2);
                }

                QColor lineColor(opt->palette.color(QPalette::Base));
                if (DGuiApplicationHelper::toColorType(lineColor) == DGuiApplicationHelper::LightType) {
                    // 内侧绘制一个像素的inside border
                    p->setPen(QPen(QColor(0, 0, 0, 0.05 * 255), Metrics::Painter_PenWidth));
                    // normal状态
                    p->setBrush(QColor(0, 0, 0, 0.3 * 255));

                    if (scrollBar->state & QStyle::State_MouseOver)
                        // hover 状态
                        p->setBrush(QColor(0, 0, 0, 0.6 * 255));
                    if (scrollBar->state & QStyle::State_Sunken)
                        // active状态
                        p->setBrush(QColor(0, 0, 0, 0.5 * 255));
                } else {
                    // 外侧拓展一个像素的outside border
                    p->setPen(QPen(QColor(0, 0, 0, 0.2 * 255), Metrics::Painter_PenWidth));
                    p->setBrush(Qt::NoBrush);
                    p->drawRoundedRect(rect.adjusted(-1, -1, 1, 1), realRadius, realRadius);
                    // 内侧绘制一个像素的inside border
                    p->setPen(QPen(QColor(255, 255, 255, 0.05 * 255), Metrics::Painter_PenWidth));
                    // normal状态
                    p->setBrush(QColor(96, 96, 96, 0.7 * 255));

                    if (scrollBar->state & QStyle::State_MouseOver)
                        // hover 状态
                        p->setBrush(QColor(96, 96, 96, 0.8 * 255));
                    if (scrollBar->state & QStyle::State_Sunken)
                        // active状态
                        p->setBrush(QColor(112, 112, 112, 0.8 * 255));
                }

                p->drawRoundedRect(rect, realRadius, realRadius);
                p->restore();
            }
            break;
        }
        case CE_MenuBarItem: {
            if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
                p->save();
                QRect rect = mbi->rect;
                drawMenuBarItem(mbi, rect, p, w);
                p->restore();
                return;
            }
        } break;
        case CE_MenuBarEmptyArea: {
            p->save();
            QRect menubarRect = opt->rect;
            p->setPen(Qt::NoPen);
            p->setBrush(getColor(opt, QPalette::Window));
            p->drawRect(menubarRect);
            p->restore();
            return;
        }
        case CE_MenuItem: {
            if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
                p->save();
                drawMenuItem(menuItem, p, w);
                p->restore();
                return;
            }
            break;
        }
        case CE_MenuEmptyArea: {
            drawMenuItemBackground(opt, p, QStyleOptionMenuItem::EmptyArea);
            return;
        }
        case CE_MenuScroller: {
            QStyleOption arrowOpt = *opt;
            arrowOpt.state |= State_Enabled;
            int minSize = qMin(arrowOpt.rect.width(), arrowOpt.rect.height());
            arrowOpt.rect.setWidth(minSize);
            arrowOpt.rect.setHeight(minSize);
            arrowOpt.rect.moveCenter(opt->rect.center());
            proxy()->drawPrimitive(
                ((opt->state & State_DownArrow) ? PE_IndicatorArrowDown : PE_IndicatorArrowUp), &arrowOpt, p, w);
            return;
        }
        case CE_PushButton: {
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                proxy()->drawControl(CE_PushButtonBevel, btn, p, w);
                QStyleOptionButton subopt = *btn;
                subopt.rect -= frameExtentMargins();
                subopt.rect = proxy()->subElementRect(SE_PushButtonContents, &subopt, w);
                proxy()->drawControl(CE_PushButtonLabel, &subopt, p, w);

                if (btn->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*btn);
                    fropt.rect = proxy()->subElementRect(SE_PushButtonFocusRect, btn, w);
                    proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, p, w);
                }

                return;
            }
            break;
        }
        case CE_PushButtonBevel: {
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                QRect br = btn->rect;
                int dbi = proxy()->pixelMetric(PM_ButtonDefaultIndicator, btn, w);
                if (btn->features & QStyleOptionButton::DefaultButton)
                    proxy()->drawPrimitive(PE_FrameDefaultButton, opt, p, w);
                if (btn->features & QStyleOptionButton::AutoDefaultButton)
                    br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);
                if (!(btn->features & (QStyleOptionButton::Flat | QStyleOptionButton::CommandLinkButton)) ||
                    btn->state & (State_Sunken | State_On) ||
                    (btn->features & QStyleOptionButton::CommandLinkButton && btn->state & State_MouseOver)) {
                    QStyleOptionButton tmpBtn = *btn;
                    tmpBtn.rect = br;
                    proxy()->drawPrimitive(PE_PanelButtonCommand, &tmpBtn, p, w);
                }
            }
            return;
        }
        case CE_TabBarTabShape: {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
                if (drawTabBar(p, tab, w))
                    return;
            }
        } break;
        case CE_TabBarTabLabel: {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
                if (drawTabBarLabel(p, tab, w))
                    return;
            }
        } break;
        case CE_TabBarTab: {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
                QStyleOptionButton btn;
                btn.rect = tab->rect;
                bool type_check = false;
                if (w)
                    type_check = w->property("_d_dtk_tabbartab_type").toBool();

                int leftMarge = 0;
                int rightMarge = 0;
                if (!type_check) {
                    leftMarge = TabBar_TabMargin / 2;
                    rightMarge = TabBar_TabMargin / 2;
                }
                if (verticalTabs(tab->shape)) {
                    btn.rect.adjust(0, leftMarge, 0, -(rightMarge));
                } else {
                    btn.rect.adjust(leftMarge, 0, -(rightMarge), 0);
                }

                btn.state = tab->state;

                if (tab->state & QStyle::State_Selected) {
                    btn.state |= QStyle::State_On;
                }

                if (type_check) {
                    QColor inactive;
                    if (btn.state & State_On) {
                        inactive = getColor(opt, QPalette::ToolTipBase);

                        if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::DarkType) {
                            inactive = DGuiApplicationHelper::adjustColor(
                                getColor(opt, QPalette::ToolTipBase), 0, 0, 0, -10, -10, -10, 0);
                        }

                        p->setBrush(inactive);
                    } else {
                        // 初始化 tabbar 的背景色
                        if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::LightType ||
                            (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType &&
                             DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::UnknownType)) {
                            inactive =
                                DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Light), 0, 0, 0, +20, +20, +20, 0);
                        } else if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::DarkType ||
                                   (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType &&
                                    DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::UnknownType)) {
                            inactive =
                                DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Light), 0, 0, 0, -57, -57, -57, 0);
                        } else {
                            inactive =
                                DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Light), 0, 0, 0, +20, +20, +20, 0);
                        }

                        p->setBrush(inactive);
                        // Bug:33899 此处添加update出现重复触发绘图事件 导致cpu占用过高 目前注释未发现问题
                        // const_cast<QWidget *>(w)->update();
                    }

                    p->setPen(Qt::NoPen);
                    p->setRenderHint(QPainter::Antialiasing);
                    p->drawRect(opt->rect);

                    // 绘制边框线
                    if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::LightType ||
                        (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::UnknownType &&
                         DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)) {
                        p->setPen(QPen(getColor(opt, DPalette::FrameBorder, w), Metrics::Painter_PenWidth));
                    } else if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::DarkType ||
                               (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::UnknownType &&
                                DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)) {
                        p->setPen(QPen(QColor(0, 0, 0, static_cast<int>(0.05 * 255)), Metrics::Painter_PenWidth));
                    } else {
                        p->setPen(QPen(getColor(opt, DPalette::FrameBorder, w), Metrics::Painter_PenWidth));
                    }

                    p->setBrush(Qt::NoBrush);
                    p->drawRect(opt->rect);
                    //对中间的tabbar尾后加一根明显的线
                    if (QStyleOptionTab::End != tab->position && QStyleOptionTab::OnlyOneTab != tab->position) {
                        const QRect &lintRect = opt->rect;
                        if (verticalTabs(tab->shape)) {
                            p->drawLine(lintRect.bottomLeft(), lintRect.bottomRight());
                        } else {
                            p->drawLine(lintRect.topRight(), lintRect.bottomRight());
                        }
                    }
                } else {
                    DStyle::drawControl(CE_PushButtonBevel, &btn, p, w);
                }

                QStyleOptionTab *newTab = const_cast<QStyleOptionTab *>(tab);
                newTab->rect = btn.rect;
                proxy()->drawControl(CE_TabBarTabLabel, newTab, p, w);
                return;
            }
            break;
        }
        case CE_RubberBand: {
            if (qstyleoption_cast<const QStyleOptionRubberBand *>(opt)) {
                p->save();
                QColor color = opt->palette.highlight().color();
                color.setAlphaF(0.1);

                // draw rectangle
                p->setRenderHint(QPainter::Antialiasing, false);
                p->fillRect(opt->rect, color);

                // draw inner border
                // 保证border绘制在矩形内部，且不超越了矩形范围
                color.setAlphaF(0.2);
                p->setClipRegion(QRegion(opt->rect) - opt->rect.adjusted(1, 1, -1, -1));
                p->fillRect(opt->rect, color);
                p->restore();
                return;
            }
            break;
        }
        case CE_Header:
            if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
                QRegion clipRegion = p->clipRegion();
                p->setClipRect(opt->rect);
                proxy()->drawControl(CE_HeaderSection, header, p, w);
                QStyleOptionHeader subopt = *header;
                subopt.rect = subElementRect(SE_HeaderLabel, header, w);
                if (subopt.rect.isValid())
                    proxy()->drawControl(CE_HeaderLabel, &subopt, p, w);
                if (header->sortIndicator != QStyleOptionHeader::None) {
                    subopt.rect = subElementRect(SE_HeaderArrow, opt, w);
                    proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &subopt, p, w);
                }
                p->setClipRegion(clipRegion);
            }
            return;
        case CE_ShapedFrame: {
            if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
                int frameShape = f->frameShape;
                int frameShadow = QFrame::Plain;
                if (f->state & QStyle::State_Sunken) {
                    frameShadow = QFrame::Sunken;
                } else if (f->state & QStyle::State_Raised) {
                    frameShadow = QFrame::Raised;
                }

                int lw = f->lineWidth;
                int mlw = f->midLineWidth;
                QPalette::ColorRole foregroundRole = QPalette::WindowText;
                if (w)
                    foregroundRole = w->foregroundRole();

                switch (frameShape) {
                    case QFrame::Box:
                        if (qobject_cast<QComboBoxPrivateContainer *>(const_cast<QWidget *>(w)) &&
                            DGuiApplicationHelper::isTabletEnvironment())
                            break;

                        if (frameShadow == QFrame::Plain) {
                            qDrawPlainRect(p, f->rect, f->palette.color(foregroundRole), lw);
                        } else {
                            qDrawShadeRect(p, f->rect, f->palette, frameShadow == QFrame::Sunken, lw, mlw);
                        }
                        break;
                    case QFrame::StyledPanel:
                        // keep the compatibility with Qt 4.4 if there is a proxy style.
                        // be sure to call drawPrimitive(QStyle::PE_Frame) on the proxy style
                        if (w) {
                            w->style()->drawPrimitive(QStyle::PE_Frame, opt, p, w);
                        } else {
                            proxy()->drawPrimitive(QStyle::PE_Frame, opt, p, w);
                        }
                        break;
                    case QFrame::Panel:
                        if (frameShadow == QFrame::Plain) {
                            qDrawPlainRect(p, f->rect, f->palette.color(foregroundRole), lw);
                        } else {
                            qDrawShadePanel(p, f->rect, f->palette, frameShadow == QFrame::Sunken, lw);
                        }
                        break;
                    case QFrame::WinPanel:
                        if (frameShadow == QFrame::Plain) {
                            qDrawPlainRect(p, f->rect, f->palette.color(foregroundRole), lw);
                        } else {
                            qDrawWinPanel(p, f->rect, f->palette, frameShadow == QFrame::Sunken);
                        }
                        break;
                    case QFrame::HLine:
                    case QFrame::VLine: {
                        QPoint p1, p2;
                        if (frameShape == QFrame::HLine) {
                            p1 = QPoint(opt->rect.x(), opt->rect.y() + opt->rect.height() / 2);
                            p2 = QPoint(opt->rect.x() + opt->rect.width(), p1.y());
                        } else {
                            p1 = QPoint(opt->rect.x() + opt->rect.width() / 2, opt->rect.y());
                            p2 = QPoint(p1.x(), p1.y() + opt->rect.height());
                        }
                        if (frameShadow == QFrame::Plain) {
                            QPen oldPen = p->pen();
                            QColor color = opt->palette.color(foregroundRole);
                            color.setAlphaF(0.1);
                            p->setPen(QPen(color, lw));
                            p->drawLine(p1, p2);
                            p->setPen(oldPen);
                        } else {
                            qDrawShadeLine(p, p1, p2, f->palette, frameShadow == QFrame::Sunken, lw, mlw);
                        }
                        break;
                    }
                }
                return;
            }
            break;
        }
        case CE_ComboBoxLabel: {
            if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                if (drawComboBoxLabel(p, cb, w))
                    return;
            }
            break;
        }
        case CE_PushButtonLabel:
            if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                QRect textRect = button->rect;
                uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, button, w))
                    tf |= Qt::TextHideMnemonic;

                const QPalette::ColorRole &text_color_role =
                    opt->state & State_On ? QPalette::HighlightedText : QPalette::ButtonText;

                QPalette pa = button->palette;

                if (button->features & DStyleOptionButton::WarningButton) {
                    pa.setBrush(QPalette::ButtonText, getColor(opt, DPalette::TextWarning, w));
                } else if (button->features & DStyleOptionButton::SuggestButton) {
                    pa.setBrush(QPalette::ButtonText, getColor(opt, QPalette::HighlightedText));
                } else {
                    pa.setBrush(QPalette::ButtonText, getColor(opt, text_color_role));
                }

                // 设置文字和图标的绘制颜色
                p->setPen(QPen(pa.buttonText(), 1));

                if (!button->icon.isNull()) {
                    // Center both icon and text
                    QRect iconRect;

                    int pixmapWidth = button->iconSize.width();
                    int pixmapHeight = button->iconSize.height();
                    int labelWidth = pixmapWidth;
                    int labelHeight = pixmapHeight;
                    int iconSpacing = 4;  //### 4 is currently hardcoded in QPushButton::sizeHint()
                    int textWidth = button->fontMetrics.boundingRect(opt->rect, tf, button->text).width();
                    if (!button->text.isEmpty())
                        labelWidth += (textWidth + iconSpacing);

                    iconRect = QRect(textRect.x() + (textRect.width() - labelWidth) / 2,
                                     textRect.y() + (textRect.height() - labelHeight) / 2,
                                     pixmapWidth,
                                     pixmapHeight);

                    iconRect = visualRect(button->direction, textRect, iconRect);

                    tf |= Qt::AlignLeft;  // left align, we adjust the text-rect instead

                    if (button->direction == Qt::RightToLeft)
                        textRect.setRight(iconRect.left() - iconSpacing);
                    else
                        textRect.setLeft(iconRect.left() + iconRect.width() + iconSpacing);

                    if (button->state & (State_On | State_Sunken))
                        iconRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, w),
                                           proxy()->pixelMetric(PM_ButtonShiftVertical, opt, w));

                    auto icon_mode_state = toIconModeState(opt);
                    button->icon.paint(p, iconRect, Qt::AlignCenter, icon_mode_state.first, icon_mode_state.second);
                } else {
                    tf |= Qt::AlignHCenter;
                }
                if (button->state & (State_On | State_Sunken))
                    textRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, w),
                                       proxy()->pixelMetric(PM_ButtonShiftVertical, opt, w));

                if (button->features & QStyleOptionButton::HasMenu) {
                    QRect rectArrowAndLine = drawButtonDownArrow(opt, nullptr, w);
                    int frameRadius = DStyle::pixelMetric(PM_FrameRadius);
                    drawButtonDownArrow(button, p, w);

                    if (button->direction == Qt::LeftToRight) {
                        textRect.setRight(rectArrowAndLine.left() - frameRadius);
                    } else {
                        textRect.setLeft(rectArrowAndLine.right() + frameRadius);
                    }
                }

                proxy()->drawItemText(p, textRect, tf, pa, (button->state & State_Enabled), button->text, QPalette::ButtonText);
                return;
            }
            break;
        case CE_ProgressBar: {  //显示进度区域
            if (const QStyleOptionProgressBar *progBar = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(Qt::NoPen);
                p->drawRect(opt->rect);

                QStyleOptionProgressBar progGroove = *progBar;
                proxy()->drawControl(CE_ProgressBarGroove, &progGroove, p, w);

                QRect rect = progBar->rect;  //滑块区域矩形
                int min = progBar->minimum;
                int max = progBar->maximum;
                int val = progBar->progress;
                int drawWidth = 0;

                if (progBar->orientation == Qt::Horizontal) {
                    drawWidth = (val * 1.0 / (max - min)) * rect.width();
                    rect = QRect(rect.left(), rect.top(), drawWidth, rect.height());
                } else {
                    drawWidth = (val * 1.0 / (max - min)) * rect.height();
                    rect = QRect(rect.left(), rect.bottom() - drawWidth, rect.width(), drawWidth);
                }

                p->setPen(Qt::NoPen);
                QStyleOptionProgressBar subopt = *progBar;
                proxy()->drawControl(CE_ProgressBarContents, &subopt, p, w);

                if (progBar->textVisible && progBar->orientation == Qt::Horizontal) {
                    subopt.rect = proxy()->subElementRect(SE_ProgressBarLabel, progBar, w);
                    proxy()->drawControl(CE_ProgressBarLabel, &subopt, p, w);
                }
            }
            return;
        }
        case CE_ProgressBarGroove: {  //滑槽显示
            if (const QStyleOptionProgressBar *progBar = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
                int frameRadius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
                int height = progBar->orientation == Qt::Horizontal ? opt->rect.height() : opt->rect.width();
                if (frameRadius * 2 >= height) {
                    frameRadius = qMin(height / 2, 4);
                }
                p->setBrush(getColor(opt, DPalette::ObviousBackground, w));
                p->drawRoundedRect(opt->rect, frameRadius, frameRadius);
            }
            return;
        }
        case CE_ProgressBarContents: {  //进度滑块显示
            if (const QStyleOptionProgressBar *progBar = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
                QStyleOptionProgressBar *progBarTemp = const_cast<QStyleOptionProgressBar *>(progBar);
                progBarTemp->state &= (~State_MouseOver);
                progBarTemp = nullptr;
                QRect rect = progBar->rect;  //滑块区域矩形
                int min = progBar->minimum;
                int max = progBar->maximum;
                int val = progBar->progress;
                int drawWidth = 0;
                int frameRadius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
                int height = progBar->orientation == Qt::Horizontal ? rect.height() : rect.width();
                if (frameRadius * 2 >= height) {
                    frameRadius = qMin(height / 2, 4);
                }

                if (progBar->orientation == Qt::Horizontal) {
                    drawWidth = (val * 1.0 / (max - min)) * rect.width();
                    rect = QRect(rect.left(), rect.top(), drawWidth, rect.height());
                } else {
                    drawWidth = (val * 1.0 / (max - min)) * rect.height();
                    rect = QRect(rect.left(), rect.bottom() - drawWidth, rect.width(), drawWidth);
                }

                QPointF pointStart(rect.left(), rect.center().y());
                QPointF pointEnd(rect.right(), rect.center().y());
                QLinearGradient linear(pointStart, pointEnd);
                QColor startColor = getColor(opt, DPalette::Highlight);
                QColor endColor = DGuiApplicationHelper::adjustColor(startColor, 0, 0, +30, 0, 0, 0, 0);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, endColor);
                linear.setSpread(QGradient::PadSpread);
                p->setBrush(QBrush(linear));

                if (progBar->textVisible) {
                    QPainterPath pathRect;
                    pathRect.addRect(rect);
                    QPainterPath pathRoundRect;
                    pathRoundRect.addRoundedRect(opt->rect, frameRadius, frameRadius);
                    QPainterPath inter = pathRoundRect.intersected(pathRect);
                    p->drawPath(inter);
                } else {
                    //进度条高度 <= 8px && 进度条宽度 <= 8px && value有效
                    if (rect.height() <= ProgressBar_MinimumStyleHeight && rect.width() <= ProgressBar_MinimumStyleHeight &&
                        progBar->progress > 0) {
                        QPainterPath path;
                        QRect startRect = rect;
                        startRect.setWidth(rect.height());
                        startRect.setHeight(rect.height());
                        path.moveTo(rect.x() + startRect.width() / 2.0, rect.y());
                        //绘制进度条最小样式前半圆
                        path.arcTo(startRect, 90, 180);
                        p->drawPath(path);

                        //绘制进度条最小样式后半圆
                        if (rect.width() > startRect.width() / 2) {
                            QRect endRect = startRect;
                            int width = rect.width() - startRect.width() / 2;
                            endRect.setX(startRect.x() + startRect.width() / 2 - width);
                            endRect.setWidth(width * 2);

                            QPainterPath path2;
                            path2.moveTo(endRect.x() + endRect.width() / 2.0, rect.y());
                            path2.arcTo(endRect, 90, -180);
                            p->drawPath(path2);
                        }
                    } else
                        p->drawRoundedRect(rect, frameRadius, frameRadius);
                }
            }
            return;
        }
        case CE_ProgressBarLabel: {
            if (const QStyleOptionProgressBar *progBar = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
                double val = progBar->progress * 1.0 / (progBar->maximum - progBar->minimum);
                int radius = DStyle::pixelMetric(PM_FrameRadius);
                int drawWidth = val * (opt->rect.width() + 2 * radius);

                QRect rect = progBar->fontMetrics.boundingRect(progBar->rect, progBar->textAlignment, progBar->text);

                if (rect.left() <= drawWidth && drawWidth <= rect.right()) {
                    double division = (drawWidth - rect.left()) / (rect.width() * 1.0);
                    QPointF pointStart(rect.left(), rect.center().y());
                    QPointF pointEnd(rect.right(), rect.center().y());
                    QLinearGradient linear(pointStart, pointEnd);
                    linear.setColorAt(0, getColor(opt, DPalette::HighlightedText));
                    linear.setColorAt(division, getColor(opt, DPalette::HighlightedText));
                    linear.setColorAt(division + 0.01, getColor(opt, DPalette::ButtonText));
                    linear.setColorAt(1, getColor(opt, DPalette::ButtonText));
                    linear.setSpread(QGradient::PadSpread);

                    p->setPen(QPen(QBrush(linear), 1));  //设置画笔渐变色
                } else if (drawWidth < rect.left()) {
                    p->setPen(getColor(opt, DPalette::ButtonText));
                } else {
                    p->setPen(getColor(opt, DPalette::HighlightedText));
                }

                p->drawText(progBar->rect, progBar->textAlignment, progBar->text);
            }
            return;
        }
        case CE_ToolButtonLabel: {
            if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
                QRect rect = toolbutton->rect;
                int toolButtonAlign = Qt::AlignLeft;
                if (w)
                    toolButtonAlign = w->property("_d_dtk_toolButtonAlign").toInt();  // 设置tool button的对齐方式
                int radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);  //在绘画icon和text之前,先绘画一层表示靠近或按下状态
                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(Qt::NoPen);
                p->setBrush(Qt::NoBrush);

                if (toolbutton->state & (State_MouseOver | State_Sunken))  // hover状态 、press状态
                    p->setBrush(getBrush(toolbutton, DPalette::Button));

                // 繪制背景
                if (toolbutton->state & State_Enabled) {
                    if ((toolbutton->features & QStyleOptionToolButton::MenuButtonPopup) &&
                        (toolbutton->state & (QStyle::State_MouseOver | QStyle::State_Sunken))) {
                        // 绘制外层背景色
                        int menuButtonIndicatorMargin = 4;
                        auto btn = *toolbutton;
                        if (btn.state & (QStyle::State_MouseOver))
                            btn.state &= ~QStyle::State_MouseOver;
                        if (btn.state & (QStyle::State_Sunken))
                            btn.state &= ~QStyle::State_Sunken;
                        p->setPen(getColor(&btn, DPalette::Button));
                        p->setBrush(getBrush(&btn, DPalette::Button));
                        QRect tmp = rect;
                        tmp.adjust(
                            0, 0, proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w) + 2 * menuButtonIndicatorMargin, 0);
                        p->drawRoundedRect(tmp, radius, radius);

                        // 绘制子控件背景色
                        p->setPen(getColor(toolbutton, DPalette::Button));
                        p->setBrush(getBrush(toolbutton, DPalette::Button));
                        if (toolbutton->activeSubControls & QStyle::SC_ToolButton) {
                            DDrawUtils::drawRoundedRect(
                                p, rect, radius, radius, DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
                        } else if (toolbutton->activeSubControls & QStyle::SC_ToolButtonMenu) {
                            QRect r = rect;
                            r.adjust(r.width(),
                                     0,
                                     proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w) + 2 * menuButtonIndicatorMargin,
                                     0);
                            DDrawUtils::drawRoundedRect(
                                p, r, radius, radius, DDrawUtils::TopRightCorner | DDrawUtils::BottomRightCorner);
                        }
                    } else {
                        p->drawRoundedRect(rect, radius, radius);
                    }
                }

                // Arrow type always overrules and is always shown
                bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
                if (((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty()) ||
                    toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {  //只显示文字的情景
                    int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                    if (!proxy()->styleHint(SH_UnderlineShortcut, opt, w))
                        alignment |= Qt::TextHideMnemonic;
                    p->setFont(toolbutton->font);
                    if (toolbutton->state & State_On) {
                        p->setPen(getColor(toolbutton, DPalette::Highlight));
                    } else {
                        p->setPen(getColor(toolbutton, DPalette::ButtonText));
                    }

                    p->drawText(rect, alignment, toolbutton->text);
                } else {  //只显示文字的情景 的 补集
                    QIcon icon;
                    QSize pmSize = toolbutton->iconSize;

                    switch (toolbutton->arrowType) {
                        case Qt::UpArrow:
                            icon = DStyle::standardIcon(SP_ArrowUp);
                            break;
                        case Qt::DownArrow:
                            icon = DStyle::standardIcon(SP_ArrowDown);
                            break;
                        case Qt::LeftArrow:
                            icon = DStyle::standardIcon(SP_ArrowLeft);
                            break;
                        case Qt::RightArrow:
                            icon = DStyle::standardIcon(SP_ArrowRight);
                            break;
                        default:
                            icon = toolbutton->icon;
                            break;
                    }

                    p->setRenderHint(QPainter::Antialiasing);
                    p->setPen(Qt::NoPen);
                    p->setBrush(Qt::NoBrush);

                    if (toolbutton->state & (State_MouseOver | State_Sunken))  // hover状态 、press状态
                        p->setBrush(getBrush(toolbutton, DPalette::Button));

                    if (toolbutton->state & State_HasFocus)
                        p->setPen(getColor(toolbutton, DPalette::Highlight));

                    //强制绘制　日历　左右翻页背景
                    if (w && (w->objectName() == "qt_calendar_prevmonth" || w->objectName() == "qt_calendar_nextmonth")) {
                        p->setBrush(getColor(toolbutton, DPalette::Button));
                    }

                    // pr为图标的大小
                    QRect pr = rect;
                    // tr为文字的大小
                    QRect tr = rect;
                    pr.setHeight(pmSize.height());
                    pr.setWidth(pmSize.width());

                    if (toolbutton->state & State_On)  // active状态
                        p->setPen(getColor(toolbutton, DPalette::Highlight));
                    else
                        p->setPen(getColor(toolbutton, DPalette::ButtonText));

                    if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {  //只显示icon 的补集情况
                        p->setFont(toolbutton->font);

                        int alignment = Qt::TextShowMnemonic;
                        if (!proxy()->styleHint(SH_UnderlineShortcut, opt, w))
                            alignment |= Qt::TextHideMnemonic;

                        if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {  //文字在图标下面
                            pr.moveCenter(QPoint(rect.center().x(), rect.center().y() / 2));
                            tr.adjust(0, pr.height(), 0, 0);

                            drawIcon(toolbutton, p, pr, icon);
                            alignment |= Qt::AlignCenter;

                        } else if (toolbutton->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                            if (toolButtonAlign == Qt::AlignCenter) {  // toolButton居中对齐
                                //计算文字宽度
                                int textWidget = w->fontMetrics().horizontalAdvance(toolbutton->text);
                                //图标 spacing 文字的矩形
                                QRect textIcon = QRect(0, 0, pr.width() + ToolButton_MarginWidth + textWidget, rect.height());
                                textIcon.moveCenter(rect.center());
                                pr.moveCenter(rect.center());
                                //图标padding
                                pr.moveLeft(textIcon.x() > ToolButton_ItemSpacing ? textIcon.x() : ToolButton_ItemSpacing);
                                //调整text距离
                                tr.adjust(pr.width() + ToolButton_AlignCenterPadding, 0, 0, 0);
                                drawIcon(toolbutton, p, pr, icon);
                                alignment |= Qt::AlignCenter;
                            } else if (toolButtonAlign == Qt::AlignRight) {  // toolButton右对齐
                                int textWidget = w->fontMetrics().horizontalAdvance(toolbutton->text);
                                pr.moveCenter(rect.center());
                                pr.moveRight(tr.width() - textWidget - ToolButton_AlignLeftPadding - ToolButton_ItemSpacing);
                                tr.adjust(-ToolButton_AlignLeftPadding - pr.width() - ToolButton_MarginWidth,
                                          0,
                                          -ToolButton_AlignLeftMargin,
                                          0);
                                drawIcon(toolbutton, p, pr, icon);
                                alignment |= Qt::AlignVCenter | Qt::AlignRight;
                            } else {  // toolButton左对齐
                                pr.moveCenter(rect.center());
                                pr.moveLeft(ToolButton_AlignRightPadding);
                                tr.adjust(ToolButton_AlignRightPadding + pr.width() + ToolButton_MarginWidth,
                                          0,
                                          -ToolButton_ItemSpacing,
                                          0);
                                drawIcon(toolbutton, p, pr, icon);
                                alignment |= Qt::AlignVCenter | Qt::AlignLeft;
                            }
                        } else {  //其他几种（文字和icon布局）方式
                            int radius = DStyle::pixelMetric(PM_FrameRadius);
                            pr.moveCenter(QPoint(rect.left(), rect.center().y()));
                            pr.moveLeft(radius);
                            tr.adjust(pr.width() + radius, 0, 0, 0);

                            drawIcon(toolbutton, p, pr, icon);
                            alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                        }

                        p->drawText(tr, alignment, toolbutton->text);
                    } else {  //只显示icon情况
                        if (toolbutton->features & QStyleOptionToolButton::HasMenu &&
                            !(toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)) {
                            rect.adjust(0, 0, -(proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w) + 4), 0);
                        }
                        pr.moveCenter(rect.center());
                        drawIcon(toolbutton, p, pr, icon);
                    }
                }
            }
            return;
        }
        case CE_HeaderSection: {
            const auto headerOption(qstyleoption_cast<const QStyleOptionHeader *>(opt));
            if (!headerOption)
                return;
            const bool horizontal(headerOption->orientation == Qt::Horizontal);
            const bool isLast(headerOption->position == QStyleOptionHeader::End);

            // fill background
            QColor color(opt->palette.color(QPalette::Base));
            QColor lineColor(opt->palette.color(QPalette::Button));  // 挑选一个比较接近的基准色 Button，基于此做微调

            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
                color = DGuiApplicationHelper::adjustColor(color, 0, 0, 0, 0, 0, 0, 60);
                lineColor = DGuiApplicationHelper::adjustColor(lineColor, 0, 0, 0, 0, 0, 0, 10);
            } else {
                color = DGuiApplicationHelper::adjustColor(color, 0, 0, -20, 0, 0, 0, 80);
                lineColor = DGuiApplicationHelper::adjustColor(lineColor, 0, 0, -25, 0, 0, 0, 0);
            }

            p->fillRect(opt->rect, QBrush(color));
            p->setPen(lineColor);
            if (horizontal) {
                if (!isLast) {
                    QPoint unit(0, opt->rect.height() / 5);
                    p->drawLine(opt->rect.topRight() + unit, opt->rect.bottomRight() - unit);
                }
                p->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
            } else {
                if (!isLast) {
                    p->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
                }
                p->drawLine(opt->rect.topRight(), opt->rect.bottomRight());
            }
            return;
        }
        case CE_SizeGrip: {
            p->save();
            int x, y, w, h;
            opt->rect.getRect(&x, &y, &w, &h);

            int sw = qMin(h, w);
            if (h > w)
                p->translate(0, h - w);
            else
                p->translate(w - h, 0);

            Qt::Corner corner;
            if (const QStyleOptionSizeGrip *sgOpt = qstyleoption_cast<const QStyleOptionSizeGrip *>(opt))
                corner = sgOpt->corner;
            else if (opt->direction == Qt::RightToLeft)
                corner = Qt::BottomLeftCorner;
            else
                corner = Qt::BottomRightCorner;

            bool ok = false;
            int radius = DStyle::pixelMetric(PM_FrameRadius, opt) / 2;
            int _d_radius = 0;

            if (opt->styleObject->property("_d_radius").isValid())
                _d_radius = opt->styleObject->property("_d_radius").toInt(&ok);

            if (ok && _d_radius >= 0 && _d_radius != radius)
                radius = _d_radius;

            p->setRenderHint(QPainter::Antialiasing, true);
            DGuiApplicationHelper *guiAppHelp = DGuiApplicationHelper::instance();
            if (guiAppHelp->themeType() == DGuiApplicationHelper::ColorType::DarkType) {  //暗色主题
                p->setPen(QPen(QColor(255, 255, 255, 0.2 * 255), 1));
            } else {
                p->setPen(QPen(QColor(0, 0, 0, 0.2 * 255), 1));
            }

            QRectF rectInner(0, 0, 1.4 * sw, 1.4 * sw);  // 内侧弧线的外切正方形
            QRectF rectExternal(0, 0, 2 * sw, 2 * sw);   // 外侧弧线的外切正方形

            if (corner == Qt::BottomLeftCorner) {
                rectExternal.moveBottomLeft(QPointF(opt->rect.bottomLeft().x() + radius, opt->rect.bottomLeft().y() - radius));
                rectInner.moveCenter(rectExternal.center());
                p->drawArc(rectInner, 205 * 16, 40 * 16);
                p->drawArc(rectExternal, 200 * 16, 50 * 16);
            } else if (corner == Qt::BottomRightCorner) {
                rectExternal.moveBottomRight(QPointF(opt->rect.bottomRight().x() - radius, opt->rect.bottomRight().y() - radius));
                rectInner.moveCenter(rectExternal.center());
                p->drawArc(rectInner, 295 * 16, 40 * 16);
                p->drawArc(rectExternal, 290 * 16, 50 * 16);
            } else if (corner == Qt::TopRightCorner) {
                rectExternal.moveTopRight(QPointF(opt->rect.topRight().x() - radius, opt->rect.topRight().y() + radius));
                rectInner.moveCenter(rectExternal.center());
                p->drawArc(rectInner, 25 * 16, 40 * 16);
                p->drawArc(rectExternal, 25 * 16, 50 * 16);
            } else if (corner == Qt::TopLeftCorner) {
                rectExternal.moveTopLeft(QPointF(opt->rect.topLeft().x() + radius, opt->rect.topLeft().y() + radius));
                rectInner.moveCenter(rectExternal.center());
                p->drawArc(rectInner, 115 * 16, 40 * 16);
                p->drawArc(rectExternal, 110 * 16, 50 * 16);
            }
            p->restore();
            return;
        }
        case CE_ItemViewItem: {
            if (w && w->objectName() == "qt_calendar_calendarview") {
                if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                    p->save();
                    p->setClipRect(opt->rect);

                    //绘制禁用项
                    if (!(vopt->state & QStyle::State_Enabled)) {
                        p->save();
                        p->setPen(Qt::NoPen);
                        p->setBrush(getColor(vopt, DPalette::Window));
                        p->drawRect(vopt->rect);
                        p->restore();
                    }

                    // 绘制当前选中项
                    proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, w);

                    // draw the text
                    if (!vopt->text.isEmpty()) {
                        QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
                        if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                            cg = QPalette::Inactive;

                        if (vopt->state & QStyle::State_Selected) {
                            p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
                        } else {
                            p->setPen(vopt->palette.color(cg, QPalette::Text));
                        }

                        QCommonStylePrivate *d = reinterpret_cast<QCommonStylePrivate *>(qGetPtrHelper(d_ptr));
                        d->viewItemDrawText(p, vopt, opt->rect);
                    }

                    //绘制日历分割线
                    if (vopt->index.row() == 0) {
                        p->save();
                        QColor color = getColor(vopt, DPalette::FrameBorder, w);
                        color.setAlpha(static_cast<int>(255 * 0.05));
                        QPen pen(color, 2);
                        p->setPen(pen);
                        const QTableView *view = static_cast<const QTableView *>(w);

                        int margins = DStyle::pixelMetric(proxy(), DStyle::PM_ContentsMargins);

                        if (vopt->index.column() == 0)
                            p->drawLine(vopt->rect.bottomLeft() + QPoint(margins, 0), vopt->rect.bottomRight());
                        else if (vopt->index.column() == view->model()->columnCount() - 1) {
                            p->drawLine(vopt->rect.bottomLeft(), vopt->rect.bottomRight() - QPoint(margins, 0));
                        } else {
                            p->drawLine(vopt->rect.bottomLeft(), vopt->rect.bottomRight());
                        }

                        p->restore();
                    }

                    // draw the focus rect
                    if (vopt->state & QStyle::State_HasFocus) {
                        QStyleOptionFocusRect o;
                        o.QStyleOption::operator=(*vopt);
                        o.rect = proxy()->subElementRect(SE_ItemViewItemFocusRect, vopt, w);
                        o.state |= QStyle::State_KeyboardFocusChange;
                        o.state |= QStyle::State_Item;
                        QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
                        o.backgroundColor = vopt->palette.color(
                            cg, (vopt->state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Window);
                        proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, w);
                    }

                    p->restore();
                }
                return;
            }
            break;
        }
        default:
            break;
    }

    DStyle::drawControl(element, opt, p, w);
}
}  // namespace chameleon
