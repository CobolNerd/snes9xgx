#ifndef _IBUFFALO_H_
#define _IBUFFALO_H_

#include <gctypes.h>

#ifdef __cplusplus
extern "C" {
#endif

bool Ibuffalo_ScanPads();
u32 Ibuffalo_ButtonsHeld(int chan);
char* Ibuffalo_Status();

#ifdef __cplusplus
}
#endif

#endif
