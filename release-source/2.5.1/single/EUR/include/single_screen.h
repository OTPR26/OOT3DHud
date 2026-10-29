#ifndef OOT3D_SINGLE_SCREEN_H
#define OOT3D_SINGLE_SCREEN_H

void SingleScreen_DrawSecondaryCallback(void* context,
                                        void (*callback)(void*));
void SingleScreen_AfterTopPass(void* pass);
void SingleScreen_BeforeTopOverlay(void);
void SingleScreen_BeforeTopPresentation(void);
void SingleScreen_BeforeMenuTitle(void);
void SingleScreen_AfterMenuTitle(void);

#endif
