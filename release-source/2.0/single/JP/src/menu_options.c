#include "menu_options.h"
#include "menu_options_model.h"
#include "camera.h"
#include "native_hud.h"
#include "hid.h"
#include "menu_touch.h"
#include "movement_options.h"
#include "menu_preferences.h"
#include "memory_probe.h"
#include "draw.h"
#if defined(Version_JP)
#include "fonts/jp_options_font.h"
#endif

#if UNIFIED_MENU_PROTOTYPE && (defined(Version_USA) || defined(Version_EUR) || defined(Version_JP))
static MenuOptionsModel sDraft;
static u8 sOpen, sNativeExit, sRightPage, sSaveFailed;
static const char* sBuildingText;
static const char* const sWords[]={
    "L-TARGETING", "FIRST PERSON CAMERA", "MOTION CONTROL",
    "HUD SIZE", "FREE-CAMERA DISTANCE", "FREE CAMERA",
    "Hold", "Switch", "Normal", "Inverted", "On", "Off",
    "Hidden", "75%", "100%", "125%",
    "75%", "87.5%", "100%", "112.5%", "125%", "137.5%", "150%",
    "Normal", "Invert X", "Invert Y", "Invert Both",
    "Hold or press the targeting control.",
    "Vertical controls for first-person looking and aiming.",
    "Use motion controls for first-person looking and aiming.",
    "Size of the gameplay HUD. Does not resize these menus.",
    "Exploration free camera only. Walls still limit distance.",
    "Right-stick exploration camera. Does not change the Gear doll.",
    "Cancel discards changes. OK applies them.",
    "Up / Down: scroll    Left / Right: adjust",
    "<", ">", "< L   OCARINA", "REFRAMED   R >",
    "MOVEMENT SPEED", "ROLL SPEED", "ROLL RECOVERY",
    "Normal", "+10%", "+20%", "Normal", "-10%", "-20%",
    "Walking and running speed. Other actions stay normal.",
    "Forward roll travel speed. Also increases roll distance.",
    "Shortens the final recovery window of a forward roll.",
    "1/2 v", "2/2 ^",
    "Could not save options. Retry OK or Cancel."
};
#define WORD_COUNT (sizeof(sWords)/sizeof(sWords[0]))
static u32 sText[WORD_COUNT][0x50/4];
static u8 sTextReady[WORD_COUNT];
static const u8 sLabels[]={0,1,2,3,4,5,39,40,41};
static const u8 sValueBase[]={6,8,10,12,16,23,42,42,45};
static const u8 sHelp[]={27,28,29,30,31,32,48,49,50};
static char sEncoded[256];

static u32 HelpWord(void) {
    return sSaveFailed ? 53 : sDraft.row<MENU_OPTIONS_FOOTER ?
        sHelp[sDraft.column*3+sDraft.row] : 33;
}

static void ReleaseText(u32 id) {
    if (!sTextReady[id]) return;
    /* Native menu replacement calls this destructor, then separately frees
     * its object. Our object storage is static: destroy contents only. The
     * destructor synchronizes rendering and unregisters its two nodes. */
    ((void(*)(void*))ADDR(0x002F6944))(sText[id]);
    for (u32 n=0;n<0x50/4;++n) sText[id][n]=0;
    sTextReady[id]=0;
}

static void PrepareText(void) {
    u8 needed[WORD_COUNT]={0};
    needed[34]=needed[35]=needed[36]=needed[37]=needed[38]=1;
    needed[51+sRightPage]=needed[HelpWord()]=1;
    for (u32 col=0;col<2;++col) for (u32 row=0;row<3;++row) {
        u32 i=col*3+row+(col ? sRightPage*3 : 0);
        needed[sLabels[i]]=needed[sValueBase[i]+sDraft.values[i]]=1;
    }
    /* Retire obsolete words before allocating replacements, not afterward.
     * At most19 unique labels are resident instead of all54. */
    for (u32 i=0;i<WORD_COUNT;++i) if (!needed[i]) ReleaseText(i);
    for (u32 i=0;i<WORD_COUNT;++i) if (needed[i] && !sTextReady[i]) {
        sBuildingText=sWords[i];
        ((void(*)(void*,u32,int,int,int))ADDR(0x002F57F0))(sText[i],0x94f,0,0,0);
        sBuildingText=0; sTextReady[i]=1;
        MemoryProbe_Log("visible-text",i);
    }
}

