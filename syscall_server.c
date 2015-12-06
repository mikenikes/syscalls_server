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
#include <linux/fdtable.h>
#include <linux/string.h>

#define SERVERPORT 15555
#define SERVERADDR "127.0.0.1"
static struct socket *clientsocket = NULL;

unsigned long **sys_call_table;
int num_probes = 0;
char strbuf[256];

static struct jprobe jp_open;
static struct jprobe jp_close;


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

static void send_udp_msg(char* msgstr, int msglen) {
	struct msghdr msg;
	struct iovec iov;
	mm_segment_t oldfs;
	struct sockaddr_in to;
	
	memset(&to,0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = in_aton( SERVERADDR );  
	to.sin_port = htons( (unsigned short) SERVERPORT );
	memset(&msg,0,sizeof(msg));
	msg.msg_name = &to;
	msg.msg_namelen = sizeof(to);
	iov.iov_base = msgstr;
	iov.iov_len  = msglen;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_iov    = &iov;
	msg.msg_iovlen = 1;
	oldfs = get_fs();
	set_fs( KERNEL_DS );
	sock_sendmsg( clientsocket, &msg, msglen );
	set_fs( oldfs );
}

static asmlinkage long jp_open_entry(const char __user *filename, int flags, umode_t mode) {
	char *slowname;
	slowname = kmalloc(10 * sizeof(char), GFP_KERNEL);
	if (!slowname) { 
		printk(KERN_ERR "alloc failed");
		goto JPO_ERR;
	}
	
	//slow down call if we open this specific file
	strcpy(slowname,"client.js");
	if (strstr(filename,slowname)){
		sprintf(strbuf "!!!!!!SLOWING DOWN OPENING client.js\n");
		send_udp_msg(strbuf,strlen(strbuf));
		msleep(5000);
	}
	
	//printk(KERN_ERR "open - filename: %s, flags: %d, modes: %d\n", filename, flags, mode);
	sprintf(strbuf, "open - filename: %s, flags: %d, modes: %d\n", filename, flags, mode);
	send_udp_msg(strbuf,strlen(strbuf));
	kfree(slowname);
	JPO_ERR: jprobe_return();
	return 0;
}

static asmlinkage long jp_close_entry(unsigned int fd) {
	struct files_struct *files = current->files;
	char *tmp;
	char *pathname;
	struct file *file;
	struct path *path;
	
	spin_lock(&files->file_lock);
	file = fcheck_files(files, fd);
	if (!file) {
		spin_unlock(&files->file_lock);
		goto JPE_ERR;
	}
	
	path = &file->f_path;
	path_get(path);
	spin_unlock(&files->file_lock);
	
	tmp = (char *)__get_free_page(GFP_TEMPORARY);
	
	if (!tmp) {
		path_put(path);
		goto JPE_ERR;
	}

	pathname = d_path(path, tmp, PAGE_SIZE);
	path_put(path);
	
	if (IS_ERR(pathname)) {
		free_page((unsigned long)tmp);
		goto JPE_ERR;
	}

	/* do something here with pathname */
	printk(KERN_ERR "close - fd: %d, filename: %s\n", fd, pathname);

	free_page((unsigned long)tmp);
	
	JPE_ERR: jprobe_return();
	return 0; 
}

static int __init syscall_server_start(void) 
{
	//kprobe stuff
	int i=1;
	int ret;

	//get syscall table
	if(!(sys_call_table = aquire_sys_call_table()))
		return -1;
	 
	while(sys_call_table[i] != sys_call_table[__NR_seccomp]) {
		num_probes++;
		i++;
	}
	
	//init socket
	if( sock_create( PF_INET,SOCK_DGRAM,IPPROTO_UDP,&clientsocket) < 0 ){
		printk( KERN_INFO "server: Error creating clientsocket.n" );
		return -EIO;
	}
	else {
		printk(KERN_INFO "success making socket");
	}
	
	//jprobe for open/close
	jp_open.kp.addr = (void *) sys_call_table[__NR_open];
	jp_open.entry = jp_open_entry;
	
	jp_close.kp.addr = (void *) sys_call_table[__NR_close];
	jp_close.entry = jp_close_entry;

	ret = register_jprobe(&jp_open);
	if (!(ret < 0)) { 
		printk(KERN_DEBUG "registered probe to addr %p\n", jp_open.kp.addr); 
	}
	else {
		printk(KERN_DEBUG "unsucessful registering ret=%d\n",ret);
		if (ret == -EINVAL) printk(KERN_DEBUG "einvalue error in jprobe\n");
		return ret;
	}
	ret = register_jprobe(&jp_close);
	if (!(ret < 0)) { 
		printk(KERN_DEBUG "registered probe to addr %p\n", jp_close.kp.addr); 
	}
	else {
		printk(KERN_DEBUG "unsucessful registering ret=%d\n",ret);
		if (ret == -EINVAL) printk(KERN_DEBUG "einvalue error in jprobe\n");
		return ret;
	}
	return 0;
}

static void __exit syscall_server_end(void) 
{

	unregister_jprobe(&jp_open);
	unregister_jprobe(&jp_close);
	if( clientsocket )    sock_release( clientsocket );
    printk(KERN_INFO "Closing Socket\n");
	
	
	printk(KERN_DEBUG "uninstalling syscall_server LKM\n");
	
}

module_init(syscall_server_start);
module_exit(syscall_server_end);

MODULE_LICENSE("GPL");