/*
 * This program loads a quest binary directly onto quest from yocto linux, using shared memory to transfer the binary.
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "questbinloader_helper.h"
#include <sys/stat.h>

typedef struct __attribute__ ((__packed__)) {
  int argv_size; //Used to store total size of argv[] buffer
  char argv[MAX_ARGS_LENGTH]; //Contains argv for quest binary, if any
  size_t binary_size; //Quest binary size
  char binexec[]; //The binary itself
} program_t;


int main(int argc, char *argv[], char *const envp[]) {

  if(elf_version(EV_CURRENT) == EV_NONE) {
    printf ("ELF Library, libelf, initialization has failed\n");
    exit(-1);
  }

  char *usage = "Usage: questbinloader <quest binary> <argc> <argv1> ... <argvn>";

  if(argc < 2) {
    printf ("Wrong usage.\n%s\n", usage);
    exit(-1);
  }

  int fd;

  if((fd = open(argv[1], O_RDONLY)) < 0) {
    printf("Bad binary name, unable to open quest binary  %s.\n", argv[1]);
    exit(-1);
  }

  struct stat st;
  if (fstat(fd, &st) != 0) {
    printf("Bad quest binary %s size.\n", argv[1]);
    close(fd);
    exit(-1);
  }

  // Assuming this to be the quest core used. 
  int coreid = 0;  

  // Finding quest binary size
  size_t packetsize = st.st_size + sizeof(program_t);
  program_t* qprogram = (program_t*) malloc (packetsize);
  qprogram->binary_size = st.st_size;

  // Setting up argv needed to be sent via shmcomm for the quest binary, if any
  int ctr = 2; //Iterates through each argv
  int chariter=0; //Iterates through each character of each argv
  int argvsizectr = 0; //Keeps track of total size of argv character by character
  char argvtemp[MAX_ARGS_LENGTH];

  if (argc > 2) { // there are arguments expected
    for (ctr=2; ctr < argc; ctr++) {
        for(chariter=0; argv[ctr][chariter]!='\0'; chariter++) {
          argvtemp[argvsizectr++] = argv[ctr][chariter];
        }
      if (ctr < argc - 1) argvtemp[argvsizectr++] = ' ';
    }
  }
  qprogram->argv_size = argvsizectr;
  argvtemp[argvsizectr] = '\0';

  // Copy argv to be sent onto the packet being sent
  int iter = 0;
  for (iter = 0; iter < qprogram->argv_size; iter++) {
    qprogram->argv[iter] = argvtemp[iter];	
  }

  DEBUG_PRINT("\nSet up argvs if any (%s), setting up quest binexec itself...", argvtemp);
  
  char *binary_data = malloc(qprogram->binary_size);
  if (read(fd, binary_data,qprogram-> binary_size) != qprogram->binary_size) {
    printf("Bad binary file read issue> %s.\n", argv[1]);
    free(binary_data);
    close(fd);
    exit(-1);
  }
  close(fd);
  memcpy(qprogram->binexec, binary_data, qprogram->binary_size);

  // Sending to quest.
  DEBUG_PRINT("\nSending packet to quest (binary_size = %d)\n", qprogram->binary_size);
  send_to_guest(coreid, qprogram, packetsize);
  return 0;
}
