#pragma once
/* Emulators forward a full-screen touch as 320x240 HID coordinates.
 * The unified presentation uses 400x240 logical coordinates. */
static inline int MenuTouch_X(int x) { return x*5/4; }
static inline int MenuTouch_In(int x,int y,int l,int t,int w,int h) {
    return x>=l && x<l+w && y>=t && y<t+h;
}
static inline int MenuTouch_Tab(int x,int y) {
    for (int i=0;i<3;++i)
        if (MenuTouch_In(x,y,12+i*56,9,50,22)) return i+1;
    return 0;
}
static inline int MenuTouch_Footer(int x,int y) {
    if (MenuTouch_In(x,y,45,209,70,20)) return 1;
    if (MenuTouch_In(x,y,285,209,70,20)) return 2;
    return 0;
}
/* Inverse of Save's 140x105 viewport at (130,43). Restrict input
 * to the two visible choices, never hidden Back/Options hit targets. */
static inline int MenuTouch_Save(int x,int y,int* nx,int* ny) {
    *nx=(x-130)*320/140; *ny=(y-43)*240/105;
    return MenuTouch_In(*nx,*ny,39,96,120,48) ||
           MenuTouch_In(*nx,*ny,164,96,120,48);
}
