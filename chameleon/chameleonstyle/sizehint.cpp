#include "chameleonstyle.h"

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

int ChameleonStyle::styleHint(QStyle::StyleHint sh, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *shret) const
{
    switch (sh) {
        case SH_ItemView_ShowDecorationSelected:
        case SH_ScrollBar_Transient:
            return true;
        //增加菜单鼠标事件跟踪
        case SH_MenuBar_MouseTracking:
        case SH_Menu_MouseTracking:
            return true;
        //增加TabBar超出范围的左右导航按钮
        case SH_TabBar_PreferNoArrows:
            return false;
        case SH_ComboBox_Popup:
            return true;
        case SH_ComboBox_PopupFrameStyle:
            return true;
        case SH_Slider_AbsoluteSetButtons:
            return Qt::LeftButton | Qt::MiddleButton;
        case SH_ToolTipLabel_Opacity:
            return 255;
        default:
            break;
    }

    return DStyle::styleHint(sh, opt, w, shret);
}

}  // namespace chameleon
