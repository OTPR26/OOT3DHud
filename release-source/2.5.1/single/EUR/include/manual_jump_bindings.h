#pragma once
/* Verified against the regional movement routines, not uniform address deltas. */
#define MANUAL_JUMP_MENU_STATE 0x0050AF68
#if defined(Version_USA)
#define MANUAL_JUMP_START_ADDR 0x0034B3DC
#define MANUAL_JUMP_ACTION_A 0x004BA378
#define MANUAL_JUMP_ACTION_B 0x004A279C
#define MANUAL_JUMP_ACTION_C 0x004A34D0
#elif defined(Version_EUR)
#define MANUAL_JUMP_START_ADDR 0x0034B3DC
#define MANUAL_JUMP_ACTION_A 0x004BA378
#define MANUAL_JUMP_ACTION_B 0x004A27BC
#define MANUAL_JUMP_ACTION_C 0x004A34F0
#elif defined(Version_JP)
#define MANUAL_JUMP_START_ADDR 0x0034AEF4
#define MANUAL_JUMP_ACTION_A 0x004BA378
#define MANUAL_JUMP_ACTION_B 0x004A2774
#define MANUAL_JUMP_ACTION_C 0x004A34A8
#else
#error "Complete the regional manual-jump binding audit before enabling this build"
#endif
