#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/timer.h>

#include "monitor_ioctl.h"

#define DEVICE_NAME "container_monitor"

static int major;
static struct timer_list monitor_timer;

struct container_entry {
    struct list_head list;
    char id[32];
    pid_t pid;
    size_t soft_limit;
    size_t hard_limit;
};

static LIST_HEAD(container_list);

/* ---------------- IOCTL ---------------- */
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct container_info info;
    struct container_entry *entry;

    if (cmd == IOCTL_REGISTER) {

        if (copy_from_user(&info, (void __user *)arg, sizeof(info)))
            return -EFAULT;

        entry = kmalloc(sizeof(*entry), GFP_KERNEL);
        if (!entry)
            return -ENOMEM;

        strncpy(entry->id, info.id, sizeof(entry->id) - 1);
        entry->pid = info.pid;
        entry->soft_limit = info.soft_limit;
        entry->hard_limit = info.hard_limit;

        list_add(&entry->list, &container_list);

        printk(KERN_INFO "[monitor] Registered: %s pid=%d soft=%zu hard=%zu\n",
               entry->id, entry->pid, entry->soft_limit, entry->hard_limit);
    }

    return 0;
}

/* ---------------- MONITOR LOGIC ---------------- */
static void monitor_fn(struct timer_list *t)
{
    struct container_entry *entry;

    list_for_each_entry(entry, &container_list, list) {

        struct task_struct *task;
        unsigned long rss;

        task = pid_task(find_get_pid(entry->pid), PIDTYPE_PID);
        if (!task || !task->mm)
            continue;

        rss = get_mm_rss(task->mm) << PAGE_SHIFT;

        printk(KERN_INFO "[monitor] pid=%d rss=%lu\n", entry->pid, rss);

        /* SOFT LIMIT */
        if (rss >= entry->soft_limit) {
            printk(KERN_INFO "[monitor] SOFT LIMIT %s pid=%d\n",
                   entry->id, entry->pid);
        }

        /* HARD LIMIT */
        if (rss >= entry->hard_limit) {
            printk(KERN_INFO "[monitor] HARD LIMIT %s pid=%d -> killing\n",
                   entry->id, entry->pid);

            kill_pid(find_get_pid(entry->pid), SIGKILL, 1);
        }
    }

    /* run every 1 second */
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(1000));
}

/* ---------------- FILE OPS ---------------- */
static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
};

/* ---------------- INIT ---------------- */
static int __init monitor_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
        printk(KERN_ALERT "Failed to register device\n");
        return major;
    }

    timer_setup(&monitor_timer, monitor_fn, 0);
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(1000));

    printk(KERN_INFO "[monitor] Loaded. Major=%d\n", major);
    return 0;
}

/* ---------------- EXIT ---------------- */
static void __exit monitor_exit(void)
{
    del_timer(&monitor_timer);
    unregister_chrdev(major, DEVICE_NAME);

    printk(KERN_INFO "[monitor] Unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
MODULE_LICENSE("GPL");
