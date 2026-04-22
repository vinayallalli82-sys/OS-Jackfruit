# OS-Jackfruit - Mini Container Runtime

## Overview

This project implements a lightweight container runtime in C along with a Linux kernel module for monitoring and enforcing memory usage.

It demonstrates container concepts such as process isolation, supervision, IPC, logging, and resource control.

---

## Architecture

User CLI (engine) sends commands to the supervisor using a UNIX domain socket.  
The supervisor creates and manages containers and communicates with the kernel module.

Flow:

CLI -> Engine -> UNIX Socket -> Supervisor -> Container
                                                      |
                                               Kernel Module

The kernel module monitors memory usage (RSS) and enforces limits.

---

## Features

1. Multi-container supervision  
   Multiple containers run under a single supervisor process.

2. Metadata tracking  
   Container ID, PID, and state are displayed using:
   ./engine ps

3. Logging  
   Container output is stored in:
   ../rootfs-alpha/logs/<container>.log

4. CLI and IPC  
   Commands are sent from CLI to supervisor using UNIX domain sockets.

5. Memory enforcement  
   Soft limit: warning shown in dmesg  
   Hard limit: process is killed  

6. CPU scheduling experiment  
   CPU usage is compared between normal and container execution.

7. Clean teardown  
   Containers are stopped properly and no zombie processes remain.

---

## Project Structure

OS-Jackfruit/
|
|-- boilerplate/
|   |-- engine.c
|   |-- monitor.c
|   |-- monitor_ioctl.h
|   |-- cpu_hog
|   |-- memory_hog
|
|-- rootfs-alpha/
|   |-- logs/
|
|-- screenshots/
|   |-- multi_container.png
|   |-- metadata_ps.png
|   |-- logging_output.png
|   |-- cli_ipc.png
|   |-- memory_limits.png
|   |-- cpu_normal.png
|   |-- cleanup.png
|
|-- README.md

---

## Build Instructions

cd boilerplate

gcc -o engine engine.c -lpthread

make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

sudo insmod monitor.ko

grep container_monitor /proc/devices
sudo mknod /dev/container_monitor c <major> 0
sudo chmod 666 /dev/container_monitor

---

## Run Instructions

Start supervisor:

sudo ./engine supervisor ../rootfs-alpha

Start container:

sudo ./engine start test ../rootfs-alpha "/memory_hog 5 10000"

List containers:

./engine ps

View logs:

cat ../rootfs-alpha/logs/test.log

Stop container:

sudo ./engine stop test

---

## Screenshots

Multi-container supervision  
screenshots/multi_container.png  

Metadata tracking  
screenshots/metadata_ps.png  

Logging  
screenshots/logging_output.png  

CLI and IPC  
screenshots/cli_ipc.png  

Memory limits (soft and hard)  
screenshots/memory_limits.png  

CPU experiment  
screenshots/cpu_normal.png  

Clean teardown  
screenshots/cleanup.png  

---

## Conclusion

This project demonstrates the complete lifecycle of containers including creation, monitoring, enforcement, and cleanup using both user-space and kernel-space components.
