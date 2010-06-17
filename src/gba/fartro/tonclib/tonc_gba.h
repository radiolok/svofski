//
// tonc_gba.h : main header file for TONC utility functions
//
// (Created: 2003-05-05, Modified: 2004-07-08, Cearn)

#ifndef GBA_TONC
#define GBA_TONC

#ifdef __cplusplus
extern "C" {
#endif

// === basic types: ===================================================
#include "types.h"
// === control registers ==============================================
#include "regs.h"

// === core functions + dma and timers ================================
#include "core.h"
// === geometry: points, rects, vectors (oh, my) ======================
#include "geom.h"
// === color macros ===================================================
#include "color.h"
// === keys functions =================================================
#include "keypad.h"
// === affine functions (rotate, scale, shear) for bgs and objs =======
#include "affine.h"

// === hardware interrupts ============================================
#include "interrupt.h"
// === software interrupts ============================================
#include "swi.h"


// === video stuff ====================================================
#include "vid.h"
// === background =====================================================
#include "bg.h"
// === objects (sprites) ==============================================
#include "oam.h"

#ifdef __cplusplus
}
#endif

#endif	// GBA_TONC
