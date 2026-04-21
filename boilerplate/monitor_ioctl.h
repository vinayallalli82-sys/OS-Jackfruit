#ifndef MONITOR_IOCTL_H
#define MONITOR_IOCTL_H

#include <linux/ioctl.h>

#define DEVICE_FILE "/dev/container_monitor"

struct container_info {
    char id[32];
    pid_t pid;
    size_t soft_limit;
    size_t hard_limit;
};

#define IOCTL_REGISTER _IOW('a', 'a', struct container_info)

#endif
