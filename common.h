#ifndef COMMON_H
#define COMMON_H

#include <windows.h>

HANDLE setup_com(char *com_name);
void close_com(HANDLE hComm);
int set_com_timeouts(HANDLE hComm);
int send_data(HANDLE hComm, BYTE *data, DWORD data_len);
DWORD receive_data(HANDLE hComm, BYTE *data, DWORD data_len);

#endif // #ifndef COMMON_H
