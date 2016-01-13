#include<linux/init.h>
#include<linux/module.h>
#include<linux/debugfs.h>
#include<linux/fs.h>
#include<linux/seq_file.h>

MODULE_LICENSE("GPL");

static struct dentry *dir;

static ssize_t info_show(struct seq_file *m, void *data)
{
	seq_printf(m, "read claled");
	return 0;
}

static int info_open(struct inode *inode, struct file *file)
{
	return single_open(file, info_show, NULL);
}

static ssize_t pid_write(struct file *file, char const __user *buff, size_t len,
			loff_t *offset)
{
	return len;
}

static struct file_operations pid_ops = {
	.write = pid_write
};

static struct file_operations info_ops = {
	.open = info_open,
	.read = seq_read
};

static int __init proc_info_init(void)
{
	dir = debugfs_create_dir("proc_info", NULL);

	if (!dir)
		return -ENODEV;

	debugfs_create_file("pid", 0222, dir, (void *) "pid", &pid_ops);
	debugfs_create_file("info", 0444, dir, (void *) "info", &info_ops);
 
	return 0;
}

static void __exit  proc_info_exit(void)
{
	debugfs_remove_recursive(dir);
}

module_init(proc_info_init);
module_exit(proc_info_exit);

