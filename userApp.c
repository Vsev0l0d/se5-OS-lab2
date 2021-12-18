#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>

#define WR_VALUE _IOW('a','a',struct message*)
#define RD_VALUE _IOR('a','b',struct message*)
#define MAX_PCI_DEV_COUNT 64

struct pci_dev_info{
    unsigned short  device[MAX_PCI_DEV_COUNT];
    unsigned short  vendor[MAX_PCI_DEV_COUNT];
};

struct signal_struct_info {
    int     nr_threads;
    int     group_exit_code;
    int     notify_count;
    int     group_stop_count;
    unsigned int    flags;
};

struct message {
    struct signal_struct_info ssi;
    struct pci_dev_info pdi;
    int pci_dev_count;
};


int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Not enough arguments. Enter pid\n" );
        return 0;
    }
    pid_t pid = atoi(argv[1]);

    int fd;
    struct message msg;

    printf("\nOpening Driver\n");
    fd = open("/dev/etx_device", O_RDWR);
    if(fd < 0) {
        printf("Cannot open device file...\n");
        return 0;
    }


    printf("Writing Pid to Driver\n");
    ioctl(fd, WR_VALUE, (pid_t*) &pid); 

    printf("Reading Value from Driver\n\n");
    ioctl(fd, RD_VALUE, (struct message*) &msg);

    printf("pci found %d devices:\n", msg.pci_dev_count);
    for (int i = 0; i < msg.pci_dev_count; i++){
        printf("\tpci found device = %d, vendor = %d\n", msg.pdi.device[i], msg.pdi.vendor[i]);
    }

    printf("\nsignal_struct_info for PID %d: \n", pid);
    printf("\tnr_threads = %d\n", msg.ssi.nr_threads);
    printf("\tgroup_exit_code = %d\n", msg.ssi.group_exit_code);
    printf("\tnotify_count = %d\n", msg.ssi.notify_count);
    printf("\tgroup_stop_count = %d\n", msg.ssi.group_stop_count);
    printf("\tflags = %d\n", msg.ssi.flags);

    printf("\nClosing Driver\n");
    close(fd);
}