/* Only override the message lookup made while constructing our private
 * text objects. All ordinary messages use the unchanged native lookup. */
u32 MenuOptions_Lookup(void* context,u32 id,u32* record) {
    if (!sBuildingText)
        return ((u32(*)(void*,u32,u32*))ADDR(0x0044D548))(context,id,record);
    u32 length=0;
    while (sBuildingText[length] && length<250) {
        sEncoded[length]=sBuildingText[length]; ++length;
    }
    sEncoded[length++]=0x7f; sEncoded[length++]=0;
    for (u32 i=0;i<24;++i) record[i]=0;
    record[0]=id;
    for (u32 i=0;i<10;++i) {
        record[4+i*2]=(u32)sEncoded; record[5+i*2]=length;
    }
    return 1;
}

void MenuOptions_Begin(void) {
    const volatile u32* native=(const volatile u32*)0x0055BE90;
    for (u32 i=0;i<3;++i) sDraft.values[i]=native[i]&1;
    u8 hud=NativeHud_GetScalePercent();
    sDraft.values[3]=hud==0 ? 0 : hud==75 ? 1 : hud==125 ? 3 : 2;
    sDraft.values[4]=Camera_GetDistanceOption();
    sDraft.values[5]=Camera_GetControlMode();
    for (u32 i=0;i<3;++i) sDraft.values[6+i]=MovementOptions_Get(i);
    sDraft.row=sDraft.column=sRightPage=sSaveFailed=0; sOpen=1;
    MemoryProbe_Log("options-before",WORD_COUNT);
}
void MenuOptions_End(void) {
    sOpen=0;
    for (u32 i=0;i<WORD_COUNT;++i) ReleaseText(i);
    MemoryProbe_Log("options-end",0);
}

static void SyncSelection(void) {
    if (sDraft.column && sDraft.row<MENU_OPTIONS_FOOTER) sRightPage=sDraft.row/3;
    ((volatile u32*)0x0050A508)[0x30/4]=sDraft.row==MENU_OPTIONS_FOOTER ?
        3+sDraft.column : sDraft.row%3;
}

int MenuOptions_Touch(int x,int y) {
    if (MenuTouch_In(x,y,132,132,43,16)) return 0;
    if (MenuTouch_In(x,y,226,132,43,16)) return 1;
    if (MenuTouch_In(x,y,338,47,32,16)) {
        sRightPage^=1; sDraft.column=1; sDraft.row=sRightPage*3;
        SyncSelection(); return -1;
    }
    for (int col=0;col<2;++col) {
        const int left=col ? 207 : 30;
        if (MenuTouch_In(x,y,left,47,163,16)) MenuOptions_Column(&sDraft,col);
        for (int row=0;row<3;++row) {
            if (!MenuTouch_In(x,y,left+2,63+row*21,159,21)) continue;
            sDraft.column=col; sDraft.row=row+(col ? sRightPage*3 : 0);
            if (x<left+27) MenuOptions_Change(&sDraft,-1);
            else if (x>=left+135) MenuOptions_Change(&sDraft,1);
        }
    }
    SyncSelection();
    return -1;
}

