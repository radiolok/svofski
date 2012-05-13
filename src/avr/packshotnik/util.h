/// \file
/// \brief Utilities
///
/// Bit settings. BCD conversions. Calendar.
///
#ifndef _UTIL_H
#define _UTIL_H

#define BV2(a,b) (_BV(a)|_BV(b))
#define BV3(a,b,c) (_BV(a)|_BV(b)|_BV(c))
#define BV4(a,b,c,d) (_BV(a)|_BV(b)|_BV(c)|_BV(d))
#define BV5(a,b,c,d,e) (_BV(a)|_BV(b)|_BV(c)|_BV(d)|_BV(e))
#define BV6(a,b,c,d,e,f) (_BV(a)|_BV(b)|_BV(c)|_BV(d)|_BV(e)|_BV(f))
#define BV7(a,b,c,d,e,f,g) (_BV(a)|_BV(b)|_BV(c)|_BV(d)|_BV(e)|_BV(f)|_BV(g))

/// Convert to binary from BCD representation 
/// \see frombcd
#define _frombcd(x) ((x & 017) + (((x) & 0160)>>4) * 10)

/// Convert to binary from BCD representation as a function.
/// \see _frombcd
uint8_t frombcd(uint8_t);

uint16_t tobcd16(uint16_t);

/// Increment BCD value by 1
uint8_t bcd_increment(uint8_t x);

#endif
