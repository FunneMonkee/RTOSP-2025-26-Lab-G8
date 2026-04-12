#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

#define ENTRY_NAME "ring_buffer"
#define BUFFER_LEN 100
#define QUEUE_LEN 5

static struct semaphore empty;
static struct semaphore full;
static struct semaphore mutex;
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
  strcpy(ring.queue[ring.write_item].buffer, buffer);
  increment(&ring.write_item);
  return 1;
}

int proc_open(struct inode *inode, struct file *filp) {
  printk(KERN_INFO "LKM: %s:[%d] open\n", ENTRY_NAME, current->pid);
  return 0;
}
ssize_t proc_read(struct file *filp, char __user *buf, size_t count,
                  loff_t *f_pos) {
  char buffer[BUFFER_LEN];
  int ret = 0, len = 0;

  // wait
  if (down_interruptible(&full))
    return -ERESTARTSYS;

  if (*f_pos > len) {
    *f_pos = 0;
    return 0;
  }

  // critical section
  if (down_interruptible(&mutex)) {
    up(&full);
    return -ERESTARTSYS;
  }
  printk(KERN_INFO "LKM: %s:[%d] read\n", ENTRY_NAME, current->pid);

  ret = dequeue(buffer);
  up(&mutex);
  up(&empty);

  if (ret <= 0)
    return ret;
  len = strlen(buffer);
  if (len <= 0)
    return -EFAULT;
  if (count < len)
    return -EFAULT;
  ret = raw_copy_to_user(buf, buffer, len);

  if (ret != 0)
    return -EFAULT;
  *f_pos += count - len;

  return len;
}
ssize_t proc_write(struct file *filp, const char *buf, size_t count,
                   loff_t *f_pos) {
  int ret;
  char buffer[BUFFER_LEN];
  if (count > BUFFER_LEN)
    return -EINVAL;
  ret = raw_copy_from_user(buffer, buf, count);
  if (ret != 0)
    return -EFAULT;
  buffer[count] = 0;

  // wait
  if (down_interruptible(&empty))
    return -ERESTARTSYS;

  // critical section
  if (down_interruptible(&mutex)) {
    up(&empty);
    return -ERESTARTSYS;
  }
  printk(KERN_INFO "LKM: %s:[%d] write\n", ENTRY_NAME, current->pid);

  ret = enqueue(buffer);

  if (ret <= 0)
    return ret;

  up(&mutex);
  up(&full);
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

  sema_init(&empty, QUEUE_LEN);
  sema_init(&full, 0);
  sema_init(&mutex, 1);
  ring.write_item = 0;
  ring.read_item = 0;

  return 0;
}

void proc_exit(void) {
  remove_proc_entry(ENTRY_NAME, NULL);
  printk(KERN_INFO "LKM: /proc/%s removed\n", ENTRY_NAME);
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PBS");
MODULE_DESCRIPTION("Ring buffer implementation");
