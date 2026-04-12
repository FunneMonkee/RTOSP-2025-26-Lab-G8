#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#define ENTRY_NAME "atomic_threads_b"

static struct task_struct *waking_th;
static struct task_struct *writer_th;
static struct task_struct *reader_th;

static int nr;

static DECLARE_COMPLETION(wake_writer);
static DECLARE_COMPLETION(writer_done);
static const struct proc_ops proc_ops = {
    .proc_open = NULL,
    .proc_read = NULL,
    .proc_write = NULL,
    .proc_release = NULL,
};
int proc_init(void);
void proc_exit(void);
int waking_fn(void *data);
int writer_fn(void *data);
int reader_fn(void *data);

int waking_fn(void *data) {
  while (!kthread_should_stop()) {
    msleep(1000);

    printk(KERN_INFO "WAKING: signaling writer\n");

    reinit_completion(&wake_writer);
    complete(&wake_writer);
  }
  return 0;
}

int writer_fn(void *data) {
  while (!kthread_should_stop()) {

    wait_for_completion_interruptible(&wake_writer);

    if (kthread_should_stop())
      break;

    nr++;
    printk(KERN_INFO "WRITER: incremented value = %d\n", nr);

    reinit_completion(&wake_writer);
    complete(&writer_done);
  }
  return 0;
}

int reader_fn(void *data) {
  while (!kthread_should_stop()) {

    wait_for_completion_interruptible(&writer_done);

    if (kthread_should_stop())
      break;

    printk(KERN_INFO "READER: read value = %d\n", nr);
  }
  return 0;
}

int proc_init(void) {
  struct proc_dir_entry *proc_entry = NULL;
  proc_entry = proc_create(ENTRY_NAME, 0666, NULL, &proc_ops);
  if (proc_entry == NULL)
    return -ENOMEM;

  init_completion(&wake_writer);
  init_completion(&writer_done);

  waking_th = kthread_run(waking_fn, NULL, "waking_thread");
  writer_th = kthread_run(writer_fn, NULL, "writer_thread");
  reader_th = kthread_run(reader_fn, NULL, "reader_thread");

  printk(KERN_INFO "LKM: /proc/%s created\n", ENTRY_NAME);
  printk(KERN_INFO "LKM:%s:[%d] init: nr: %d\n", ENTRY_NAME, current->pid, nr);
  return 0;
}

void proc_exit(void) {
  if (waking_th)
    kthread_stop(waking_th);

  if (writer_th)
    kthread_stop(writer_th);

  if (reader_th)
    kthread_stop(reader_th);

  remove_proc_entry(ENTRY_NAME, NULL);
  printk(KERN_INFO "LKM: Removing /proc/%s.\n", ENTRY_NAME);
  printk(KERN_INFO "LKM:%s:[%d] exit: nr: %d\n", ENTRY_NAME, current->pid, nr);
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PBS");
MODULE_DESCRIPTION("The kernel thread example");
