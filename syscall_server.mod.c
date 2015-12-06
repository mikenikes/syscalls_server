#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x135dd1a3, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x97053b4e, __VMLINUX_SYMBOL_STR(sock_release) },
	{ 0x70201233, __VMLINUX_SYMBOL_STR(unregister_jprobe) },
	{ 0x1f7f6fac, __VMLINUX_SYMBOL_STR(register_jprobe) },
	{ 0x83b11e0f, __VMLINUX_SYMBOL_STR(sock_create) },
	{        0, __VMLINUX_SYMBOL_STR(sys_close) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x7809656a, __VMLINUX_SYMBOL_STR(sock_sendmsg) },
	{ 0x1b6314fd, __VMLINUX_SYMBOL_STR(in_aton) },
	{ 0xd0d8621b, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x1e6d26a8, __VMLINUX_SYMBOL_STR(strstr) },
	{ 0xe914e41e, __VMLINUX_SYMBOL_STR(strcpy) },
	{ 0xd1ec4656, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x6e0c8b24, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x4302d0eb, __VMLINUX_SYMBOL_STR(free_pages) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x78a52e7, __VMLINUX_SYMBOL_STR(path_put) },
	{ 0x8d5b16aa, __VMLINUX_SYMBOL_STR(d_path) },
	{ 0x93fca811, __VMLINUX_SYMBOL_STR(__get_free_pages) },
	{ 0x313656ea, __VMLINUX_SYMBOL_STR(path_get) },
	{ 0x1b9aca3f, __VMLINUX_SYMBOL_STR(jprobe_return) },
	{ 0x68e2f221, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0x67f7403e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x4189195, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "432F0175B26F5F8431AAE73");