int MenuOptions_Update(u32 pressed) {
    if (pressed&BUTTON_B) return 0;
    if (pressed&BUTTON_L1) MenuOptions_Column(&sDraft,0);
    else if (pressed&BUTTON_R1) MenuOptions_Column(&sDraft,1);
    if (pressed&(BUTTON_UP|CPAD_UP)) MenuOptions_Move(&sDraft,-1);
    else if (pressed&(BUTTON_DOWN|CPAD_DOWN)) MenuOptions_Move(&sDraft,1);
    int direction=(pressed&(BUTTON_LEFT|CPAD_LEFT)) ? -1 :
                  (pressed&(BUTTON_RIGHT|CPAD_RIGHT)) ? 1 : 0;
    if (sDraft.row==MENU_OPTIONS_FOOTER) {
        if (direction) sDraft.column^=1;
        if (pressed&BUTTON_A) return sDraft.column;
    } else if (direction || (pressed&BUTTON_A)) {
        MenuOptions_Change(&sDraft,direction ? direction : 1);
    }
    /* Keep native button selection synchronized for its own decorations. */
    SyncSelection();
    return -1;
}

u32 MenuOptions_NativeButtons(void) { return sNativeExit ? BUTTON_A : 0; }
void MenuOptions_ExitNative(int accept) {
    if (accept) {
        /* Commit and verify the sidecar before applying or leaving Options.
         * A failed write must remain visible, not silently become session-only. */
        if (!MenuPreferences_Save(sDraft.values)) { sSaveFailed=1; return; }
        sSaveFailed=0;
        volatile u32* native=(volatile u32*)0x0055BE90;
        for (u32 i=0;i<3;++i) native[i]=sDraft.values[i];
        volatile u8* settings=(volatile u8*)0x00587958;
        settings[0x2d]=sDraft.values[0]; settings[0x13d8]=sDraft.values[1];
        settings[0xf]=sDraft.values[2];
        static const u8 hud[]={0,75,100,125};
        NativeHud_SetScalePercent(hud[sDraft.values[3]]);
        Camera_SetDistanceOption(sDraft.values[4]);
        Camera_SetControlMode(sDraft.values[5]);
        for (u32 i=0;i<3;++i) MovementOptions_Set(i,sDraft.values[6+i]);
    }
    volatile u32* save=(volatile u32*)0x0050A508;
    save[0x30/4]=accept ? 4 : 3; save[0x3c/4]=2;
    sNativeExit=1;
    ((void(*)(void))ADDR(0x0043AE94))();
    sNativeExit=0;
}

static void Text(u32 id,float x,float y,float scale,u8 selected) {
    if (!sTextReady[id]) return;
    u32* text=sText[id];
    for (u32 i=0;i<2;++i) {
        u8* node=(u8*)text[0x10/4+i];
        if (!node) return;
        *(float*)(node+0x3c)=x+(i ? 0 : 0.6f);
        *(float*)(node+0x40)=y+(i ? 0 : 0.6f);
        *(float*)(node+0x48)=*(float*)(node+0x4c)=scale;
        /* Constructor blackens resource zero: shadow then foreground. */
        *(float*)(node+0xf0)=i ? 1 : 0;
        *(float*)(node+0xf4)=i ? (selected ? 0.83f : 1) : 0;
        *(float*)(node+0xf8)=i ? (selected ? 0.25f : 1) : 0;
        *(float*)(node+0xfc)=1;
    }
    ((void(*)(void*))ADDR(0x002F7684))(text);
    for (u32 i=0;i<2;++i) {
        void* node=(void*)text[0x10/4+i];
        ((void(*)(void*))(*(u32*)(*(u32*)node+12)))(node);
    }
}

/* Two immutable native atlas boards: one per active column. Geometry and
 * colors are never rewritten while a previous GPU draw may still use them. */
