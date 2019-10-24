#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <stdint.h>

#define START_SYMBOL        ':'

#define POLINOM             0x8C

void crc8_calc(uint8_t *crc, uint8_t data);
int hex_symbol_to_digit(char symbol, uint8_t *digit);

HANDLE setup_com(char *com_name);
void close_com(HANDLE hComm);
int set_com_timeouts(HANDLE hComm);
int send_data(HANDLE hComm, BYTE *data, DWORD data_len);
DWORD receive_data(HANDLE hComm, BYTE *data, DWORD data_len);

#endif // #ifndef COMMON_H
