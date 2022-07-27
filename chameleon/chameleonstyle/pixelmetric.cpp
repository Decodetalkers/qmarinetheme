// redraw icons
#include "chameleonstyle.h"
#include "common.h"
#include <DGuiApplicationHelper>
#include <DIconButton>
#include <DListView>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>
#include <dtabbar.h>
#include <private/qcombobox_p.h>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {

int ChameleonStyle::pixelMetric(QStyle::PixelMetric m, const QStyleOption *opt, const QWidget *widget) const
{
    switch (static_cast<int>(m)) {
        case PM_SpinBoxSliderHeight:
            return SpinBox_MiniHeight;
        case PM_SpinBoxFrameWidth:
            return SpinBox_FrameWidth;
        case PM_TabCloseIndicatorWidth:
        case PM_TabCloseIndicatorHeight:
            // 平板需求
            return DGuiApplicationHelper::isTabletEnvironment() ? TabletTabBar_TabButtonSize : TabBar_TabButtonSize;
        case PM_TabBarTabVSpace:
        case PM_TabBarTabHSpace:
            return DStyle::pixelMetric(PM_FrameRadius, opt, widget) * 2;
        case PM_TabBarTabOverlap:
            return TabBar_TabOverlap;
        case PM_TabBarBaseOverlap:
            return TabBar_BaseOverlap;
        case PM_TabBarTabShiftHorizontal:
        case PM_TabBarTabShiftVertical:
            return 0;
        case PM_TabBarScrollButtonWidth:
            // DTabBar有自己的scroll按钮，需要隐藏QTabBar的按钮
            if (widget && qobject_cast<DTabBar *>(widget->parent()))
                return 0;

            return DStyle::pixelMetric(PM_ButtonMinimizedSize, opt, widget);
        case PM_MenuScrollerHeight:
            return 10 + Metrics::Frame_FrameWidth;
        case PM_MenuPanelWidth:
            //非特效效果则 menu border 为 1
            return DWindowManagerHelper::instance()->hasComposite() ? 0 : 1;
        case PM_SubMenuOverlap:
            return 0;
        case PM_ComboBoxFrameWidth: {  //这是ComboBox VMargin
            const QStyleOptionComboBox *comboBoxOption(qstyleoption_cast<const QStyleOptionComboBox *>(opt));
            return comboBoxOption && comboBoxOption->editable ? Metrics::ComboBox_FrameWidth : Metrics::LineEdit_FrameWidth;
        }
        case PM_MenuVMargin:
            return 8;
        case PM_MenuHMargin:
            return 0;
        default:
            break;
    }

    return DStyle::pixelMetric(m, opt, widget);
}
}  // namespace chameleon
