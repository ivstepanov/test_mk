#include "common.h"

HANDLE setup_com(char *com_name)
{
    DCB dcb;
    HANDLE hComm;

    hComm = CreateFile(com_name, 
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                       );

    if (hComm == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    memset(&dcb, 0, sizeof(DCB));
   
    //  115200 bps, 8 data bits, no parity, and 1 stop bit.
    dcb.BaudRate = CBR_115200;     //  baud rate
    dcb.ByteSize = 8;             //  data size, xmit and rcv
    dcb.Parity   = NOPARITY;      //  parity bit
    dcb.StopBits = ONESTOPBIT;    //  stop bit

    if (!SetCommState(hComm, &dcb)) {
        CloseHandle(hComm);
        return INVALID_HANDLE_VALUE;
    }
    return hComm;
}

void close_com(HANDLE hComm)
{
    CloseHandle(hComm);
}

int set_com_timeouts(HANDLE hComm)
{
    COMMTIMEOUTS Timeouts;

    Timeouts.ReadIntervalTimeout         = 100;
    Timeouts.ReadTotalTimeoutConstant    = 100;
    Timeouts.ReadTotalTimeoutMultiplier  = 1;
    Timeouts.WriteTotalTimeoutConstant   = 100;
    Timeouts.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(hComm, &Timeouts)) {
        return -1;
    }
    return 0;
}

int send_data(HANDLE hComm, BYTE *data, DWORD data_len)
{
    DWORD written_bytes; 

    if (hComm == INVALID_HANDLE_VALUE || data == NULL || data_len == 0) {
        return -1;
    }

   if (WriteFile(hComm, data, data_len, &written_bytes, NULL) == 0) {
        return 1;
   }
   if (written_bytes != data_len) {
        printf("Warning: write %d bytes, written %d bytes\n",
            data_len, written_bytes);
    }

    return 0;
}

DWORD receive_data(HANDLE hComm, BYTE *data, DWORD data_len)
{
    DWORD readed = 0;

    if (hComm == INVALID_HANDLE_VALUE || data == NULL || data_len == 0) {
        return 0;
    }

    if (ReadFile(hComm, data, data_len, &readed, NULL) == 0) {
        return 0;
    }
    return readed;
}

void crc8_calc(uint8_t *crc, uint8_t data)
{
    volatile uint8_t mask;
    volatile int8_t i;

    for (i = 0; i < 8; i++) {
        mask = (*crc & 0x1) ^ (data & 0x1);
        data >>=  1;
        *crc >>=  1;
        
        if (mask == 1) {
            *crc ^= POLINOM;
        }
    }
}

int hex_symbol_to_digit(char symbol, uint8_t *digit)
{
    if (symbol >= 'A' && symbol <= 'F') {
        *digit = symbol - 'A' + 10;
    } else if (symbol >= '0' && symbol <= '9') {
        *digit = symbol - '0';
    } else {
        return -1;
    }
    return 0;
}


