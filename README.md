# Direct Quest Loader
# Quest Binary Loader details
This loads an ELF Binary of a guest operating system (Quest) onto that system using Linux, using shared memory (shmcomm). 
Beware, this needs the Quest-V SDK and also the shmcomm library functionality for it to run, which can only be obtained by contacting Dr. Rich West.

### Usage
1. Install the SDK and update quest(make sure proc_creat is in the new quest build), run the following commands to run a joint quest and yocto system in the SDK:
```
sudo modprobe -r kvm_intel
sudo modprobe kvm_intel nested=1
sudo qemu-system-i386 -netdev user,id=net0,hostfwd=tcp::8080-:80 -device e1000,netdev=net0     -object rng-random,filename=/dev/urandom,id=rng0 -device virtio-rng-pci,rng=rng0     -drive file=/home/vi/viquest/core-image-minimal-xfce-questv-dx1100.wic,format=raw     -drive if=pflash,format=qcow2,file=/home/vi/viquest/ovmf.qcow2 -cpu Skylake-Client-v4,+vmx,+vmx-vpid -enable-kvm  -m 8G -serial mon:stdio -serial null     -device virtio-vga  -display sdl,show-cursor=off -smp 2 -machine q35 
```
2. Select the Quest-V Yocto option to run both of them jointly
3. We will end up in yocto. Transfer all loader files there
4. Run echo "proc_creat" > /dev/qSB0, to run proc_creat on quest
6. Run the following to transfer any quest binary to directly execute in quest:
```
questbinloader <quest binary> <argv1> ... <argvn>
Eg: ./questbinloader samplebin lorem ipsum
```

### Mechanism
*For Quest:*

We create a channel between Quest and Linux with `vshm_key` of `1001` to send a packet containing args, the binary, etc:
```
typedef struct __attribute__ ((__packed__)) {
  int argv_size; //Used to store total size of argv[] buffer
  char argv[80]; //Contains argv for quest binary, if any
  size_t binary_size; //Quest binary size
  char binexec[]; //The binary itself, stored in raw binary format
} program_t;
```

This channel is a synchronous channel with only one element of size 384 KB (for now).
This is similar to a few number of shared pages between Linux and Quest.

This packet is written to the shared pages by Linux.
The same shared pages are read by Quest, and used to execute the binary. 

### Existing test cases in test_executables
- hello_argvs is the quest hello, prints argv[1]
- hello is the quest hello which takes in no arguments
- test2_demo and vcpu_demo are both just executables from the test directory (Quest-V-SDK-Build/quest_driveos/tests)

 
