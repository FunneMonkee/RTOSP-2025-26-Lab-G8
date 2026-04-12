#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/rbtree.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define BUFFER_LEN 100
#define ENTRY_NAME "sorted_queue"

static DEFINE_MUTEX(tree_mutex);

struct queue_item {
  char buffer[BUFFER_LEN];
  struct rb_node node;
};
static int pop(char *buffer);
static int push(char *buffer);

int proc_open(struct inode *inode, struct file *filp);
ssize_t proc_read(struct file *filp, char __user *buf, size_t count,
                  loff_t *f_pos);
ssize_t proc_write(struct file *filp, const char *buf, size_t count,
                   loff_t *f_pos);
int proc_close(struct inode *inode, struct file *filp);

int proc_init(void);
void proc_exit(void);

struct rb_root root = RB_ROOT;
struct queue_item *first = NULL;

struct proc_dir_entry *proc_entry = NULL;

static int pop(char *buffer) {
  struct rb_node *node;
  struct queue_item *elem;

  mutex_lock(&tree_mutex);
  if (first == NULL) {
    mutex_unlock(&tree_mutex);
    return 0;
  }

  elem = first;

  strcpy(buffer, elem->buffer);

  first = NULL;
  node = rb_next(&elem->node);
  if (node) {
    first = rb_entry(node, struct queue_item, node);
  }
  rb_erase(&elem->node, &root);

  printk(KERN_INFO "LKM: %s:[%d] pop\n", ENTRY_NAME, current->pid);
  mutex_unlock(&tree_mutex);
  kfree(elem);

  return 1;
}
static int push(char *buffer) {
  struct rb_node **node;
  struct rb_node *parent = NULL;
  struct queue_item *entry;
  int is_first = 1;

  struct queue_item *elem = kmalloc(sizeof(struct queue_item), GFP_KERNEL);
  if (!elem)
    return 0;
  strcpy(elem->buffer, buffer);

  mutex_lock(&tree_mutex);

  node = &root.rb_node;

  while (*node != NULL) {
    parent = *node;
    entry = rb_entry(parent, struct queue_item, node);
    // if((char)(elem->buffer[0]) < (char)(entry->buffer[0])){
    if ((char)(*elem->buffer) < (char)(*entry->buffer)) {
      node = &parent->rb_left;
    } else {
      node = &parent->rb_right;
    }
  }
  rb_link_node(&elem->node, parent, node);
  rb_insert_color(&elem->node, &root);

  printk(KERN_INFO "LKM: %s:[%d] push\n", ENTRY_NAME, current->pid);
  first = rb_entry(rb_first(&root), struct queue_item, node);

  mutex_unlock(&tree_mutex);
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
  printk(KERN_INFO "LKM: %s:[%d] read\n", ENTRY_NAME, current->pid);

  if (*f_pos > 0)
    return 0;
  if (!pop(buffer))
    return 0;
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
  printk(KERN_INFO "LKM: %s:[%d] write\n", ENTRY_NAME, current->pid);
  if (count > BUFFER_LEN)
    return -EINVAL;
  ret = raw_copy_from_user(buffer, buf, count);
  if (ret != 0)
    return -EFAULT;
  buffer[count] = 0;
  if (!push(buffer))
    return -EFAULT;
  return count;
}

int proc_close(struct inode *inode, struct file *filp) {
  printk(KERN_INFO "LKM: %s:[%d] release\n", ENTRY_NAME, current->pid);
  return 0;
}
static const struct proc_ops proc_ops = {
    .proc_open = proc_open,
    .proc_read = proc_read,
    .proc_write = proc_write,
    .proc_release = proc_close,
};
int proc_init(void) {
  proc_entry = proc_create(ENTRY_NAME, 0, NULL, &proc_ops);
  if (proc_entry == NULL)
    return -ENOMEM;

  printk(KERN_INFO "LKM: /proc/%s created\n", ENTRY_NAME);
  return 0;
}

void proc_exit(void) {
  struct queue_item *tmp;
  struct rb_node *node;
  remove_proc_entry(ENTRY_NAME, NULL);
  mutex_lock(&tree_mutex);

  printk(KERN_INFO "LKM: /proc/%s removed\n", ENTRY_NAME);
  node = rb_first(&root);
  while (node != NULL) {
    tmp = rb_entry(node, struct queue_item, node);
    rb_erase(node, &root);
    kfree(tmp);
    node = rb_first(&root);
  }

  mutex_unlock(&tree_mutex);
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PBS");
MODULE_DESCRIPTION("Sorted queue implementation");
