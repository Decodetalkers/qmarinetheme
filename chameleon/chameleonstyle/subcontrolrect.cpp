#include "chameleonstyle.h"
#include "common.h"

#include <QComboBox>
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {

QRect ChameleonStyle::subControlRect(QStyle::ComplexControl cc,
                                     const QStyleOptionComplex *opt,
                                     QStyle::SubControl sc,
                                     const QWidget *w) const
{
    switch (cc) {
        case CC_SpinBox: {
            if (const QStyleOptionSpinBox *option = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
                switch (sc) {
                    case SC_SpinBoxEditField: {
                        if (option->buttonSymbols == QAbstractSpinBox::NoButtons)
                            return proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxFrame, w);

                        int rightBorder = option->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, opt, w) * 2 : 0;
                        int border = w && w->property("_d_dtk_spinBox").toBool() ? 0 : DStyle::pixelMetric(PM_ContentsMargins);
                        QRect spinboxRect = option->rect;
                        QRect dButtonRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, w);

                        spinboxRect.setRight(dButtonRect.left() - rightBorder - frameExtentMargins().left() - border);
                        return spinboxRect;
                    }
                    case SC_SpinBoxUp: {
                        if (w && w->property("_d_dtk_spinBox").toBool()) {
                            break;
                        }

                        if (option->buttonSymbols == QAbstractSpinBox::NoButtons)
                            return QRect();

                        int rectHeight = option->rect.height();
                        int border = w && w->property("_d_dtk_spinBox").toBool() ? 0 : DStyle::pixelMetric(PM_ContentsMargins);
                        rectHeight -= (option->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, opt, w) * 2 : 0);

                        if (opt->rect.width() < rectHeight * 3)
                            break;

                        QRect uButtonRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, w);
                        QRect buttonRect = option->rect;
                        buttonRect.setLeft(uButtonRect.left() - rectHeight - border);
                        buttonRect.setRight(uButtonRect.left());
                        buttonRect.setSize(QSize(rectHeight, rectHeight));
                        return buttonRect;
                    }
                    case SC_SpinBoxDown: {
                        if (w && w->property("_d_dtk_spinBox").toBool()) {
                            break;
                        }

                        if (option->buttonSymbols == QAbstractSpinBox::NoButtons)
                            return QRect();

                        int rectHeight = option->rect.height();
                        rectHeight -= (option->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, opt, w) * 2 : 0);

                        if (opt->rect.width() < rectHeight * 3)
                            break;

                        QRect buttonRect(option->rect.topLeft(), QSize(rectHeight, rectHeight));  //按高度计算
                        buttonRect.moveRight(option->rect.right());
                        return buttonRect;
                    }
                    case SC_SpinBoxFrame: {
                        return option->rect - frameExtentMargins();
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case CC_Slider: {
            if (const QStyleOptionSlider *option = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
                int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);
                QRectF rect = option->rect;                                                 // Slider控件总的大小矩形
                int slider_size = proxy()->pixelMetric(PM_SliderControlThickness, opt, w);  //滑块的高度
                //            int tick_size = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, w);         //刻度的高度
                QRectF slider_handle_rect = rect;  //滑块和滑漕的的最小公共矩形 (后面被用作临时且被改变的)

                if (option->orientation == Qt::Horizontal) {
                    slider_handle_rect.setHeight(slider_size);
                    slider_handle_rect.adjust(margin, 0, -margin, 0);
                    if (option->tickPosition == QSlider::TicksAbove)
                        slider_handle_rect.moveBottom(rect.bottom() - margin);
                    if (option->tickPosition == QSlider::TicksBelow)
                        slider_handle_rect.moveTop(rect.top() + margin);
                    if (option->tickPosition == QSlider::TicksBothSides || option->tickPosition == QSlider::NoTicks)
                        slider_handle_rect.moveCenter(rect.center());
                } else {
                    slider_handle_rect.setWidth(slider_size);
                    slider_handle_rect.adjust(0, margin, 0, -margin);
                    if (option->tickPosition == QSlider::TicksRight)
                        slider_handle_rect.moveLeft(rect.left() + margin);
                    if (option->tickPosition == QSlider::TicksLeft)
                        slider_handle_rect.moveRight(rect.right() - margin);
                    if (option->tickPosition == QSlider::TicksBothSides || option->tickPosition == QSlider::NoTicks)
                        slider_handle_rect.moveCenter(rect.center());
                }

                QRectF rectStatic = slider_handle_rect;  // rectStatic作为 滑块和滑漕的的最小公共矩形(不改变)

                switch (sc) {
                    case SC_SliderGroove: {  //滑漕
                        qreal groove_size = slider_size / 4.0;
                        QRectF groove_rect;

                        if (option->orientation == Qt::Horizontal) {
                            groove_rect.setWidth(slider_handle_rect.width());
                            groove_rect.setHeight(groove_size);
                        } else {
                            groove_rect.setWidth(groove_size);
                            groove_rect.setHeight(slider_handle_rect.height());
                        }

                        groove_rect.moveCenter(slider_handle_rect.center());
                        return groove_rect.toRect();
                    }
                    case SC_SliderHandle: {  //滑块
                        int sliderPos = 0;
                        int len = proxy()->pixelMetric(PM_SliderLength, option, w);
                        bool horizontal = option->orientation == Qt::Horizontal;
                        sliderPos =
                            sliderPositionFromValue(option->minimum,
                                                    option->maximum,
                                                    option->sliderPosition,
                                                    (horizontal ? slider_handle_rect.width() : slider_handle_rect.height()) - len,
                                                    option->upsideDown);
                        if (horizontal) {
                            slider_handle_rect.moveLeft(slider_handle_rect.left() + sliderPos);
                            slider_handle_rect.setWidth(len);
                            slider_handle_rect.moveTop(rectStatic.top());
                        } else {
                            slider_handle_rect.moveTop(slider_handle_rect.top() + sliderPos);
                            slider_handle_rect.setHeight(len);
                            slider_handle_rect.moveLeft(rectStatic.left());
                        }

                        return slider_handle_rect.toRect();
                    }
                    case SC_SliderTickmarks: {  //刻度的矩形
                        if (option->tickPosition & QSlider::NoTicks)
                            return QRect(0, 0, 0, 0);

                        QRectF tick_rect = rect;

                        if (option->orientation == Qt::Horizontal) {
                            tick_rect.setHeight(rect.height() - slider_handle_rect.height() - (2 * margin));
                            tick_rect.adjust(margin, 0, -margin, 0);

                            if (option->tickPosition == QSlider::TicksAbove) {
                                tick_rect.moveTop(rect.top() + margin);
                            } else if (option->tickPosition == QSlider::TicksBelow) {
                                tick_rect.moveBottom(rect.bottom() - margin);
                            }
                        } else {
                            tick_rect.setWidth(rect.width() - slider_handle_rect.width() - (2 * margin));
                            tick_rect.adjust(0, margin, 0, -margin);

                            if (option->tickPosition == QSlider::TicksLeft) {
                                tick_rect.moveLeft(rect.left() + margin);
                            } else if (option->tickPosition == QSlider::TicksRight) {
                                tick_rect.moveRight(rect.right() - margin);
                            }
                        }

                        return tick_rect.toRect();
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case CC_ComboBox: {
            if (qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                DStyleHelper dstyle(proxy());
                int frameMargins = dstyle.pixelMetric(PM_FrameMargins, opt, w);

                switch (sc) {
                    case SC_ComboBoxArrow: {
                        QRect rect(0,
                                   0,
                                   qMax(static_cast<int>(Metrics::ComboBox_ArrowButtonWidth), opt->rect.height()),
                                   opt->rect.height());
                        int boxHeight = qAbs(rect.height());

                        if (w && qobject_cast<const QComboBox *>(w) && !qobject_cast<const QComboBox *>(w)->isEditable())
                            break;
                        if (opt->direction == Qt::LeftToRight)
                            rect.moveRight(opt->rect.right());
                        else
                            rect.moveLeft(opt->rect.left());

                        int buttonRectSize = boxHeight;
                        rect.setSize(QSize(buttonRectSize, buttonRectSize));

                        return rect;
                    }
                    case SC_ComboBoxEditField: {
                        QRect rect = opt->rect;
                        const QRect arrow_rect = proxy()->subControlRect(cc, opt, SC_ComboBoxArrow, w);

                        if (opt->direction == Qt::LeftToRight) {
                            rect.setRight(arrow_rect.left());
                            rect.adjust(frameMargins, 0, 0, 0);
                        } else {
                            rect.setLeft(arrow_rect.right());
                            rect.adjust(0, 0, -frameMargins, 0);
                        }

                        return rect;
                    }
                    case SC_ComboBoxFrame: {
                        return opt->rect;
                    }
                    case SC_ComboBoxListBoxPopup: {
                        QSize size = proxy()->sizeFromContents(CT_Menu, opt, opt->rect.size(), w);
                        QRect rect = opt->rect;

                        if (auto styopt = static_cast<const QStyleOption *>(opt)) {
                            if (auto menuItem = static_cast<const QStyleOptionMenuItem *>(styopt)) {
                                if (menuItem->icon.isNull()) {
                                    rect.setWidth(size.width() + Menu_CheckMarkWidth);
                                }
                            }
                        }
                        return rect;
                    }
                    default:
                        break;
                }
            }
            break;
        }
        default:
            break;
    }
    return DStyle::subControlRect(cc, opt, sc, w);
}
}  // namespace chameleon
