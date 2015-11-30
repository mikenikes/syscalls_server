#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/paravirt.h>
#include <linux/kprobes.h>
#include <asm/unistd.h>
#include <linux/slab.h>
//#include <asm/syscall.h> //how the fuck?!

unsigned long **sys_call_table;

struct kprobe **arr_kp = NULL;
int num_probes = 0;
//extern const sys_call_ptr_t sys_call_table[];


static unsigned long **aquire_sys_call_table(void)
{
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;

	while (offset < ULLONG_MAX) {
		sct = (unsigned long **)offset;

		if (sct[__NR_close] == (unsigned long *) sys_close) 
			return sct;

		offset += sizeof(void *);
	}
	
	return NULL;
}

static int probe_pre_handler(struct kprobe *probe, struct pt_regs *regs) {
	printk(KERN_INFO "pre_handler: p->addr = 0x%p, ip = %lx,"
		" flags = 0x%lx\n",
        probe->addr, regs->ip, regs->flags);

	return 0;
}

static int probe_fault_handler(struct kprobe *probe,
			       struct pt_regs *regs, int trap)
{
	printk(KERN_DEBUG "probe_fault_handler: addr %p, trap %i\n",
	       probe->addr, trap);
	return 0;
}

static int __init syscall_server_start(void) 
{
	int i=1;
	struct kprobe *kp;
	int ret;
	/*
	 * entry 0 is always a ni_syscall
	 */

	if(!(sys_call_table = aquire_sys_call_table()))
		return -1;
	 
	
	while(sys_call_table[i] != sys_call_table[__NR_seccomp]) {
		num_probes++;
		i++;
	}
	

	printk(KERN_DEBUG "%d entries in table to probe\n", num_probes);
	arr_kp = kmalloc(sizeof(struct kprobe) * num_probes, GFP_KERNEL);
	for (i = 0; i < num_probes; i++) {
		arr_kp[i] = NULL;
	}
	
	for (i = 0; i < num_probes; i++) {
	
		if (sys_call_table[i+1] == sys_call_table[__NR_read]) printk(KERN_DEBUG "found sys_read, call #%d addr:%p \n", i+1, sys_call_table[i+1]);
		if (sys_call_table[i+1] == sys_call_table[__NR_open]) printk(KERN_DEBUG "found sys_open, call #%d addr:%p \n", i+1, sys_call_table[i+1]);
		
		if(sys_call_table[1+i] != sys_call_table[__NR_seccomp]) {
			kp = kmalloc(sizeof(struct kprobe), GFP_KERNEL);
			kp->pre_handler = probe_pre_handler;
			//kp->fault_handler = probe_fault_handler;
			kp->addr = (void *)sys_call_table[1+i];
			arr_kp[i] = kp;
			printk(KERN_DEBUG "adding probe #%d to syscall %d, addr:%p\n", i, i+1, sys_call_table[i+1]);
			/*
			ret = register_kprobe(arr_kp[i]);
			if (ret < 0) printk(KERN_DEBUG "registered probe %d", i); 
			else {
				printk(KERN_DEBUG "unsucessful registering ret=%d",ret);
				return ret;
			}
			*/
		}
	}
	printk(KERN_DEBUG "attempt register probe to addr %p", arr_kp[0]->addr); 
	ret = register_kprobe(arr_kp[0]);
	if (!(ret < 0)) { 
		printk(KERN_DEBUG "registered probe to addr %p", arr_kp[0]->addr); 
	}
	else {
		printk(KERN_DEBUG "unsucessful registering ret=%d",ret);
		if (ret == -EINVAL) printk(KERN_DEBUG "you dun fucked up kprobe code");
		return ret;
	}
	

	return 0;
}

static void __exit syscall_server_end(void) 
{
	/*
	 * entry 0 is always a ni_syscall
	 */
	int i;
	if(!sys_call_table) {
		printk(KERN_DEBUG "uninstalling syscall_server LKM, wtfmode\n");
		return;
	}
	
	printk(KERN_DEBUG "uninstalling syscall_server LKM\n");
	/*
	for (i = 0; i < num_probes; i++) {
		if (arr_kp[i]) unregister_kprobe(arr_kp[i]);
	}
	*/
	
	/*
	write_cr0(original_cr0 & ~0x00010000);
	sys_call_table[__NR_read] = (unsigned long *)ref_sys_read;
	write_cr0(original_cr0);
	*/
	msleep(2000);
	
}

module_init(syscall_server_start);
module_exit(syscall_server_end);

MODULE_LICENSE("GPL");