static void DrawPanels(void) {
    static float positions[2][78*12] __attribute__((aligned(16)));
    static float uvs[2][78*8] __attribute__((aligned(16)));
    static float colors[2][78*16] __attribute__((aligned(16)));
    static u16 indices[78*6];
    static u32 resources[2][0x1b8/4] __attribute__((aligned(16)));
    static void* nodes[2];
    const u32 active=sDraft.column;
    if (!nodes[active]) {
        const u16 order[]={0,2,1,1,2,3};
        for (u32 q=0;q<78;++q) {
            const u32 col=q/8, part=q%8;
            const u8 visible=q<21, selected=col==active;
            float l=col ? 207 : 30, t=47, w=163, h=83;
            /* Stone slab, four slim bevels, header band and row dividers. */
            if (part==1) h=1.5f;
            if (part==2) w=1.5f;
            if (part==3) { l+=161.5f; w=1.5f; }
            if (part==4) { t+=81.5f; h=1.5f; }
            if (part==5) { l+=2; t+=2; w-=4; h=13; }
            if (part>=6) { l+=10; t=part==6 ? 84 : 105; w-=20; h=0.35f; }
            /* A quiet inset for two bounded rows of help, clear of the
             * native Cancel/OK buttons. */
            if (q>=16 && q<21) {
                l=45; t=150; w=310; h=42;
                if (q==17) h=0.75f;
                if (q==18) w=0.75f;
                if (q==19) { l+=309.25f; w=0.75f; }
                if (q==20) { t+=41.25f; h=0.75f; }
            }
            for (u32 v=0;v<4;++v) {
                positions[active][q*12+v*3]=visible ? l+((v&1) ? w : 0) : -100;
                positions[active][q*12+v*3+1]=visible ? t+(v>=2 ? h : 0) : -100;
                positions[active][q*12+v*3+2]=0;
                /* Same mineral sample as the main menu, no mirrored tiles. */
                uvs[active][q*8+v*2]=(part==0 ? 0.5f+63*(v>=2) : 32.5f)/512;
                uvs[active][q*8+v*2+1]=1-(part==0 ? 8.5f+231*(v&1) : 80.5f)/256;
                const u8 border=part>=1 && part<=4;
                float shade=part==0 ? 0.30f : part==5 ? 0.24f : 0.45f;
                if (border) shade=selected ? 1.0f : 0.70f;
                if (part==3 || part==4) shade*=0.65f;
                if (q>=16) shade=q==16 ? 0.18f : 0.48f;
                colors[active][q*16+v*4]=shade;
                colors[active][q*16+v*4+1]=shade*(selected && border ? 0.79f : 0.94f);
                colors[active][q*16+v*4+2]=shade*(selected && border ? 0.36f : 0.98f);
                colors[active][q*16+v*4+3]=visible ? 0.96f : 0;
            }
            for (u32 i=0;i<6;++i) indices[q*6+i]=q*4+order[i];
        }
        u32 descriptor[0x120/4]={0};
        for (u32 i=0;i<0x118/4;++i) descriptor[i]=((const u32*)0x004D54D0)[i];
        descriptor[0]=(u32)positions[active]; descriptor[1]=(u32)uvs[active];
        descriptor[2]=(u32)colors[active]; descriptor[4]=(u32)indices;
        ((void*(*)(void*,void*))ADDR(0x00348F34))(resources[active],descriptor);
        void* texture=((void*(*)(u32))ADDR(0x002E11D0))(6);
        ((void(*)(void*,u32,void*,u32,u32,u32,u32))ADDR(0x00348A64))
            (resources[active],0,texture,0x2601,0x2601,0x812f,0x812f);
        nodes[active]=((void*(*)(void*,void*,void*,u32))ADDR(0x0034897C))
            (*(void**)0x005C0A34,resources[active],0,0);
        if (nodes[active]) *(u32*)((u8*)nodes[active]+0x178)|=2;
    }
    if (nodes[active])
        ((void(*)(void*))(*(u32*)(*(u32*)nodes[active]+12)))(nodes[active]);
}

/* Native font advances vary; keep short descriptions visually centered
 * instead of anchoring every line at the longest description's left edge. */
static void CenteredText(u32 id,float y,float scale) {
    float width=0;
    for (const char* p=sWords[id];*p;++p) {
        const char c=*p;
        width+=c==' ' ? 6 : c=='i' || c=='l' || c=='.' || c==':' ? 4 :
               c=='m' || c=='w' || (c>='A' && c<='Z') ? 10 : 8;
    }
    Text(id,200-width*scale*0.5f,y,scale,0);
}

