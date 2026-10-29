#pragma once
#include "z3D/z3D.h"

/* Local USA prototype: independent-model probe and bounded draw test.
 * Never modifies saves or the live player's model. */
void GearPreview_Probe(GlobalContext* globalCtx);
void GearPreview_TracePresentation(void* pass);
void GearPreview_DrawCamera(void* queue);
