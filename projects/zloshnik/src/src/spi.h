#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void spi_setup();
inline void spi_wait() { while (!(SPSR & _BV(SPIF))); }
void spi_send(uint8_t byte);


#ifdef __cplusplus
}
#endif

