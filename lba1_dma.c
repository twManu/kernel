/* **************** LDD:2.0 s_23/lab1_dma.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2012
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
/*
* DMA Memory Allocation (new API solution)
*
* Write a module that allocates and maps a suitable DMA buffer, and
* obtains the bus address handle.
*
* Do this in three ways:
*
*    Using dma_alloc_coherent().
*
*    Using dma_map_single()
*
*    Using a DMA Pool.
*
* You should use NULL for the device and/or pci_dev structure
* arguments for the first and the third methods, since we don't
* actually have a physical device; however for the second method you
* have to set up and register a device.
*
* Compare the resulting kernel and bus addresses; how do they differ?
* Compare with the value of PAGE_OFFSET.
*
* In each case copy a string into the buffer and make sure it can be
* read back properly.
*
* In the case of dma_map_single(), you may want to compare the use of
* different direction arguments.
*
* We give two solutions, one with the new bus-independent interface,
* and one with the older PCI API.
*
@*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/device.h>

// int direction = PCI_DMA_TODEVICE ;
// int direction = PCI_DMA_FROMDEVICE ;
static int direction = PCI_DMA_BIDIRECTIONAL;
//int direction = PCI_DMA_NONE;

static char *kbuf;
static dma_addr_t handle;
static size_t size = (10 * PAGE_SIZE);
static struct dma_pool *mypool;
static size_t pool_size = 1024;
static size_t pool_align = 8;

static void my_release(struct device *dev)
{
	pr_info("releasing DMA device\n");
}

static struct device dev = {
	.release = my_release
};

static void output(char *kbuf, dma_addr_t handle, size_t size, char *string)
{
	unsigned long long diff;
	diff = (unsigned long long)kbuf - handle;
	pr_info("kbuf=0x%p, handle=0x%llx, size = %d\n", kbuf,
		(unsigned long long)handle, (int)size);
	pr_info("(kbuf-handle)= 0x%llx, PAGE_OFFSET=0x%lx, compare=0x%llx\n",
		diff, PAGE_OFFSET, diff - PAGE_OFFSET);
	strcpy(kbuf, string);
	pr_info("string written was, %s\n", kbuf);
}

static int __init my_init(void)
{
	dev_set_name(&dev, "my0");
	device_register(&dev);

	/* dma_alloc_coherent method */

	pr_info("Loading DMA allocation test module\n");
	pr_info("\nTesting dma_alloc_coherent()..........\n\n");
	kbuf = dma_alloc_coherent(NULL, size, &handle, GFP_KERNEL);
	if( kbuf ) {
		output(kbuf, handle, size, "This is the dma_alloc_coherent() string");
		dma_free_coherent(NULL, size, kbuf, handle);
	} else pr_info("fail w/ dma_alloc_coherent\n");

	/* dma_map/unmap_single */

	pr_info("\nTesting dma_map_single()................\n\n");
	kbuf = kmalloc(size, GFP_KERNEL);
	if( kbuf ) {
		handle = dma_map_single(&dev, kbuf, size, direction);
		if( handle ) {
			output(kbuf, handle, size, "This is the dma_map_single() string");
			dma_unmap_single(&dev, handle, size, direction);
		} else pr_info("fail w/ dma_map_single\n");
		kfree(kbuf);
	} else pr_info("fail w/ kmalloc\n");

	/* dma_pool method */
#if	0
	pr_info("\nTesting dma_pool_alloc()..........\n\n");
	mypool = dma_pool_create("mypool", NULL, pool_size, pool_align, 0);
	if( mypool ) {
		kbuf = dma_pool_alloc(mypool, GFP_KERNEL, &handle);
		if( kbuf ) {
			output(kbuf, handle, size, "This is the dma_pool_alloc() string");
			dma_pool_free(mypool, kbuf, handle);
		} else pr_info("fail w/ dma_pool_alloc\n");
		dma_pool_destroy(mypool);
	} else pr_info("fail w/ dma_pool_create\n");
#endif
	device_unregister(&dev);

	return 0;
}

static void __exit my_exit(void)
{
	pr_info("Module Unloading\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_23/lab1_dma.c");
MODULE_LICENSE("GPL v2");

