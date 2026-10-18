#include <stdio.h>

typedef struct{
    int seq;
    float temp;
}sensor_packet_t;

static float read_temp(sensor_packet_t* packet)
{
    float temp=packet->temp;
    return temp;
}

static void process_packet(sensor_packet_t* packet)
{
    float temp=read_temp(packet);
    printf("温度=%.1f\n",temp);
}

static void handle_client_data(void)
{
    sensor_packet_t* packet=NULL;
    process_packet(packet);
}

int main(void)
{
    printf("[main] processing packet\n");
    handle_client_data();
    return 0;
}