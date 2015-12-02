#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <net/sock.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/paravirt.h>
#include <linux/kprobes.h>
#include <asm/unistd.h>
#include <linux/slab.h>
//#include <asm/syscall.h> //???

#define SERVERPORT 5555
static struct socket *clientsocket=NULL;

unsigned long **sys_call_table;

struct kprobe **arr_kp = NULL;
int num_probes = 0;
//extern const sys_call_ptr_t sys_call_table[];
static struct kprobe ktest;


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
	/*
	int len;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_in to;
	mm_segment_t oldfs;
	char buf[64];
	
	memset(&to,0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = in_aton( "127.0.0.1" );  
	
	to.sin_port = htons( (unsigned short)
		SERVERPORT );
	memset(&msg,0,sizeof(msg));
	msg.msg_name = &to;
	msg.msg_namelen = sizeof(to);
	memcpy( buf, "hallo from kernel space", 24 );
	iov.iov_base = buf;
	iov.iov_len  = 24;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_iov    = &iov;
	msg.msg_iovlen = 1;
	

	oldfs = get_fs();
	set_fs( KERNEL_DS );
	len = sock_sendmsg( clientsocket, &msg, 24 );
	set_fs( oldfs );
	//printk(KERN_INFO "syscall #%d, addr = 0x%p\n",
    //    (void *)(probe->addr) - (void *)(sys_call_table[0]), probe->addr);
	*/
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
	//kprobe stuff
	int i=1;
	struct kprobe *kp;
	int ret;
	//socket stuff
	struct msghdr msg;
	struct iovec iov;
	mm_segment_t oldfs;
	struct sockaddr_in to;
	/*
	 * entry 0 is always a ni_syscall
	 */

	if(!(sys_call_table = aquire_sys_call_table()))
		return -1;
	 
	
	while(sys_call_table[i] != sys_call_table[__NR_seccomp]) {
		num_probes++;
		i++;
	}
	
	//init socket
	if( sock_create( PF_INET,SOCK_DGRAM,IPPROTO_UDP,&clientsocket)<0 ){
	printk( KERN_INFO "server: Error creating clientsocket.n" );
	return -EIO;
	}
	

	//more kprobe stuff
	printk(KERN_DEBUG "%d entries in table to probe\n", num_probes);
	arr_kp = kmalloc(sizeof(struct kprobe) * num_probes, GFP_KERNEL);
	for (i = 0; i < num_probes; i++) {
		arr_kp[i] = NULL;
	}
	
	for (i = 0; i < num_probes; i++) {
	
		if (sys_call_table[i+1] == sys_call_table[__NR_read]) printk(KERN_DEBUG "found sys_read, call #%d addr:%p \n", i+1, sys_call_table[i+1]);
		if (sys_call_table[i+1] == sys_call_table[__NR_open]) printk(KERN_DEBUG "found sys_open, call #%d addr:%p \n", i+1, sys_call_table[i+1]);
		
		if(sys_call_table[1+i] != sys_call_table[__NR_seccomp]) {
			kp = kmalloc(sizeof(struct kprobe), GFP_KERNEL); //may need to set fields to null...
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
	
	ktest.pre_handler = probe_pre_handler;
	ktest.addr = (void *)sys_call_table[3];
	printk(KERN_DEBUG "attempt register probe to addr %p", ktest.addr); 
	ret = register_kprobe(&ktest);
	if (!(ret < 0)) { 
		printk(KERN_DEBUG "registered probe to addr %p", ktest.addr); 
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
	unregister_kprobe(&ktest);
	if( clientsocket )    sock_release( clientsocket );
    printk(KERN_INFO "Closing Socket\n");
	
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