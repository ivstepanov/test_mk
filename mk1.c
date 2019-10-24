#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <conio.h>
#include "common.h"

static HANDLE hComm;

void usage(void)
{
    printf("usage: mk1 <com-port>\n Press 'e' to finish app\n\n");
}

char * make_packet(uint8_t pkt_len, uint8_t brd_status, int8_t temperature, 
    uint16_t position)
{
    /* packet format (ASCII): 
        start symbol: ':', 
        length: 1byte (2 ASCII symbols), 
        data: 4byte (8 ASCII symbols) 
        crc8: 1byte (2 ASCII symbols)*/
    static char packet_str[14];
    size_t i, len;
    uint8_t crc8;

    sprintf(packet_str, "%c%02X%02X%02X%04X", START_SYMBOL, pkt_len, brd_status, 
        (uint8_t)temperature, position);

    len = strlen(packet_str);

    for (crc8 = 0, i = 0; i < len; i++) {
        crc8_calc(&crc8, packet_str[i]);
    }
    sprintf(packet_str + len, "%02X", crc8);
    return packet_str;
}

VOID CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
    static int timer_cnt;
    int8_t temperature;
    uint8_t brd_status;
    uint16_t position;
    uint8_t crc8;
    char *pkt_str;

    ++timer_cnt;
    /* 1s */
    if (timer_cnt >= 100) {
        brd_status = (uint8_t)rand();
        temperature = (uint8_t)rand();
        timer_cnt = 0;
    }

    position =(uint16_t)rand();

    pkt_str = make_packet(4, brd_status, temperature, position);
    send_data(hComm, (BYTE*)pkt_str, strlen(pkt_str));
}

int main(int argc, char *argv[])
{
    UINT uTimerId;
    MSG Msg;

    if (argc != 2) {
        usage();
        return -1;
    }

    srand(time(NULL));

    hComm = setup_com(argv[1]);
    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Error can't open '%s'\n", argv[1]);
        return -1;
    }

    uTimerId = SetTimer(NULL, 0, 10, (void*)&TimerProc);
    /* printf("Timer: %d\n", uTimerId); */

    for (;;) {
        if (GetMessage(&Msg, NULL, 0, 0)) {
            DispatchMessage(&Msg);
        }
        if (_kbhit() == 0) {
            continue;
        }
        if (_getch() == 'e') {
            break;
        }
    }

    KillTimer(NULL, uTimerId);
    close_com(hComm);

    return 0;
}
