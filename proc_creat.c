/*                    The Quest Operating System
 *  Copyright (C) 2005-2025  Richard West, Boston University
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Quest Remote Binary Loader for Direct Quest binaries 

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <shmcomm_user.h>
#include "shmcomm_matlab.h" // only needed for Benchmarking
#include <string.h>
#include <vcpu.h>


typedef struct __attribute__ ((__packed__)) {
  int argv_size;
  char argv[80];
  size_t binary_size;
  char* binexec;
} program_t; 


#define NESTEDBIN_CHANNEL_ID 1001
#define TEST_PROG_SIZE 400000 

int main() {
  init_clock(); 
  int child_pid = fork();

  if(child_pid < 0) {
    fprintf(stderr, "fork failed: pid = %d\n", child_pid);
  }

  if (child_pid) {
    waitpid(child_pid);
    exit(0);
  } else {
    printf ("Q Creating shmcomm channel of VSHM_KEY 1001 to receive the quest binary through\n");

    shmcomm_channel_t read_channel;
    int res = shmcomm_open_receive(NESTEDBIN_CHANNEL_ID, SHMCOMM_CREATE_CH
      | SHMCOMM_SYNC_CH, NULL, TEST_PROG_SIZE, 1, &read_channel);

    if(res < 0) {
      printf("Q Shmcomm error! Terminating! \n");
      exit(0);
    }

    program_t* q_program = (program_t*) malloc(TEST_PROG_SIZE);

    unsigned long long proc_create_recv_start;
    
    while(1) {
      int read_bytes;
      printf ("Q Waiting for binary\n");
      while (1) {
        proc_create_recv_start = get_usecs_matlab();
        read_bytes = shmcomm_read(read_channel, (void *) q_program, TEST_PROG_SIZE);
        if (read_bytes > 0) {
          printf ("Q Received binary of binary_size %d \n", q_program->binary_size, read_bytes); 
          break;
        }
        usleep(1000);
      }
      shmcomm_destroy(read_channel);

      // Actually running the executable
      int nested_child = fork();
      if(nested_child) {
        waitpid(nested_child);
        exit(1);
      } else {
        // Setting up args to be sent to execvp
        // Execvp() uses _exec() function, which is modified to execute raw binaries if we pass on "RAW" as the first argv 
        // It expects "RAW", the binary size, and then the actual_args (which would be the program name - RAW here, and then the argvs)
        // This is stored in prog_args
        char actual_args[80];
        strcpy(actual_args, "RAW ");
        memcpy(actual_args + 4, q_program->argv, q_program->argv_size);
        actual_args[4 + q_program->argv_size + 1] = 0;
        char* program = (char *)(&(q_program->binexec));
        char* prog_args[] = {"/RAW\0", q_program->binary_size, actual_args};
        execvp(program, prog_args);
        return 0;
      }
      return 0;
    }
  }

  return 0;
}
