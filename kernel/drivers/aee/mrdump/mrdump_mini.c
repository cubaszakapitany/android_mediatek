#include <linux/mm.h>
#include <linux/elf.h>
#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/mrdump.h>
#include <linux/aee.h>

extern void ipanic_header_update_minirdump(size_t size);
extern size_t ipanic_offset_mini_rdump(void);
extern int ipanic_mem_write(void *buf, int off, int len, int encrypt);

/* 
 * ramdump mini 
 * implement ramdump mini prototype here 
 */
static char mrdump_mini_buf[MRDUMP_MINI_HEADER_SIZE];
static void __mrdump_mini_core(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, va_list ap)
{
	int i;
	unsigned long reg, start, end, size;
	int errno;
	loff_t offset = 0;
	loff_t sd_start = ipanic_offset_mini_rdump();
	struct mrdump_mini_header *hdr = (struct mrdump_mini_header *)mrdump_mini_buf;
	loff_t sd_offset = sd_start + MRDUMP_MINI_HEADER_SIZE;

	memset(mrdump_mini_buf, 0x0, MRDUMP_MINI_HEADER_SIZE);

	if (sizeof(struct mrdump_mini_header) > MRDUMP_MINI_HEADER_SIZE) {
		/* mrdump_mini_header is too large, write 0x0 headers to ipanic */
		printk(KERN_ALERT"mrdump_mini_header is too large(%d)\n", 
				sizeof(struct mrdump_mini_header));
		offset += MRDUMP_MINI_HEADER_SIZE;
		goto ipanic_write;
	}

	for(i = 0; i < ELF_NGREG; i++) {
		reg = regs->uregs[i];
		hdr->reg_desc[i].reg = reg;
		if (virt_addr_valid(reg)) {
			/* 
			 * ASSUMPION: memory is always in normal zone.
			 * 1) dump at most 32KB around valid kaddr
			 */
			/* align start address to PAGE_SIZE for gdb */
			start = round_down((reg - SZ_16K), PAGE_SIZE);
			end = start + SZ_32K;
			start = clamp(start, (unsigned long)PAGE_OFFSET, (unsigned long)high_memory);
			end = clamp(end, (unsigned long)PAGE_OFFSET, (unsigned long)high_memory) - 1;
			hdr->reg_desc[i].kstart = start;
			hdr->reg_desc[i].kend = end;
			hdr->reg_desc[i].offset = offset;
			hdr->reg_desc[i].valid = 1;
			size = end - start + 1;
			errno = ipanic_mem_write((void*)start, sd_offset + offset, size, 1);
			offset += size;
			if (IS_ERR(ERR_PTR(errno)))
				break;
		} else {
			hdr->reg_desc[i].kstart = 0;
			hdr->reg_desc[i].kend = 0;
			hdr->reg_desc[i].offset = 0;
			hdr->reg_desc[i].valid = 0;
		}
	}

ipanic_write:
	errno = ipanic_mem_write(mrdump_mini_buf, sd_start, ALIGN(MRDUMP_MINI_HEADER_SIZE, SZ_512), 1);
	if (IS_ERR(ERR_PTR(errno))) {
		printk(KERN_ERR"mini_rdump failed (%d), offset: 0x%llx\n", 
		       errno, (unsigned long long)offset);
	} else {
		ipanic_header_update_minirdump(MRDUMP_MINI_BUF_SIZE);
	}
}

void __mrdump_mini_create_oops_dump(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	__mrdump_mini_core(reboot_mode, regs, msg, ap);
	va_end(ap);
}
EXPORT_SYMBOL(__mrdump_mini_create_oops_dump);

static int mrdump_mini_create_oops_dump(struct notifier_block *self, unsigned long cmd, void *ptr)
{
	struct die_args *dargs = (struct die_args *)ptr;
	smp_send_stop();
	__mrdump_mini_create_oops_dump(AEE_REBOOT_MODE_KERNEL_PANIC, dargs->regs, "kernel Oops");
	return NOTIFY_DONE;
}

static struct notifier_block mrdump_mini_oops_blk = {
	.notifier_call	= mrdump_mini_create_oops_dump,
};

static int __init mrdump_mini_init(void)
{
	register_die_notifier(&mrdump_mini_oops_blk);
	return 0;
}

late_initcall(mrdump_mini_init);

