#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define ENTRY_NAME "tsc"

static inline unsigned long long read_tsc(void) {
  unsigned int lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)hi << 32) | lo;
}

ssize_t tsc_read(struct file *file, char __user *buf, size_t count,
                 loff_t *ppos) {
  char buffer[64];
  int len;
  unsigned long long tsc = read_tsc();

  len = snprintf(buffer, sizeof(buffer), "%llu\n", tsc);

  if (*ppos > 0)
    return 0;

  if (copy_to_user(buf, buffer, len))
    return -EFAULT;

  *ppos = len;
  return len;
}

static const struct proc_ops tsc_ops = {
    .proc_read = tsc_read,
};

static int __init tsc_init(void) {
  proc_create(ENTRY_NAME, 0444, NULL, &tsc_ops);
  printk(KERN_INFO "/proc/tsc created\n");
  return 0;
}

static void __exit tsc_exit(void) {
  remove_proc_entry(ENTRY_NAME, NULL);
  printk(KERN_INFO "/proc/tsc removed\n");
}

module_init(tsc_init);
module_exit(tsc_exit);
MODULE_LICENSE("GPL");
