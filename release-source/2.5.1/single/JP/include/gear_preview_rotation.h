#pragma once

/* Native C-stick range is approximately +/-156. One 30 Hz menu update
 * at full deflection turns about 3.6 degrees; remove the dead zone smoothly.
 * Reverse the original portrait direction and increase speed by 20%. */
static inline unsigned short GearPreview_UpdateYaw(unsigned short yaw, int stickX,
                                                   int active, int wasActive) {
    if (!active || !wasActive) return 0;
    if (stickX > 156) stickX = 156;
    if (stickX < -156) stickX = -156;
    int magnitude = stickX < 0 ? -stickX : stickX;
    if (magnitude <= 30) return yaw;
    int delta = (magnitude - 30) * 546 * 6 / (126 * 5);
    return (unsigned short)(yaw + (stickX < 0 ? delta : -delta));
}
