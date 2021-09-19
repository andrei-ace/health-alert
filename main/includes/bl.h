#pragma once

void initialise_bl();

void start_bl(void);
void stop_bl(void);
void bl_get_simple_message(char *);

#define ADDR_LEN 18
#define MAX_ADDR_LEN 10
#define MSG_QUEUE_SIZE (MAX_ADDR_LEN * (ADDR_LEN + 3) + 50)