#pragma once
/* Draft-only editing: callers apply values on OK and discard on Cancel. */
typedef struct { unsigned char values[9], column, row; } MenuOptionsModel;
#define MENU_OPTIONS_FOOTER 6
static inline unsigned MenuOptions_Rows(unsigned column) { return column ? 6 : 3; }
static inline void MenuOptions_Column(MenuOptionsModel* m,unsigned column) {
    m->column=column ? 1 : 0;
    if (m->row!=MENU_OPTIONS_FOOTER && m->row>=MenuOptions_Rows(m->column)) m->row=2;
}
static inline void MenuOptions_Move(MenuOptionsModel* m,int direction) {
    unsigned count=MenuOptions_Rows(m->column);
    if (m->row==MENU_OPTIONS_FOOTER) m->row=direction>0 ? 0 : count-1;
    else if (direction>0) m->row=m->row+1<count ? m->row+1 : MENU_OPTIONS_FOOTER;
    else m->row=m->row ? m->row-1 : MENU_OPTIONS_FOOTER;
}
static inline void MenuOptions_Change(MenuOptionsModel* m, int direction) {
    static const unsigned char counts[9]={2,2,2,4,7,4,3,3,3};
    if (m->row>=MenuOptions_Rows(m->column)) return;
    unsigned i=m->column*3+m->row, n=counts[i];
    m->values[i]=(m->values[i]+n+direction)%n;
}
static inline unsigned MenuOptions_Distance(unsigned index) { return 75+index*125/10; }
