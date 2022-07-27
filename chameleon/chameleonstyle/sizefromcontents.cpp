//#include "chameleonstyle.h"
//#include "common.h"
//#include "gettypecolor.hpp"
//
//#include <DListView>
//#include <QToolButton>
//#include <private/qcombobox_p.h>
#include "chameleonstyle.h"
#include "common.h"
#include "chameleontools.hpp"
#include <DPlatformTheme>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>

#include <DIconButton>
#include <DListView>
#include <private/qcombobox_p.h>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {

QSize ChameleonStyle::sizeFromContents(QStyle::ContentsType ct,
                                       const QStyleOption *opt,
                                       const QSize &contentsSize,
                                       const QWidget *widget) const
{
    QSize size = DStyle::sizeFromContents(ct, opt, contentsSize, widget);

    switch (ct) {
        case CT_LineEdit: {
            int button_margin = proxy()->pixelMetric(QStyle::PM_ButtonMargin, opt, widget);
            size += QSize(button_margin, button_margin);
            Q_FALLTHROUGH();
        }
        case CT_ComboBox: {
            if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                if (cmb->editable) {
                    //这是从lineedit设置margin处拿来
                    int frame_margins = DStyle::pixelMetric(PM_FrameMargins, opt, widget);
                    int left_margins = DStyle::pixelMetric(PM_ContentsMargins, opt, widget);
                    size.setWidth(size.width() + frame_margins + left_margins);
                } else {
                    size.setWidth(size.width() + opt->fontMetrics.horizontalAdvance("..."));  //设置宽度为最小省略号("...")的宽度
                }
            }
            Q_FALLTHROUGH();
        }
        case CT_TabBarTab: {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
                QStyleOptionButton button;
                button.QStyleOption::operator=(*opt);
                button.text = tab->text;
                size = DStyle::sizeFromContents(QStyle::CT_PushButton, &button, tab->fontMetrics.size(0, tab->text), widget);
                int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, widget);
                // 获得Icon引起的增量
                int iconSizeDelta = 0;
                if (!tab->icon.isNull()) {
                    iconSizeDelta += tab->iconSize.width();
                    if (!tab->text.isEmpty())
                        iconSizeDelta += Icon_Margins;
                }
                size.rwidth() +=
                    2 * frame_radius + proxy()->pixelMetric(PM_TabCloseIndicatorWidth, opt, widget) + TabBar_TabMargin;
                size.rwidth() += iconSizeDelta;
                // TabBar 竖直方向改变其宽高
                if (verticalTabs(tab->shape)) {
                    qSwap(size.rwidth(), size.rheight());
                }
            }
            Q_FALLTHROUGH();
        }
        case CT_PushButton: {
            int frame_margins = DStyle::pixelMetric(PM_FrameMargins, opt, widget);
            size += QSize(frame_margins * 2, frame_margins * 2);

            if (const QStyleOptionButton *bopt = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, widget);

                // 为文本添加额外的margin
                if (!bopt->text.isEmpty()) {
                    size.rwidth() += 2 * frame_radius;
                }

                if (bopt->features & QStyleOptionButton::HasMenu)
                    size.rwidth() += frame_margins;  // qt源码会在带有menu的btn样式中,添加一个箭头矩形的width
            }

            int button_min_size = DStyle::pixelMetric(PM_ButtonMinimizedSize, opt, widget);
            size = size.expandedTo(QSize(button_min_size, button_min_size));
            break;
        }
        case CT_ItemViewItem: {
            if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                const QMargins &item_margins = qvariant_cast<QMargins>(vopt->index.data(Dtk::MarginsRole));

                if (!item_margins.isNull()) {
                    //加上Item自定义的margins
                    size = QRect(QPoint(0, 0), size).marginsAdded(item_margins).size();
                }
                size.setWidth(size.width() + opt->fontMetrics.horizontalAdvance("xxx"));

                return size;
            }
            break;
        }
        case CT_Slider: {
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
                /*2019-09-19　约定枚举值含义　　　　　中文含义
                 * PM_SliderThickness:　　　　　　Slider总的高度　＝　滑块高度＋刻度高度
                 * PM_SliderControlThickness:   只是滑块的单独高度
                 * PM_SliderLength:             只是滑块的长度
                 * PM_SliderTickmarkOffset:     用作slider的刻度线的高度
                 * PM_SliderSpaceAvailable      暂时未用到
                 */

                int sliderContHeight = proxy()->pixelMetric(PM_SliderControlThickness, opt, widget);
                int tickMarkHeight = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, widget);
                sliderContHeight += tickMarkHeight;

                if (slider->tickPosition == QSlider::NoTicks) {
                    sliderContHeight -= tickMarkHeight;
                } else if (slider->tickPosition == QSlider::TicksBothSides) {
                    sliderContHeight += tickMarkHeight;
                } else {
                }

                int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);
                if (slider->orientation == Qt::Horizontal) {
                    size.setHeight(qMax(size.height(), sliderContHeight) + (2 * margin));
                    size.setWidth(50);
                } else {
                    size.setWidth(qMax(size.width(), sliderContHeight) + (2 * margin));
                }
            }
            break;
        }
        case CT_MenuBarItem: {
            int frame_margins = DStyle::pixelMetric(PM_FrameMargins, opt, widget);
            size += QSize(frame_margins * 2, frame_margins * 2);
            break;
        }
        case CT_MenuItem: {
            if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
                int m_width = size.width();
                bool hideShortcutText = false;

                if (hideShortcutText) {
                    m_width -= menuItem->tabWidth;
                    int tabIndex = menuItem->text.indexOf(QLatin1Char('\t'));

                    if (tabIndex != -1) {
                        int textWidth = menuItem->fontMetrics.horizontalAdvance(menuItem->text.mid(tabIndex + 1));

                        if (menuItem->tabWidth == 0)
                            m_width -= textWidth;
                    }
                }

                int frameRadius = DStyle::pixelMetric(
                    PM_FrameRadius);  //打钩矩形的左侧距离item的左边缘； 也是 打钩矩形的右侧距离 图文内容的左边缘
                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, opt, widget);  //打钩的宽度
                int realMargins = smallIconSize + 2 * frameRadius;  //左侧固定预留的margin，无论是否能够打钩都要预留

                m_width = realMargins;
