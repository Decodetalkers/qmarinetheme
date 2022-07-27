#include "chameleonstyle.h"
#include <DSearchEdit>
#include <DTabBar>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {
QRect ChameleonStyle::subElementRect(QStyle::SubElement r, const QStyleOption *opt, const QWidget *widget) const
{
    switch (r) {
        case SE_HeaderArrow: {
            QRect rect;
            int h = opt->rect.height();
            int w = opt->rect.width();
            int x = opt->rect.x();
            int y = opt->rect.y();
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, opt, widget);

            if (opt->state & State_Horizontal) {
                // designer: whatever how big the QHeaderView it is, the arrow size is fixed.
                // size the same as the arrow in combobox. PM_MenuButtonIndicator
                int horiz_size = 12;
                int vert_size = 12;
                rect.setRect(x + w - margin * 2 - horiz_size, y + (h - vert_size) / 2, horiz_size, vert_size);
            } else {
                int horiz_size = 12;
                int vert_size = 12;
                rect.setRect(x + (w - horiz_size) / 2, y + h - margin * 2 - vert_size, horiz_size, vert_size);
            }
            rect = visualRect(opt->direction, opt->rect, rect);
            return rect;
        }
        case SE_PushButtonFocusRect:
        case SE_ItemViewItemFocusRect:
            return opt->rect;
        case SE_ItemViewItemCheckIndicator:
            Q_FALLTHROUGH();
        case SE_ItemViewItemDecoration:
            Q_FALLTHROUGH();
        case SE_ItemViewItemText:
            if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, widget);
                QStyleOptionViewItem option(*vopt);
                option.rect = opt->rect.adjusted(frame_radius, 0, -frame_radius, 0);

                QStyleOptionViewItem::ViewItemFeatures old_features = option.features;

                // 默认把checkbox放置在右边，因此使用QCommonStyle的Item布局时先移除HasCheckIndicator标志
                option.features &= ~QStyleOptionViewItem::HasCheckIndicator;

                if (r == SE_ItemViewItemDecoration) {
                    return DStyle::subElementRect(r, &option, widget);
                }

                QRect text_rect = DStyle::subElementRect(SE_ItemViewItemText, &option, widget);
                int indicator_width = proxy()->pixelMetric(PM_IndicatorWidth, &option, widget);
                int indicator_height = proxy()->pixelMetric(PM_IndicatorHeight, &option, widget);

                const QRect indicator_rect = alignedRect(
                    opt->direction, Qt::AlignRight | Qt::AlignVCenter, QSize(indicator_width, indicator_height), text_rect);

                if (old_features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
                    int margin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, opt, widget);
                    text_rect.setRight(qMin(text_rect.right(), indicator_rect.left() - margin));
                }

                return r == SE_ItemViewItemText ? text_rect : indicator_rect;
            }
            break;
        case SE_LineEditContents: {
            int frame_margins = DStyle::pixelMetric(PM_FrameMargins, opt, widget);
            int left_margins = DStyle::pixelMetric(PM_ContentsMargins, opt, widget);
            if (widget && qobject_cast<DSearchEdit *>(widget->parentWidget())) {
                return opt->rect.adjusted(frame_margins / 2, 0, -left_margins / 2, 0);
            }

            return opt->rect.adjusted(frame_margins + left_margins, 0, -(frame_margins + left_margins), 0);
        }
        case SE_RadioButtonFocusRect:
        case SE_CheckBoxFocusRect: {
            QRect re;
            re = subElementRect(SE_CheckBoxIndicator, opt, widget);
            int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);
            re.adjust(-margin, -margin, margin, margin);
            return re;
        }
        case SE_RadioButtonClickRect:
        case SE_CheckBoxClickRect: {
            QRect re = DStyle::subElementRect(SE_CheckBoxIndicator, opt, widget);
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                int spacing = proxy()->pixelMetric(PM_CheckBoxLabelSpacing, opt, widget);
                re.setWidth(re.width() + widget->fontMetrics().horizontalAdvance(btn->text) + spacing * 2);
            }
            return re;
        }
        case SE_RadioButtonIndicator:
        case SE_RadioButtonContents:
        case SE_CheckBoxContents:
        case SE_CheckBoxIndicator:
            if (const QStyleOptionButton *vopt = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                QStyleOptionButton option(*vopt);
                int margin = DStyle::pixelMetric(PM_FocusBorderWidth) +
                             DStyle::pixelMetric(PM_FocusBorderSpacing);  //来自SE_CheckBoxFocusRect状态时
                option.rect.translate(margin, 0);  //需往右偏margin数值，FocusRect框显示正常；故对应其size的width也增加margin
                return DStyle::subElementRect(r, &option, widget);
            }
            break;
        case SE_PushButtonContents:
            if (const QStyleOptionButton *vopt = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                QRect buttonContentRect = vopt->rect;
                int buttonIconMargin = proxy()->pixelMetric(QStyle::PM_ButtonMargin, opt, widget);
                buttonContentRect.adjust(buttonIconMargin, 0, -buttonIconMargin, 0);

                return buttonContentRect;
            }
            break;
        case SE_TabBarScrollLeftButton: {
            const bool vertical = opt->rect.width() < opt->rect.height();
            const int buttonWidth = proxy()->pixelMetric(PM_TabBarScrollButtonWidth, opt, widget);
            QRect buttonRect =
                vertical ? QRect(0, 0, opt->rect.width(), buttonWidth) : QRect(0, 0, buttonWidth, opt->rect.height());

            return buttonRect;
        }
        case SE_TabBarScrollRightButton: {
            const bool vertical = opt->rect.width() < opt->rect.height();
            const int buttonWidth = proxy()->pixelMetric(PM_TabBarScrollButtonWidth, opt, widget);
            QRect buttonRect = vertical ? QRect(0, opt->rect.height() - buttonWidth, opt->rect.width(), buttonWidth) :
                                          QRect(opt->rect.width() - buttonWidth, 0, buttonWidth, opt->rect.height());

            return buttonRect;
        }
        case SE_ProgressBarLabel: {
            int radius = DStyle::pixelMetric(PM_FrameRadius);
            return opt->rect.marginsRemoved(QMargins(radius, radius, radius, radius));
        }
        case SE_TabBarTearIndicatorLeft:
        case SE_TabBarTearIndicatorRight:
            // DTabBar有自己的scroll按钮
            if (widget && qobject_cast<DTabBar *>(widget->parent()))
                return QRect(0, 0, 0, 0);
            break;
        default:
            break;
    }

    return DStyle::subElementRect(r, opt, widget);
}
}  // namespace chameleon
