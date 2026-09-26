#include <assert.h>
#include "menu_options_model.h"
int main(void) {
    MenuOptionsModel m={{0,0,0,2,2,0},0,0};
    MenuOptionsModel original=m;
    MenuOptions_Change(&m,-1); assert(m.values[0]==1);
    MenuOptions_Change(&m,1); assert(m.values[0]==0);
    m.column=1; m.row=0; MenuOptions_Change(&m,1); assert(m.values[3]==3);
    MenuOptions_Change(&m,1); assert(m.values[3]==0);
    m.row=1; m.values[4]=6; MenuOptions_Change(&m,1); assert(m.values[4]==0);
    MenuOptions_Change(&m,-1); assert(m.values[4]==6);
    m.row=2; MenuOptions_Change(&m,-1); assert(m.values[5]==3);
    m.row=3; MenuOptions_Change(&m,1); assert(m.values[6]==1);
    MenuOptions_Change(&m,-1); assert(m.values[6]==0);
    MenuOptions_Change(&m,-1); assert(m.values[6]==2);
    m.row=4; MenuOptions_Change(&m,1); assert(m.values[7]==1);
    m.row=5; MenuOptions_Change(&m,1); assert(m.values[8]==1);
    MenuOptions_Move(&m,1); assert(m.row==MENU_OPTIONS_FOOTER);
    MenuOptions_Change(&m,1); assert(m.values[8]==1);
    MenuOptions_Move(&m,-1); assert(m.row==5);
    MenuOptions_Column(&m,0); assert(m.row==2 && m.column==0);
    MenuOptions_Move(&m,1); assert(m.row==MENU_OPTIONS_FOOTER);
    MenuOptions_Column(&m,1); assert(m.row==MENU_OPTIONS_FOOTER);
    MenuOptions_Move(&m,1); assert(m.row==0);
    MenuOptions_Move(&m,-1); assert(m.row==MENU_OPTIONS_FOOTER);
    assert(original.values[3]==2 && original.values[4]==2 && original.values[5]==0);
    assert(MenuOptions_Distance(0)==75 && MenuOptions_Distance(2)==100 && MenuOptions_Distance(6)==150);
    return 0;
}
