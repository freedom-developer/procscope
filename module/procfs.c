#include "common.h"

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/seq_file.h>

#include "procfs.h"

static struct proc_dir_entry *procscope_proc_root;
static const char procscope_proc_root_name[] = "procscope";

static int thread_ids_show(struct seq_file *m, void *data)
{
	struct task_struct *thread = (struct task_struct *)m->private;
	if (!thread)
		return -EINVAL;

	seq_printf(m, "pid: %d, tgid: %d, ppid: %d, pgid: %d, sid: %d\n", 
				task_pid_nr(thread), task_tgid_nr(thread), task_ppid_nr_ns(thread, NULL), 
				task_pgrp_vnr(thread), task_session_vnr(thread));

	
	return 0;
}

static int create_thread_files (struct task_struct *thread, struct proc_dir_entry *thread_dir)
{
	// 展示各种ID: 
	if (!proc_create_single_data("ids", 0444, thread_dir, thread_ids_show, (void *)thread)) {
		pr_err("create thread ids file failed");
		return -EINVAL;
	}

	return 0;
}

static int create_process_proc_files(struct task_struct *process, struct proc_dir_entry *process_dir)
{
	int ret;


	if (atomic_read(&process->signal->live) == 1 || process->flags & PF_KTHREAD)
		return 0;

	struct proc_dir_entry *tsk_threads_proc_dir = proc_mkdir("", process_dir);
	if (!tsk_threads_proc_dir) {
		pr_err("create process proc files failed");
		return -EINVAL;
	}

	if (create_thread_files(process, process_dir)) {

	}
}

int create_procscope_procfs(void)
{
	struct task_struct *process;

	if (procscope_proc_root) {
		pr_warn("procscope procfs root already exists\n");
		return 0;
	}

	procscope_proc_root = proc_mkdir(procscope_proc_root_name, NULL);
	if (!procscope_proc_root) {
		pr_err("failed to create procfs root\n");
		return -EINVAL;
	}

	for_each_process(process) {
		struct task_struct *thread;
		int tgid = task_tgid_nr(process);
		char tsk_name[16];
		struct proc_dir_entry *tsk_proc_dir;

		snprintf(tsk_name, sizeof(tsk_name), "%d", tgid);
		tsk_proc_dir = proc_mkdir(tsk_name, procscope_proc_root);
		if (!tsk_proc_dir) {
			pr_err("create proc directory /proc/procscope/%s failed\n", tsk_name);
			destroy_procscope_procfs();
			return -EINVAL;
		}

		if (create_process_proc_files(process, tsk_proc_dir)) {
			pr_err("create process proc files")
		}

		for_each_thread(process, thread) {
			struct proc_dir_entry *thread_proc_dir;
			char thread_name[16];

			snprintf(thread_name, sizeof(thread_name), "%d", task_pid_nr(thread));
			thread_proc_dir = proc_mkdir(thread_name, tsk_tasks_proc_dir);
			if (!thread_proc_dir) {
				pr_err("create proc directory /proc/procscope/%s/tasks/%s failed\n",
				       tsk_name, thread_name);
				destroy_procscope_procfs();
				return -EINVAL;
			}

			int ret = create_thread_files(thread, thread_proc_dir);
			if (ret) {
				destroy_procscope_procfs();
				return ret;
			}

		}
	}

	return 0;
}

void destroy_procscope_procfs(void)
{
	if (!procscope_proc_root)
		return;

	proc_remove(procscope_proc_root);
	procscope_proc_root = NULL;
}
