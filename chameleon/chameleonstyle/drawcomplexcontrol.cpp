#include "chameleonstyle.h"
#include <DSearchEdit>
#include <DTabBar>
#include <DGuiApplicationHelper>
#include <DSlider>
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {

void ChameleonStyle::drawComplexControl(QStyle::ComplexControl cc,
                                        const QStyleOptionComplex *opt,
                                        QPainter *p,
                                        const QWidget *w) const
{
    switch (cc) {
        case CC_SpinBox: {
            if (const QStyleOptionSpinBox *option = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
                if (drawSpinBox(option, p, w))
                    return;
            }
            break;
        }
        case CC_ToolButton: {
            if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
                QRect button, menuarea;
                button = proxy()->subControlRect(cc, toolbutton, SC_ToolButton, w);
                menuarea = proxy()->subControlRect(cc, toolbutton, SC_ToolButtonMenu, w);

                State bflags = toolbutton->state & ~State_Sunken;

                if (bflags & State_AutoRaise) {
                    if (!(bflags & State_MouseOver) || !(bflags & State_Enabled)) {
                        bflags &= ~State_Raised;
                    }
                }
                State mflags = bflags;
                if (toolbutton->state & State_Sunken) {
                    if (toolbutton->activeSubControls & SC_ToolButton)
                        bflags |= State_Sunken;
                    mflags |= State_Sunken;
                }

                int menuButtonIndicatorMargin = 4;  // 菜单按钮右边距
                QStyleOption tool = *toolbutton;
                if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup) {
                    if (w && !w->property("_d_calendarToolBtn").toBool()) {
                        button.adjust(0, 0, -2 * menuButtonIndicatorMargin, 0);
                        menuarea.adjust(-2 * menuButtonIndicatorMargin, 0, 0, 0);
                    }
                    if (bflags & (State_Sunken | State_On | State_Raised)) {
                        tool.rect = button;
                        tool.state = bflags;
                        // proxy()->drawPrimitive(PE_PanelButtonTool, &tool, p, w);
                    }
                }

                QStyleOptionToolButton label = *toolbutton;
                label.state = bflags;
                int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, opt, w);
                label.rect = button.adjusted(fw, fw, -fw, -fw);

                if (w && w->property("_d_calendarToolBtn").toBool()) {
                    label.palette = DGuiApplicationHelper::instance()->applicationPalette();
                }
                proxy()->drawControl(CE_ToolButtonLabel, &label, p, w);

                if (toolbutton->state & State_HasFocus) {
                    QStyleOptionFocusRect fr;
                    fr.QStyleOption::operator=(*toolbutton);
                    // fr.rect.adjust(3, 3, -3, -3);
                    if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)
                        fr.rect.adjust(0,
                                       0,
                                       -proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator, toolbutton, w) -
                                           2 * menuButtonIndicatorMargin,
                                       0);
                    proxy()->drawPrimitive(PE_FrameFocusRect, &fr, p, w);
                }

                if (toolbutton->subControls & SC_ToolButtonMenu) {
                    tool.rect = menuarea;
                    tool.state = mflags;
                    tool.rect.adjust(menuButtonIndicatorMargin, 0, -menuButtonIndicatorMargin, 0);
                    if ((toolbutton->state & (QStyle::State_Sunken)) && (toolbutton->activeSubControls & QStyle::SC_ToolButton)) {
                        p->setPen(Qt::NoPen);
                    }
                    if ((toolbutton->state & (QStyle::State_Sunken)) &&
                        toolbutton->activeSubControls & QStyle::SC_ToolButtonMenu) {
                        p->setPen(getColor(toolbutton, DPalette::Highlight));
                    }
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &tool, p, w);
                } else if (toolbutton->features & QStyleOptionToolButton::HasMenu) {
                    int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w);
                    QRect ir = toolbutton->rect;
                    QStyleOptionToolButton newBtn = *toolbutton;
                    newBtn.rect = QRect(ir.right() - mbi - menuButtonIndicatorMargin, (ir.height() - mbi) / 2, mbi, mbi);
                    newBtn.rect = visualRect(toolbutton->direction, button, newBtn.rect);

                    // DelayedPopup 模式，箭头右居中, 加一个日历 月按钮箭头居中
                    if (w && w->objectName() == "qt_calendar_monthbutton") {
                        newBtn.rect = QRect(ir.right() + 5 - mbi, ir.y() + ir.height() / 2, mbi - 4, mbi - 4);
                        newBtn.rect = visualRect(toolbutton->direction, button, newBtn.rect);
                    }
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &newBtn, p, w);
                }

                //日历　年按钮 特制
                if (w && w->objectName() == "qt_calendar_yearbutton") {
                    QStyleOptionToolButton newBtn = *toolbutton;
                    int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w);
                    QRect ir = toolbutton->rect;

                    newBtn.rect = QRect(ir.right() + 5 - mbi, ir.y() + ir.height() / 2, mbi - 4, mbi - 4);
                    newBtn.rect = visualRect(toolbutton->direction, button, newBtn.rect);
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &newBtn, p, w);
                }
            }
            return;
        }
        case CC_Slider: {
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
                const DSlider *dslider = qobject_cast<const DSlider *>(w);
                //各个使用的矩形大小和位置
                QRectF rect = opt->rect;  // Slider控件最大的矩形(包含如下三个)
                QRectF rectHandle = proxy()->subControlRect(CC_Slider, opt, SC_SliderHandle, w);              //滑块矩形
                QRectF rectSliderTickmarks = proxy()->subControlRect(CC_Slider, opt, SC_SliderTickmarks, w);  //刻度的矩形
                QRect rectGroove = proxy()->subControlRect(CC_Slider, opt, SC_SliderGroove, w);               //滑槽的矩形
                int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);

                //            //测试(保留不删)
                //            p->fillRect(rect, Qt::gray);
                //            p->fillRect(rectSliderTickmarks, Qt::blue);
                //            p->fillRect(rectGroove, Qt::red);
                //            p->fillRect(rectHandle, Qt::green);
                //            qDebug()<<"---rect:"<<rect<<"  rectHandle:"<<rectHandle<<"
                //            rectSliderTickmarks:"<<rectSliderTickmarks<<"   rectGroove:"<<rectGroove;

                QPen pen;
                //绘画 滑槽(线)
                if (opt->subControls & SC_SliderGroove) {
                    pen.setStyle(Qt::CustomDashLine);
                    pen.setWidth(4);
                    // 默认高亮色滑槽颜色
                    pen.setBrush((opt->activeSubControls & SC_SliderHandle) ? getColor(opt, QPalette::Highlight) :
                                                                              opt->palette.highlight());
                    pen.setDashOffset(0);
                    pen.setDashPattern(QVector<qreal>() << 0.5 << 0.25);
                    pen.setCapStyle(Qt::FlatCap);
                    p->setPen(pen);
                    p->setRenderHint(QPainter::Antialiasing);

                    QColor color = getColor(opt, DPalette::ObviousBackground, w);  //绘画的右侧/上侧的滑槽颜色一定是灰

                    // 属性启用灰色滑槽
                    QVariant prop =
                        dslider ? const_cast<DSlider *>(dslider)->slider()->property("_d_dtk_sldier_across") : QVariant();
                    bool hasProperty = prop.isValid();
                    // 0. dslider 默认没有设置此属性(设置在d->slider上了...)
                    // 1. 设置了属性 true 则灰色滑槽
                    // 2. 如果设置了 false 高亮
                    // 3. 没有设置属性时，没有刻度的（圆角） slider 默认高亮色，有刻度(尖角) slider 默认灰色
                    if (prop.toBool() || (!hasProperty && !isNoticks(slider, p, w))) {
                        pen.setColor(color);
                        p->setPen(pen);
                    }

                    if (slider->orientation == Qt::Horizontal) {
                        // 绘制最左边到滑块的位置的滑槽
                        qreal rectWidth = rectHandle.width() / 2.0;
                        p->drawLine(QPointF(rectGroove.left() + rectWidth, rectHandle.center().y()),
                                    QPointF(rectHandle.center().x(), rectHandle.center().y()));
                        // 绘制滑块到最右的位置的滑槽
                        pen.setColor(color);
                        p->setPen(pen);
                        p->drawLine(QPointF(rectGroove.right() - rectWidth, rectHandle.center().y()),
                                    QPointF(rectHandle.center().x(), rectHandle.center().y()));

                    } else {
                        // 绘制最上边到滑块的位置的滑槽
                        qreal rectWidth = rectHandle.height() / 2.0;
                        p->drawLine(QPointF(rectGroove.center().x(), rectGroove.bottom() - rectWidth),
                                    QPointF(rectGroove.center().x(), rectHandle.center().y()));
                        // 绘制滑块到最下的位置的滑槽
                        pen.setColor(color);
                        p->setPen(pen);
                        p->drawLine(QPointF(rectGroove.center().x(), rectGroove.top() + rectWidth),
                                    QPointF(rectGroove.center().x(), rectHandle.center().y()));
                    }
                }

                //绘画 滑块
                if (opt->subControls & SC_SliderHandle) {
                    pen.setStyle(Qt::SolidLine);
                    p->setPen(Qt::NoPen);
                    p->setBrush((opt->activeSubControls & SC_SliderHandle) ? getColor(opt, QPalette::Highlight) :
                                                                             opt->palette.highlight());
                    drawSliderHandle(opt, rectHandle, p, w);

                    // 绘画 滑块焦点
                    if (slider->state & State_HasFocus) {
                        pen.setColor(getColor(opt, DPalette::Highlight));
                        pen.setWidth(DStyle::pixelMetric(PM_FocusBorderWidth));
                        p->setPen(pen);
                        p->setBrush(Qt::NoBrush);
                        drawSliderHandleFocus(opt, rectHandle, p, w);
                    }
                }

                //绘画 刻度,绘画方式了参考qfusionstyle.cpp
                if ((opt->subControls & SC_SliderTickmarks) && slider->tickInterval) {  //需要绘画刻度
                    p->setPen(opt->palette.windowText().color());
                    int available = proxy()->pixelMetric(PM_SliderSpaceAvailable, slider, w);  //可用空间
                    int interval = slider->tickInterval;                                       //标记间隔
                    //                int tickSize = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, w);      //标记偏移
                    //                int ticks = slider->tickPosition;                                          //标记位置

                    int v = slider->minimum;
                    int len = proxy()->pixelMetric(PM_SliderLength, slider, w);
                    while (v <= slider->maximum + 1) {  //此处不添加+1的话, 会少绘画一根线
                        const int v_ = qMin(v, slider->maximum);
                        int pos = margin +
                                  sliderPositionFromValue(slider->minimum, slider->maximum, v_, available - (2 * margin)) +
                                  len / 2;

                        if (slider->orientation == Qt::Horizontal) {
                            if (slider->tickPosition ==
                                QSlider::TicksBothSides) {  //两侧都会绘画, 总的矩形-中心滑槽滑块最小公共矩形
                                p->drawLine(pos, rect.top(), pos, rectHandle.top());
                                p->drawLine(pos, rect.bottom(), pos, rectHandle.bottom());
                            } else {
                                p->drawLine(pos, rectSliderTickmarks.top(), pos, rectSliderTickmarks.bottom());
                            }
                        } else {
                            if (slider->tickPosition == QSlider::TicksBothSides) {
                                p->drawLine(rect.left(), pos, rectHandle.left(), pos);
                                p->drawLine(rect.right(), pos, rectHandle.right(), pos);
                            } else {
                                p->drawLine(rectSliderTickmarks.left(), pos, rectSliderTickmarks.right(), pos);
                            }
                        }
                        // in the case where maximum is max int
                        int nextInterval = v + interval;
                        if (nextInterval < v)
                            break;
                        v = nextInterval;
                    }
                }
            }
            break;
        }
        case CC_ComboBox: {
            if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                if (drawComboBox(p, comboBox, w))
                    return;
            }
            break;
        }
        default:
            break;
    }

    DStyle::drawComplexControl(cc, opt, p, w);
}
}
