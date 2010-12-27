#ifndef MODPLAYER_H
#define MODPLAYER_H

#ifndef FRASMODINFO

#error Make sure you include any mod-resource file before Fras.h

#else

#define FRAS_MEDIUM_FRQ 0
#define FRAS_LOW_FRQ 1
#define FRAS_HIGH_FRQ 2

void FrasInstall(unsigned frq);

void FrasPlayMod(const modInfo *mi);
void FrasStopMod(void);

void FrasPauseMod(void);
void FrasUnPauseMod(void);
void FrasTimer1Intr(void);

#endif

#endif


