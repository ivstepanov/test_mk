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

VOID CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
    static int timer_cnt;
    static uint8_t temperature;
    static uint8_t brd_status;
    uint16_t position;
    uint8_t packet[4];
    

    ++timer_cnt;
    /* 1s */
    if (timer_cnt >= 100) {
        brd_status = (uint8_t)rand();
        temperature = (uint8_t)rand();
        timer_cnt = 0;
    }

    position =(uint16_t)rand();

    packet[0] = brd_status;
    packet[1] = temperature;
    packet[2] = position >> 8 & 0xFFU;
    packet[3] = position & 0xFFU;
    
    /* printf("."); */
    send_data(hComm, packet, sizeof(packet));
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
    printf("Timer: %d\n", uTimerId);

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
