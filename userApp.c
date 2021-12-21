#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>

#define WR_VALUE _IOW('a','a',struct message*)
#define MAX_PCI_DEV_COUNT 64

struct pci_dev_info {
    unsigned short  device[MAX_PCI_DEV_COUNT];
    unsigned short  vendor[MAX_PCI_DEV_COUNT];
    int pci_dev_count;
};

struct signal_struct_info {
    bool valid;
    int     nr_threads;
    int     group_exit_code;
    int     notify_count;
    int     group_stop_count;
    unsigned int    flags;
};

struct message {
    struct signal_struct_info* ssi;
    struct pci_dev_info* pdi;
    pid_t pid;
};

int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Not enough arguments. Enter pid\n" );
        return 0;
    }
    struct message msg;
    struct pci_dev_info pdi;
    struct signal_struct_info ssi;
    msg.pid = atoi(argv[1]);
    msg.pdi = &pdi;
    msg.ssi = &ssi;


    int fd;

    printf("\nOpening Driver\n");
    fd = open("/dev/etx_device", O_RDWR);
    if(fd < 0) {
        printf("Cannot open device file...\n");
        return 0;
    }

    printf("Writing data to Driver\n");
    ioctl(fd, WR_VALUE, (struct message*) &msg); 

    printf("pci found %d devices:\n", msg.pdi->pci_dev_count);
    for (int i = 0; i < msg.pdi->pci_dev_count; i++){
        printf("\tpci found device = %d, vendor = %d\n", msg.pdi->device[i], msg.pdi->vendor[i]);
    }

    if (msg.ssi->valid == true){
        printf("\nsignal_struct_info for PID %d: \n", msg.pid);
        printf("\tnr_threads = %d\n", msg.ssi->nr_threads);
        printf("\tgroup_exit_code = %d\n", msg.ssi->group_exit_code);
        printf("\tnotify_count = %d\n", msg.ssi->notify_count);
        printf("\tgroup_stop_count = %d\n", msg.ssi->group_stop_count);
        printf("\tflags = %d\n", msg.ssi->flags);
    } else printf("\ntask_struct for pid %d is NULL. Can not get any information\n", msg.pid);

    printf("\nClosing Driver\n");
    close(fd);
}
