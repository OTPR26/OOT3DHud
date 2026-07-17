# Implementation notes

## Scope boundary

The patch modifies presentation, physical-to-vanilla input routing, and camera behavior. It does not
intentionally change damage, movement, inventory contents, item availability, progression flags,
actor behavior, save data, or game balance.

The D-pad implementation synthesizes the same touchscreen samples a player can produce manually. It
does not invoke item or player routines directly.

## Update flow

1. The loader transfers control to the expanded code region.
2. The `GlobalContext_Update` entry hook initializes services once.
3. Physical input and C-stick state are sampled.
4. `Controls_Resolve` gives the L+R camera-settings chord priority.
5. A gameplay D-pad action becomes a vanilla touchscreen sample immediately before the normal game
   update: Left is I, Down is II, Up is Navi/View, and Right is the Ocarina.
6. OoT3D processes that sample through its original UI/action path.
7. The native HUD board is updated from live game state after the game update.
8. The established free-camera hook handles supported normal-camera updates after C-stick activation.

The touch injector remembers the exact HID ring entry it changes. If HID has not advanced on the next
frame, it clears only that matching synthetic entry and does not overwrite a newer physical touch.

## Native HUD

The release renderer reuses an OoT3D native stereoscopic top-screen board and redirects it to the
2:1 `cam_interface00` texture slot. Azahar Plus can therefore apply a high-resolution replacement
texture before internal-resolution scaling; no release HUD sprites are drawn into the 400x240
framebuffer.

The board contains live quads for:

- the action A prompt and its changing text;
- B, X, and Y items;
- Navi, Ocarina, I, and II around the D-pad;
- all 20 possible hearts, including partial and missing-health states;
- the current magic amount;
- the rupee icon and value.

I and II resolve from the active equipment state and remain blank when unassigned. HUD reads are
read-only; the renderer does not write SaveContext fields.

Only the board's low-index quads proved reliable, so the heart renderer combines adjacent hearts into
ten pair quads. A 15-state atlas bank represents every valid sequential fill state for a pair, allowing
the full 20-heart maximum while retaining reliable low-index quads for rupees and magic.

## Validated configuration

- USA retail 1.0
- Azahar Plus on Android/AYN hardware and Azahar on macOS
- normal gameplay with live items, action text, health, magic, and rupees
- all D-pad press/release routes
- standalone free camera and its L+R settings chord
- concurrent 4K custom texture pack
- 20-heart save at full health

Not yet validated: EUR, JP, other game revisions, Master Quest, original 3DS hardware, stereoscopic
3D, and every context-specific item restriction or minigame.
