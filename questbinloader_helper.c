#include <stdio.h>
#include "questbinloader_helper.h"
#include <shmcomm_user.h>

#define PROC_CHANNEL_ID 1001

// send a binary to another sandbox
int send_to_guest(int coreid, void* data, size_t sz) {
  DEBUG_PRINT ("Connecting channel to send data to core %d via shmcomm.\n", coreid);
  int try = 0, write_channel;
  do {
    write_channel = shmcomm_open_send(PROC_CHANNEL_ID + coreid, SHMCOMM_CONNECT_CH
    		| SHMCOMM_SYNC_CH, NULL, 0, 0);
    DEBUG_PRINT ("Connection successful\n");
    if(write_channel < 0) {
	  printf ("1001 Channel connection failed: %d\n", write_channel);
	  usleep(1000000UL);
	  try++;
    }
  } while(write_channel < 0 && try < 5);
  int ret = shmcomm_write(write_channel, data, sz);

  DEBUG_PRINT ("L:Written the program via shmcomm to quest, number of bytes written: %d\n", ret);
  shmcomm_close(write_channel);
  return 0;
}
