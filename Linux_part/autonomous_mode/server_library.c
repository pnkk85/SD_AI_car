#include "server_library.h"
#include <time.h>
#include <stdlib.h>
#include <linux/can.h>
#include <linux/can/raw.h>

/* Function definitions -------------------------------------------- */
int delay_ms(int number_of_ms)
{
    clock_t start = clock();
    while(clock() < start + number_of_ms);
}

int* get_steer_throttle_from_string(char* str)
{
char steer_char[4];
    char throttle_char[4];
    static int st[2];

    int s_start = 1;
    int t_start = 6;
    int length = 4;
    int index = 0;

    while(index < length)
    {
        steer_char[index] = str[s_start+index];
        index++;
    }
    steer_char[length] = '\0';
    st[0] = atoi(steer_char);

    index = 0;
    length = 4;

    while(index < length)
    {
        throttle_char[index] = str[t_start+index];
        index++;
    }
    throttle_char[length] = '\0';
    st[1] = atoi(throttle_char);

    return st;
}