#if defined(Version_JP)
/* The JP native Options text remains bound to the second-screen render target.
 * Draw a compact Japanese-only glyph subset after the native GPU replay. This
 * avoids the JP message-constructor stall and leaves the native QBF untouched. */
static const char* const sJpLabels[]={
    "L注目", "一人称カメラ", "ジャイロ操作",
    "HUDサイズ", "カメラ距離", "カメラ操作",
    "移動速度", "前転速度", "前転後の硬直"
};
static const char* const sJpHelp[]={
    "L注目の操作方法を設定します。",
    "一人称視点の上下操作を設定します。",
    "一人称視点でジャイロ操作を使います。",
    "ゲーム画面のHUDサイズを設定します。",
    "探索時のカメラ距離を設定します。",
    "右スティックのカメラ操作を設定します。",
    "歩き・走りの速さを設定します。",
    "前転の速さと移動距離を設定します。",
    "前転後の硬直時間を短くします。"
};
static const char* const sJpTargetValues[]={"ホールド","スイッチ"};
static const char* const sJpAxisValues[]={"ノーマル","反転"};
static const char* const sJpOnOffValues[]={"オン","オフ"};
static const char* const sJpHudValues[]={"非表示","75%","100%","125%"};
static const char* const sJpDistanceValues[]={
    "75%","87.5%","100%","112.5%","125%","137.5%","150%"
};
static const char* const sJpCameraValues[]={
    "ノーマル","X軸反転","Y軸反転","両軸反転"
};
static const char* const sJpSpeedValues[]={"ノーマル","+10%","+20%"};
static const char* const sJpRecoveryValues[]={"ノーマル","-10%","-20%"};

static u32 JpNextCodepoint(const char** value) {
    const u8* p=(const u8*)*value;
    u32 codepoint;
    if (p[0]<0x80) { codepoint=p[0]; *value+=1; }
    else if ((p[0]&0xE0)==0xC0) {
        codepoint=((p[0]&0x1F)<<6)|(p[1]&0x3F); *value+=2;
    } else {
        codepoint=((p[0]&0x0F)<<12)|((p[1]&0x3F)<<6)|(p[2]&0x3F);
        *value+=3;
    }
    return codepoint;
}

static const JpOptionsGlyph* JpGlyph(u32 codepoint) {
    for (u32 i=0;i<JP_OPTIONS_GLYPH_COUNT;++i)
        if (gJpOptionsGlyphs[i].codepoint==codepoint) return &gJpOptionsGlyphs[i];
    return 0;
}

static u32 JpTextWidth(const char* value) {
    u32 width=0;
    while (*value) { JpNextCodepoint(&value); width+=JP_OPTIONS_GLYPH_WIDTH; }
    return width;
}

static void MainScreenText(u32 x,u32 y,u32 color,const char* value) {
    while (*value) {
        const JpOptionsGlyph* glyph=JpGlyph(JpNextCodepoint(&value));
        if (glyph) for (u32 row=0;row<JP_OPTIONS_GLYPH_HEIGHT;++row)
            for (u32 col=0;col<JP_OPTIONS_GLYPH_WIDTH;++col)
                if (glyph->rows[row]&(1u<<(JP_OPTIONS_GLYPH_WIDTH-1-col)))
                    Draw_DrawPixelTop(x+col,y+row,color);
        x+=JP_OPTIONS_GLYPH_WIDTH;
    }
}

static void MainScreenCentered(u32 center,u32 y,u32 color,const char* value) {
    const u32 width=JpTextWidth(value);
    MainScreenText(center-(width<2*center ? width/2 : center),y,color,value);
}

static const char* JpValue(u32 word) {
    const u32 value=sDraft.values[word];
    if (word==0) return sJpTargetValues[value];
    if (word==1) return sJpAxisValues[value];
    if (word==2) return sJpOnOffValues[value];
    if (word==3) return sJpHudValues[value];
    if (word==4) return sJpDistanceValues[value];
    if (word==5) return sJpCameraValues[value];
    if (word==8) return sJpRecoveryValues[value];
    return sJpSpeedValues[value];
}

