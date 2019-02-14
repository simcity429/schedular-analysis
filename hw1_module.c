#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ktime.h>

#define procfs_name "hw1"
#define USE_SINGLE_OPEN

struct task_struct *task;


static void print_bar(struct seq_file *s){
    int i;
    for (i=0;i<120;i++){
	seq_printf(s, "-");
    }
    seq_printf(s, "\n");
}

static int my_show(struct seq_file *s, void *unused){
    int proc_cnt = 0;
    int constant = 1000/HZ;
    int system_uptime = (jiffies-INITIAL_JIFFIES)*constant;
    int start_time, start_time_ms, total_time, total_time_ms, user_time, user_time_ms, kernel_time, kernel_time_ms; 
    int user_time_mil, kernel_time_mil, total_time_mil;
    printk(KERN_INFO "my_show called\n");
    print_bar(s);
    seq_printf(s, "CURRENT SYSTEM INFORMATION >\n");
    for_each_process(task){
	proc_cnt++;
    }
    seq_printf(s, "Total %d tasks\n", proc_cnt);
    seq_printf(s, "%dHz, %d ms after system boot time\n", HZ, system_uptime);
    print_bar(s);
    seq_printf(s, "%19s%8s%13s%13s%13s%13s\n", "command", "pid", "start(s)","total(s)", "user(s)", "kernel(s)");
    print_bar(s);
    for_each_process(task){
	start_time = (int)(task->start_time.tv_sec);
	start_time_ms = (task->start_time.tv_nsec)/1000000;
	user_time_mil = task->utime * constant;
	kernel_time_mil = task->stime * constant;
	total_time_mil = user_time_mil + kernel_time_mil;
	user_time = user_time_mil / 1000;
	kernel_time = kernel_time_mil / 1000;
	user_time_ms = user_time_mil % 1000;
	kernel_time_ms = kernel_time_mil % 1000;
	total_time = total_time_mil / 1000;
	total_time_ms = total_time_mil % 1000;
	seq_printf(s, "%19s%8d%9d.%03d%9d.%03d%9d.%03d%9d.%03d\n", task->comm, task->pid, start_time, start_time_ms, total_time, total_time_ms, user_time, user_time_ms, kernel_time, kernel_time_ms);
    }
    return 0;
}

static int my_proc_open(struct inode *inode, struct file *file){
    return single_open(file, my_show, NULL);
}

static const struct file_operations my_proc_ops = {
    .owner = THIS_MODULE,
    .open = my_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};

int init_module(){
    struct proc_dir_entry *Our_Proc_File;
    Our_Proc_File = create_proc_entry(procfs_name, 0644, NULL);
    if (Our_Proc_File == NULL){
	remove_proc_entry(procfs_name, &proc_root);
	printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", procfs_name);
	return -ENOMEM;
    }

    Our_Proc_File->proc_fops = &my_proc_ops;
    printk(KERN_INFO "/proc/%s created\n", procfs_name);
    return 0;
}

void cleanup_module(){
    remove_proc_entry(procfs_name, &proc_root);
    printk(KERN_INFO "successfully removed\n");
}
