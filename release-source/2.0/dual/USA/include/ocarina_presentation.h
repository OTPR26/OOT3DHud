#ifndef OCARINA_PRESENTATION_H
#define OCARINA_PRESENTATION_H

/* Presentation only: keep an already-owned native message above Link until
 * its closing animation ends, even after the shortcut/menu has closed.
 * Never change message mode, note recognition, or native visibility. */
static inline unsigned OcarinaPresentation_Update(unsigned previous, unsigned allowed,
                                                  unsigned instrumentOpen, unsigned messageActive) {
    if (!allowed)
        return 0;
    if (instrumentOpen)
        return 1;
    return previous && messageActive;
}
#endif
