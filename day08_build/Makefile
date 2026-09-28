CC=gcc
CFLAGS=-Wall -Wextra -O0 -g

sensor_app : main.o sensor.o
	$(CC) main.o sensor.o -o sensor_app

main.o : main.c sensor.h
	$(CC) $(CFLAGS) -c main.c -o main.o

sensor.o : sensor.c sensor.h
	$(CC) $(CFLAGS) -c sensor.c -o sensor.o