static void MainScreenRow(u32 col,u32 row,u32 modelRow,u32 word) {
    const u32 panelLeft=col ? 207 : 30;
    const u32 panelRight=col ? 370 : 193;
    const u32 y=66+row*21;
    const u8 selected=sDraft.column==col && sDraft.row==modelRow;
    const u32 color=selected ? COLOR_GREEN : COLOR_WHITE;
    const char* value=JpValue(word);
    if (selected) MainScreenText(panelLeft+3,y,color,">");
    MainScreenText(panelLeft+15,y,color,sJpLabels[word]);
    MainScreenText(panelRight-5-JpTextWidth(value),y,color,value);
}

void MenuOptions_DrawMainScreen(void) {
    if (!sOpen) return;
    MainScreenCentered(200,26,COLOR_TITLE,"リフレームド設定");
    MainScreenCentered(111,48,sDraft.column==0 ? COLOR_GREEN : COLOR_GRAY,
                       "ゲーム");
    MainScreenCentered(288,48,sDraft.column==1 ? COLOR_GREEN : COLOR_GRAY,
                       sRightPage ? "移動" : "カメラ・HUD");
    for (u32 row=0;row<3;++row) {
        MainScreenRow(0,row,row,row);
        const u32 modelRow=row+sRightPage*3;
        MainScreenRow(1,row,modelRow,3+modelRow);
    }
    MainScreenCentered(200,154,COLOR_GRAY,sSaveFailed ?
        "設定を保存できません。もう一度お試しください。" :
        sDraft.row<MENU_OPTIONS_FOOTER ?
            sJpHelp[sDraft.column*3+sDraft.row] :
            "キャンセル：変更を破棄　決定：保存");
    MainScreenCentered(200,172,COLOR_WHITE,
                       "十字キー：移動・変更　A：決定");
}
#else
void MenuOptions_DrawMainScreen(void) {}
#endif

void MenuOptions_Draw(void) {
    if (!sOpen) return;
#if defined(Version_JP)
    /* JP uses the stable atlas panels here; localized text is composited into
     * the final top framebuffer after the native secondary-screen replay. */
    ((void(*)(unsigned,int,unsigned,unsigned))ADDR(0x002FEABC))(0,0,480,400);
    DrawPanels();
    return;
#endif
    PrepareText();
    ((void(*)(unsigned,int,unsigned,unsigned))ADDR(0x002FEABC))(0,0,480,400);
    DrawPanels();
    CenteredText(34,36,0.48f);
    Text(37,58,50,0.56f,sDraft.column==0);
    Text(38,241,50,0.56f,sDraft.column==1);
    Text(51+sRightPage,339,51,0.36f,sDraft.column==1);
    for (u32 col=0;col<2;++col) for (u32 row=0;row<3;++row) {
        u32 selectedRow=row+(col ? sRightPage*3 : 0), i=col*3+selectedRow;
        float x=col ? 219 : 42, y=64+row*21;
        u8 focus=sDraft.column==col && sDraft.row==selectedRow;
        Text(sLabels[i],x,y,0.52f,focus);
        Text(sValueBase[i]+sDraft.values[i],x+15,y+10,0.62f,focus);
        Text(35,x,y+10,0.62f,focus); Text(36,x+132,y+10,0.62f,focus);
    }
    CenteredText(HelpWord(),168,0.47f);
}
#else
void MenuOptions_Begin(void) {}
void MenuOptions_End(void) {}
int MenuOptions_Update(u32 pressed) { (void)pressed; return -1; }
int MenuOptions_Touch(int x,int y) { (void)x; (void)y; return -1; }
void MenuOptions_Draw(void) {}
void MenuOptions_DrawMainScreen(void) {}
u32 MenuOptions_NativeButtons(void) { return 0; }
void MenuOptions_ExitNative(int accept) { (void)accept; }
#endif
