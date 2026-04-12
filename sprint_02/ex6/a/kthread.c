#include <linux/delay.h>
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/kthread.h>
#include <linux/module.h> /* Needed by all modules */
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define ENTRY_NAME "atomic_threads_a"

// threads
static struct task_struct *controller_th;
static struct task_struct *inc_th;
static struct task_struct *dec_th;

atomic_t nr;
static wait_queue_head_t wq;
static atomic_t work_ready;

static const struct proc_ops proc_ops = {
    .proc_open = NULL,
    .proc_read = NULL,
    .proc_write = NULL,
    .proc_release = NULL,
};
int proc_init(void);
void proc_exit(void);
int controller_fn(void *data);
int inc_fn(void *data);
int dec_fn(void *data);

int controller_fn(void *data) {
  while (!kthread_should_stop()) {
    msleep(1000);

    printk(KERN_INFO "Controller: waking up workers\n");

    atomic_inc_return(&work_ready);
    wake_up_interruptible(&wq);
  }
  return 0;
}

int inc_fn(void *data) {
  while (!kthread_should_stop()) {

    wait_event_interruptible(wq,
                             atomic_read(&work_ready) || kthread_should_stop());

    if (kthread_should_stop())
      break;

    printk(KERN_INFO "INC thread: value = %d\n", atomic_inc_return(&nr));
  }
  return 0;
}

int dec_fn(void *data) {
  while (!kthread_should_stop()) {

    wait_event_interruptible(wq,
                             atomic_read(&work_ready) || kthread_should_stop());

    if (kthread_should_stop())
      break;

    printk(KERN_INFO "DEC thread: value = %d\n", atomic_dec_return(&nr));

    atomic_dec_return(&work_ready);
  }
  return 0;
}

int proc_init(void) {
  struct proc_dir_entry *proc_entry = NULL;
  proc_entry = proc_create(ENTRY_NAME, 0666, NULL, &proc_ops);
  if (proc_entry == NULL)
    return -ENOMEM;

  init_waitqueue_head(&wq);

  controller_th = kthread_run(controller_fn, NULL, "controller_thread");
  inc_th = kthread_run(inc_fn, NULL, "inc_thread");
  dec_th = kthread_run(dec_fn, NULL, "dec_thread");
  atomic_set(&nr, 0);
  atomic_set(&work_ready, 0);

  printk(KERN_INFO "LKM: /proc/%s created\n", ENTRY_NAME);
  printk(KERN_INFO "LKM:%s:[%d] init: nr: %d\n", ENTRY_NAME, current->pid,
         atomic_read(&nr));
  return 0;
}

void proc_exit(void) {
  if (controller_th)
    kthread_stop(controller_th);

  if (inc_th)
    kthread_stop(inc_th);

  if (dec_th)
    kthread_stop(dec_th);

  remove_proc_entry(ENTRY_NAME, NULL);
  printk(KERN_INFO "LKM: Removing /proc/%s.\n", ENTRY_NAME);
  printk(KERN_INFO "LKM:%s:[%d] exit: nr: %d\n", ENTRY_NAME, current->pid,
         atomic_read(&nr));
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PBS");
MODULE_DESCRIPTION("The kernel thread example");
