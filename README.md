# OS Jackfruit – Lightweight Container Runtime

##  Team Members

* **A Vinay-PES1UG24CS647**
* **B KOUSHIK RAJ-PESIUG24CS657**

---

##  Project Overview

OS Jackfruit is a lightweight container runtime implemented in C.
It demonstrates core operating system concepts such as:

* Process isolation
* Memory monitoring
* Resource control using kernel modules
* Multi-container supervision

The project consists of a **user-space runtime (`engine.c`)** and a **kernel module (`monitor.c`)** that enforces memory limits.

---

##  Features

* Start and manage multiple containers
* Track container processes
* Soft memory limit monitoring (logs warning)
* Hard memory limit enforcement (kills process)
* CLI interface for container operations
* Kernel-level memory tracking using RSS

---

##  Technologies Used

* C Programming
* Linux Kernel Modules
* System Calls (`ioctl`, `fork`, `exec`)
* Ubuntu (Virtual Machine)

---

##  Project Structure

```
OS-Jackfruit/
│
├── boilerplate/
│   ├── engine.c
│   ├── monitor.c
│   ├── monitor_ioctl.h
│   ├── memory_hog.c
│   ├── cpu_hog.c
│   ├── Makefile
│
├── screenshots/
├── README.md
```

---

##  Modifications Done

### engine.c

* Fixed struct mismatch for memory limits
* Removed hardcoded limits
* Enabled correct ioctl communication
* Passed user-defined limits to kernel

### monitor.c

* Implemented periodic monitoring using timer
* Added soft limit logging
* Implemented hard limit process termination
* Added debug logs for RSS tracking

### monitor_ioctl.h

* Corrected structure to match kernel and user space

---

##  How to Run

### 1. Compile

```
make
```

### 2. Load Kernel Module

```
sudo insmod monitor.ko
```

### 3. Create Device File

```
sudo mknod /dev/container_monitor c <major_number> 0
sudo chmod 666 /dev/container_monitor
```

### 4. Run Container

```
sudo ./engine start test ../rootfs-alpha "./memory_hog 2 1000" --soft-mib 3 --hard-mib 6
```

### 5. View Logs

```
dmesg | tail
```

---

##  Demonstrations

### 1. Multi-container Execution

![Step1](screenshots/step1.png)

### 2. Container Listing

![Step2](screenshots/step2.png)

### 3. Logging System

![Step3](screenshots/step3.png)

### 4. CLI Usage

![Step4](screenshots/step4.png)

### 5. Soft Limit Trigger

![Step5](screenshots/step5.png)

### 6. Hard Limit Enforcement

![Step6](screenshots/step6.png)

### 7. CPU Scheduling / Load

![Step7](screenshots/step7.png)

### 8. Cleanup / Teardown

![Step8](screenshots/step8.png)

---

## Conclusion

This project demonstrates how container runtimes work internally by combining user-space control with kernel-level monitoring. It provides a simplified understanding of Docker-like systems.

---

## 📎 Notes

* Root filesystem and large files are excluded from repository
* Tested on Ubuntu VirtualBox environment

---
