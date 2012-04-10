#pragma once

void spi_setup();
inline void spi_wait() { while (!(SPSR & _BV(SPIF))); }
void spi_send(uint8_t byte);
