#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>
#include "common.h"

static HANDLE hComm;

void usage(void)
{
    printf("usage: mk2 <com-port>\n Press 'e' to finish app\n\n");
}


int main(int argc, char *argv[])
{
    uint8_t packet[4] = {0};
    uint8_t data_len;
    uint8_t data_cnt;
    uint32_t stat_rx_packets = 0;
    uint32_t stat_dropped_packets = 0;
    uint8_t crc8;
    enum e_state {
        ST_WAIT_START,
        ST_RX_PACKET_LEN,
        ST_RX_PACKET_DATA,
        ST_CHECK
    } state = ST_WAIT_START;

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
        uint8_t rx_data, digit;
        DWORD rx_len;
        int valid_digit;

        rx_len = receive_data(hComm, &rx_data, 1);
        sleep(0);
        if (rx_len == 0) {
            continue;
        }

        valid_digit = hex_symbol_to_digit(rx_data, &digit);

        if (state == ST_WAIT_START) {
            crc8 = 0;
        }
        if (state != ST_CHECK) {
            crc8_calc(&crc8, rx_data);
        }
        

        switch (state) {
        case ST_WAIT_START:
            if (rx_data == START_SYMBOL) {
                
                state = ST_RX_PACKET_LEN;
                printf("RX START SYMBOL ST_WAIT_START->ST_RX_PACKET_LEN\n");
                data_cnt = 0;
            }
            break;

        case ST_RX_PACKET_LEN:
            if (-1 == valid_digit) {
                state = ST_WAIT_START;
                printf("Error: wrong data, packet dropped "
                    "(ST_RX_PACKET_LEN: %c)\n", rx_data);
                ++stat_dropped_packets;
                break;
            }
            if (data_cnt & 1) {
                data_len |= digit;
            } else {
                data_len = digit << 4;
            }
            ++data_cnt;
            if (data_cnt >= 2) {
                state = ST_RX_PACKET_DATA;
                data_cnt = 0;
                memset(packet, 0, sizeof(packet));
            }
            break;

        case ST_RX_PACKET_DATA:
            if (-1 == valid_digit) {
                state = ST_WAIT_START;
                printf("Error: wrong data, packet dropped "
                    "(ST_RX_PACKET_DATA: %c)\n", rx_data);
                ++stat_dropped_packets;
                break;
            }
            if (data_cnt & 1) {
                packet[data_cnt/2] |= digit;
            } else {
                packet[data_cnt/2] = digit << 4;
            }
            if (++data_cnt >= data_len * 2) {
                state = ST_CHECK;
                data_cnt = 0;
            }
            break;

        case ST_CHECK:
            {
                static uint8_t crc8_check;
                if (-1 == valid_digit) {
                    state = ST_WAIT_START;
                    printf("Error: wrong data, packet dropped "
                        "(ST_CHECK: %c)\n", rx_data);
                    ++stat_dropped_packets;
                    break;
                }
                if (data_cnt & 1) {
                    crc8_check |= digit;
                } else {
                    crc8_check = digit << 4;
                }
                if (++data_cnt < 2) {
                    break;
                }
                if (crc8 != crc8_check) {
                    printf("Error: wrong crc8 (%02X %02X), packet dropped\n",
                        crc8, crc8_check);
                        printf("pckt: %02X%02X%02X%02X\n",
                            packet[0], packet[1], packet[2], packet[3]
                            );
                    ++stat_dropped_packets;
                } else {
                    ++stat_rx_packets;
                    printf("board status: 0x%02X\n", packet[0]);
                    printf("temperature: %d\n", packet[1]);
                    printf("position: %u\n", packet[2] << 8 | packet[3]);
                    printf("statistic: rx=%d dropped=%d\n",
                        stat_rx_packets, stat_dropped_packets);
                }
                state = ST_WAIT_START;
            }
            break;
        }

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
