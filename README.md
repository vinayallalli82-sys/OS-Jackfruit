# 🐳 OS-Jackfruit - Mini Container Runtime

## 📌 Overview

This project implements a lightweight container runtime in C along with a Linux kernel module for monitoring and enforcing memory usage.

It demonstrates core container concepts such as:
- Process isolation
- Supervisor-based management
- Inter-process communication (IPC)
- Logging
- Kernel-level resource control

---

## 🏗️ Architecture

```
CLI → Engine → UNIX Socket → Supervisor → Container
                                           ↓
                                    Kernel Module
```

The supervisor manages containers, and the kernel module monitors memory usage (RSS) and enforces limits.

---

## 🚀 Features

- 🧱 Multi-container supervision  
- 📊 Metadata tracking using `./engine ps`  
- 📄 Logging of container output  
- 🔌 CLI ↔ Supervisor communication using UNIX sockets  
- ⚠️ Memory enforcement (soft + hard limits)  
- 🧪 CPU scheduling experiment  
- 🧹 Clean teardown (no zombie processes)  

---

## 📁 Project Structure

```
OS-Jackfruit/
├── boilerplate/
│   ├── engine.c
│   ├── monitor.c
│   ├── monitor_ioctl.h
│   ├── cpu_hog
│   ├── memory_hog
│
├── rootfs-alpha/
│   └── logs/
│
├── screenshot/
│   ├── multi_container.png
│   ├── metadata_ps.png
│   ├── logging_output.png
│   ├── cli_ipc.png
│   ├── memory_limits.png
│   ├── cpu_normal.png
│   └── cleanup.png
│
└── README.md
```

---

## ⚙️ Build Instructions

```bash
cd boilerplate

gcc -o engine engine.c -lpthread

make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

sudo insmod monitor.ko

grep container_monitor /proc/devices
sudo mknod /dev/container_monitor c <major> 0
sudo chmod 666 /dev/container_monitor
```

---

## ▶️ Run Instructions

### Start supervisor
```bash
sudo ./engine supervisor ../rootfs-alpha
```

### Start container
```bash
sudo ./engine start test ../rootfs-alpha "/memory_hog 5 10000"
```

### List containers
```bash
./engine ps
```

### View logs
```bash
cat ../rootfs-alpha/logs/test.log
```

### Stop container
```bash
sudo ./engine stop test
```

---

## 📸 Screenshots

### 🧱 Multi-container supervision
![Multi Container](screenshot/multi_container.png)

### 📊 Metadata tracking
![Metadata](screenshot/metadata_ps.png)

### 📄 Logging
![Logging](screenshot/logging_output.png)

### 🔌 CLI and IPC
![CLI IPC](screenshot/cli_ipc.png)

### ⚠️ Memory limits (Soft + Hard)
![Memory Limits](screenshot/memory_limits.png)

### 🧪 CPU experiment
![CPU](screenshot/cpu_normal.png)

### 🧹 Clean teardown
![Cleanup](screenshot/cleanup.png)

---

## 🎯 Conclusion

This project demonstrates the complete lifecycle of containers:
- creation
- monitoring
- resource enforcement
- cleanup

It provides a practical understanding of how container systems work internally using both user-space and kernel-space components.