#ifdef ENABLE_RED_POINT_TEXT
                int redPointWith = opt->fontMetrics.size(Qt::TextSingleLine, QLatin1String("99+")).width();
                m_width += redPointWith;
#endif
                int tabSpacing = MenuItem_TabSpacing;
                if (menuItem->text.contains(QLatin1Char('\t'))) {  //若是项有快捷键，文本内容会以'\t'连接快捷键文本
                    if (!hideShortcutText)
                        m_width += tabSpacing;
                } else {
                    if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
                        m_width += 2 * Menu_ArrowHMargin;
                    }
                }

                int textWidth = opt->fontMetrics.size(Qt::TextSingleLine, menuItem->text).width();

                if (!menuItem->text.isEmpty())
                    m_width += (textWidth + frameRadius);

                if (!menuItem->icon.isNull())
                    m_width += (smallIconSize + frameRadius);

                m_width += (smallIconSize + frameRadius);
                size.setWidth(m_width);

                if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                    if (!menuItem->text.isEmpty()) {
                        size.setHeight(menuItem->fontMetrics.height());
                    } else {
                        size.setHeight(2);
                    }
                } else if (!menuItem->icon.isNull()) {
                    if (const QComboBox *combo = qobject_cast<const QComboBox *>(widget)) {
                        size.setHeight(qMax(combo->iconSize().height() + 2, size.height()));
                    }
                }
            }

            size.setWidth(qMax(162, size.width()));
            size.setHeight(size.height() + qMax(Menu_ItemVMargin * 2, 0));
            break;
        }
        case CT_ScrollBar: {
            if (size.width() > size.height())
                size.setHeight(ScrollBar_SliderWidth);
            if (size.width() < size.height())
                size.setWidth(ScrollBar_SliderWidth);
            return size;
        }
        case CT_RadioButton:
        case CT_CheckBox: {
            size.rwidth() += 2 * (DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing));
            // fix 当没有文字的时候高度不够绘制焦点圈
            auto button = qobject_cast<const QAbstractButton *>(widget);
            if (button && button->text().isEmpty())
                size.rheight() += 2 * (DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing));
            break;
        }
        case CT_ToolButton: {
            qreal radius = DStyle::pixelMetric(DStyle::PM_FrameRadius);
            if (widget && widget->property("_d_calendarToolBtn").toBool()) {
                return QSize(size.width() + radius, size.height() + radius);
            }
            int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, opt, widget);
            int menuButtonIndicatorWidth = 0;
            if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
                if ((toolbutton->features & QStyleOptionToolButton::HasMenu) &&
                    (!(toolbutton->features & QStyleOptionToolButton::MenuButtonPopup))) {
                    menuButtonIndicatorWidth = proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator, opt, widget);
                }
            }
            int menuButtonIndicatorMargin = 4;
            return QSize(size.width() + radius + 2 * menuButtonIndicatorMargin + 2 * fw + menuButtonIndicatorWidth,
                         size.height() + radius + 2 * fw);
        }
        case CT_ProgressBar: {
            if (const QStyleOptionProgressBar *pbo = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
                int radius = DStyle::pixelMetric(PM_FrameRadius);

                if (!pbo->textVisible) {
                    size.setWidth(qMax(size.width(), 2 * radius));
                    size.setHeight(qMax(size.height(), 2 * radius));
                } else {
                    QSize text_size = opt->fontMetrics.size(0, pbo->text.isEmpty() ? "%" : pbo->text);
                    size.setWidth(qMax(size.width(), 2 * radius + text_size.width()));
                    size.setHeight(qMax(size.height(), 2 * radius + text_size.height()));
                }
            }
            break;
        }
        case CT_SpinBox:
            if (auto vopt = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
                int spacing = widget && widget->property("_d_dtk_spinBox").toBool() ? 0 : DStyle::pixelMetric(PM_ContentsMargins);
                const int fw = vopt->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, vopt, widget) : 0;
                // 增加左右箭头对应的宽和焦点边框的边距,(正方形箭头宽 + 边距 + 箭头控件左右边距, 焦点边框的宽）.
                size += QSize((size.height() + spacing + fw * 2) * 2, LineEdit_FrameWidth);
                return size;
            }
            break;
        case CT_SizeGrip: {
            size = QSize(16, 16);
            break;
        }
        default:
            break;
    }

    return size;
}

}  // namespace chameleon
