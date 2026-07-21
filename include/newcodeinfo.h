/* Expanded code region reserved by the linker scripts. */

#ifndef _NEWCODEINFO_H_
#define _NEWCODEINFO_H_

#if defined(Version_TWN) || defined(Version_KOR)
// The CJK builds clear BSS through 0x005CDB20 during startup.  Keep the
// injected image above that range so the game's first instruction cannot
// erase it after loader_main returns.
#define NEWCODE_OFFSET 0x005CE000
#else
#define NEWCODE_OFFSET 0x005C7000
#endif
#define NEWCODE_SIZE   0x0002E000

#endif //_NEWCODEINFO_H_
