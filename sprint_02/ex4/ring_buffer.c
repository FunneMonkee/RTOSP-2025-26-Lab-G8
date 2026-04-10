#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/timer.h>

#define ENTRY_NAME "ring_buffer"
#define BUFFER_LEN 100
#define QUEUE_LEN 5

static spinlock_t ring_lock;
static struct timer_list timer;
static char last_message[BUFFER_LEN] = "no message";

struct queue_item {
  char buffer[BUFFER_LEN];
};

struct ring {
  struct queue_item queue[QUEUE_LEN];
  int write_item;
  int read_item;
};

static int increment(int *item);
static int is_empty(int r, int w);
static int is_full(int r, int w);
static int enqueue(char *buffer);
static int dequeue(char *buffer);

static void timer_callback(struct timer_list *t);

int proc_open(struct inode *inode, struct file *filp);
ssize_t proc_read(struct file *filp, char __user *buf, size_t count,
                  loff_t *f_pos);
ssize_t proc_write(struct file *filp, const char *buf, size_t count,
                   loff_t *f_pos);
int proc_close(struct inode *inode, struct file *filp);

int proc_init(void);
void proc_exit(void);

struct ring ring;

struct proc_dir_entry *proc_entry = NULL;

static int increment(int *item) {
  int ret;
  ret = *item;
  *item = (*item + 1) % QUEUE_LEN;
  return ret;
}
static int is_empty(int r, int w) {
  int ret;
  ret = !(r ^ w); // xor
  return ret;
}
static int is_full(int r, int w) {
  int ret, write;
  write = (w + 1) % QUEUE_LEN;
  ret = (write == r);
  return ret;
}
static int dequeue(char *buffer) {
  int ret = 0;
  if (!is_empty(ring.read_item, ring.write_item)) { // if it is not empty
    strcpy(buffer, ring.queue[ring.read_item].buffer);
    increment(&ring.read_item);
    ret = 1;
  }
  return ret;
}
static int enqueue(char *buffer) {
  if (is_full(ring.read_item, ring.write_item))
    increment(&ring.read_item); // remove from buffer
  strcpy(ring.queue[ring.write_item].buffer, buffer);
  increment(&ring.write_item);
  return 1;
}

static void timer_callback(struct timer_list *t) {
  char buffer[BUFFER_LEN];
  int ret;
  unsigned long flags;

  spin_lock_irqsave(&ring_lock, flags);

  ret = dequeue(buffer);

  if (ret > 0) {
    strcpy(last_message, buffer);
    printk(KERN_INFO "dequeued: %s\n", buffer);
  } else {
    printk(KERN_INFO "no new message, last: %s\n", last_message);
  }

  spin_unlock_irqrestore(&ring_lock, flags);

  mod_timer(&timer, jiffies + msecs_to_jiffies(1000));
}

int proc_open(struct inode *inode, struct file *filp) {
  printk(KERN_INFO "LKM: %s:[%d] open\n", ENTRY_NAME, current->pid);
  return 0;
}

ssize_t proc_read(struct file *filp, char __user *buf, size_t count,
                  loff_t *f_pos) {
  int len;

  len = strlen(last_message);

  if (*f_pos > 0)
    return 0;

  if (count < len)
    return -EINVAL;

  if (raw_copy_to_user(buf, last_message, len))
    return -EFAULT;

  *f_pos = len;
  return len;
}

ssize_t proc_write(struct file *filp, const char *buf, size_t count,
                   loff_t *f_pos) {
  char buffer[BUFFER_LEN];
  unsigned long flags;

  if (count > BUFFER_LEN - 1)
    return -EINVAL;

  if (raw_copy_from_user(buffer, buf, count))
    return -EFAULT;

  buffer[count] = '\0';

  spin_lock_irqsave(&ring_lock, flags);
  printk(KERN_INFO "LKM: write [%d]\n", current->pid);
  enqueue(buffer);
  spin_unlock_irqrestore(&ring_lock, flags);

  return count;
}

int proc_close(struct inode *inode, struct file *filp) {
  printk(KERN_INFO "LKM: %s:[%d] close\n", ENTRY_NAME, current->pid);
  return 0;
}
static const struct proc_ops proc_ops = {
    .proc_open = proc_open,
    .proc_read = proc_read,
    .proc_write = proc_write,
    .proc_release = proc_close,
};
int proc_init(void) {
  proc_entry = proc_create(ENTRY_NAME, 0666, NULL, &proc_ops);
  if (proc_entry == NULL)
    return -ENOMEM;

  printk(KERN_INFO "LKM: /proc/%s created\n", ENTRY_NAME);

  spin_lock_init(&ring_lock);
  ring.write_item = 0;
  ring.read_item = 0;

  timer_setup(&timer, timer_callback, 0);
  mod_timer(&timer, jiffies + msecs_to_jiffies(1000));

  return 0;
}

void proc_exit(void) {
  del_timer_sync(&timer);
  remove_proc_entry(ENTRY_NAME, NULL);
  printk(KERN_INFO "LKM: /proc/%s removed\n", ENTRY_NAME);
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PBS");
MODULE_DESCRIPTION("Ring buffer implementation");
