#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>
#include "common.h"

static HANDLE hComm;

void usage(void)
{
    printf("usage: mk1 <com-port>\n Press 'e' to finish app\n\n");
}


int main(int argc, char *argv[])
{
    if (argc != 2) {
        usage();
        return -1;
    }

    hComm = setup_com(argv[1]);
    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Error can't open '%s'\n", argv[1]);
        return -1;
    }

    set_com_timeouts(hComm);

    for (;;) {
        uint8_t packet[4] = {0};
        DWORD rx_len;

        rx_len = receive_data(hComm, packet, sizeof(packet));
        if (rx_len == sizeof(packet)) {
            printf("board status: 0x%02X\n", packet[0]);
            printf("temperature:  0x%02X\n", packet[1]);
            printf("position: 0x%02X%02X\n\n", packet[2], packet[3]);
        }
        sleep(0);

        if (_kbhit() == 0) {
            continue;
        }
        if (_getch() == 'e') {
            break;
        }
    }

    close_com(hComm);

    return 0;
}
