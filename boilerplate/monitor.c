#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include<monitor_ioctl.h>

#define DEVICE_NAME "container_monitor"


static int major;
static struct timer_list monitor_timer;

struct container_entry {
    char id[64];
    pid_t pid;
    unsigned long soft_limit;
    unsigned long hard_limit;
    int soft_triggered;
    struct list_head list;
};



static LIST_HEAD(container_list);

static long get_rss(pid_t pid)
{
    struct task_struct *task;
    struct mm_struct *mm;
    long rss = -1;

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task) {
        rcu_read_unlock();
        return -1;
    }

    mm = get_task_mm(task);
    rcu_read_unlock();

    if (!mm)
        return -1;

    rss = (get_mm_rss(mm) + get_mm_counter(mm, MM_FILEPAGES)) << PAGE_SHIFT;

    mmput(mm);
    return rss;
}

static void log_soft(char *id, pid_t pid, unsigned long soft, long rss)
{
    printk(KERN_INFO "[container_monitor] SOFT LIMIT: %s pid=%d rss=%ld soft=%lu\n",
           id, pid, rss, soft);
}

static void kill_proc(char *id, pid_t pid, unsigned long hard, long rss)
{
    printk(KERN_INFO "[container_monitor] HARD LIMIT: killing %s pid=%d rss=%ld hard=%lu\n",
           id, pid, rss, hard);

    kill_pid(find_vpid(pid), SIGKILL, 1);
}

static void monitor_fn(struct timer_list *t)
{
    struct container_entry *e, *tmp;

    list_for_each_entry_safe(e, tmp, &container_list, list) {

        long rss = get_rss(e->pid);

        printk(KERN_INFO "[DEBUG] pid=%d rss=%ld soft=%lu hard=%lu\n",
               e->pid, rss, e->soft_limit, e->hard_limit);

        if (rss <= 0) {
            list_del(&e->list);
            kfree(e);
            continue;
        }

        // 🔥 SOFT LIMIT (force visible)
        if (!e->soft_triggered && rss >= (e->soft_limit / 2)) {
            log_soft(e->id, e->pid, e->soft_limit, rss);
            e->soft_triggered = 1;
        }

        // 🔥 HARD LIMIT (real kill)
        if (rss >= e->hard_limit) {
            kill_proc(e->id, e->pid, e->hard_limit, rss);
            list_del(&e->list);
            kfree(e);
            continue;
        }
    }

    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(1000));
}

static ssize_t device_write(struct file *file, const char __user *buf,
                            size_t len, loff_t *off)
{
    struct container_entry *e;

    // simple mock: register dummy container
    e = kmalloc(sizeof(*e), GFP_KERNEL);
    if (!e)
        return -ENOMEM;

    snprintf(e->id, sizeof(e->id), "test");
    e->pid = current->pid;
    e->soft_limit = 10 * 1024 * 1024;
    e->hard_limit = 50 * 1024 * 1024;
    e->soft_triggered = 0;

    list_add(&e->list, &container_list);

    printk(KERN_INFO "[container_monitor] Registered container pid=%d\n", e->pid);

    return len;
}

#define IOCTL_REGISTER 1


static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct monitor_request req;
    struct container_entry *entry;

    if (cmd != MONITOR_REGISTER)
        return -EINVAL;

    if (copy_from_user(&req, (void __user *)arg, sizeof(req)))
        return -EFAULT;

    entry = kmalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry)
        return -ENOMEM;

    strcpy(entry->id, req.container_id);
    entry->pid = req.pid;
    entry->soft_limit = req.soft_limit;
    entry->hard_limit = req.hard_limit;
    entry->soft_triggered = 0;

    list_add(&entry->list, &container_list);

    printk(KERN_INFO "[container_monitor] REGISTER %s pid=%d soft=%lu hard=%lu\n",
           entry->id, entry->pid, entry->soft_limit, entry->hard_limit);

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = device_ioctl,
};

static int __init monitor_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);

    printk(KERN_INFO "[container_monitor] Module loaded. Major=%d\n", major);

    timer_setup(&monitor_timer, monitor_fn, 0);
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(1000));

    return 0;
}

static void __exit monitor_exit(void)
{
   // del_timer(&monitor_timer);
    unregister_chrdev(major, DEVICE_NAME);

    printk(KERN_INFO "[container_monitor] Module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Container Memory Monitor");
