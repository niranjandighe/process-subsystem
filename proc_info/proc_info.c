#include<linux/init.h>
#include<linux/module.h>
#include<linux/debugfs.h>
#include<linux/fs.h>

MODULE_LICENSE("GPL");

static struct dentry *dir;

static int ops_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t ops_read(struct file *file, char __user *buff, size_t len,
			loff_t *offset)
{
	return len;
}

static ssize_t ops_write(struct file *file, char const __user *buff, size_t len,
			loff_t *offset)
{
	return len;
}

static struct file_operations ops = {
	.open = ops_open,
	.read = ops_read,
	.write = ops_write
};

static int __init proc_info_init(void)
{
	dir = debugfs_create_dir("proc_info", NULL);

	if (!dir)
		return -ENODEV;

	debugfs_create_file("pid", 0222, dir, (void *) "pid", &ops);
	debugfs_create_file("info", 0444, dir, (void *) "info", &ops);
 
	return 0;
}

static void __exit  proc_info_exit(void)
{
	debugfs_remove_recursive(dir);
}

module_init(proc_info_init);
module_exit(proc_info_exit);

