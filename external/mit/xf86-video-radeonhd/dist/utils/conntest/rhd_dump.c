/*
 * Copyright 2007, 2008  Luc Verhaegen <libv@exsuse.de>
 * Copyright 2007, 2008  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007, 2008  Egbert Eich   <eich@novell.com>
 * Copyright 2007, 2008  Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
/* #define DEBUG */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef XSERVER_LIBPCIACCESS
#include <pciaccess.h>
#else
#include <pci/pci.h>
#endif

#include <unistd.h>
#include <stdlib.h>

#define DEFAULT_START 0x7200
#define DEFAULT_END   0x7300

#include "git_version.h"

#ifndef ULONG
typedef unsigned int ULONG;
# define ULONG ULONG
#endif
#ifndef UCHAR
typedef unsigned char UCHAR;
# define UCHAR UCHAR
#endif
#ifndef USHORT
typedef unsigned short USHORT;
# define USHORT USHORT
#endif

#include "atombios.h"

typedef int Bool;
#define FALSE 0
#define TRUE 1
typedef unsigned char CARD8;
typedef unsigned short CARD16;
typedef unsigned int CARD32;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SHORT(x) (x)
#else
#define SHORT(x) (((x & 0xff) << 8) | ((x & 0xff) >> 8))
#endif

#define DEV_MEM "/dev/mem"

typedef enum _chipType {
    RHD_R500 = 1,
    RHD_RS690,
    RHD_R600,
    RHD_RV620
} chipType;

/* for RHD_R500/R600 */
chipType ChipType;

/*
 * Match pci ids against data and some callbacks
 */
struct RHDDevice {
    CARD16 vendor;
    CARD16 device;
    int bar;
    chipType type;
} rhdDevices[] = {

    { 0x1002, 0x7100, 2, RHD_R500},
    { 0x1002, 0x7101, 2, RHD_R500},
    { 0x1002, 0x7102, 2, RHD_R500},
    { 0x1002, 0x7103, 2, RHD_R500},
    { 0x1002, 0x7104, 2, RHD_R500},
    { 0x1002, 0x7105, 2, RHD_R500},
    { 0x1002, 0x7106, 2, RHD_R500},
    { 0x1002, 0x7108, 2, RHD_R500},
    { 0x1002, 0x7109, 2, RHD_R500},
    { 0x1002, 0x710A, 2, RHD_R500},
    { 0x1002, 0x710B, 2, RHD_R500},
    { 0x1002, 0x710C, 2, RHD_R500},
    { 0x1002, 0x710E, 2, RHD_R500},
    { 0x1002, 0x710F, 2, RHD_R500},
    { 0x1002, 0x7140, 2, RHD_R500},
    { 0x1002, 0x7141, 2, RHD_R500},
    { 0x1002, 0x7142, 2, RHD_R500},
    { 0x1002, 0x7143, 2, RHD_R500},
    { 0x1002, 0x7144, 2, RHD_R500},
    { 0x1002, 0x7145, 2, RHD_R500},
    { 0x1002, 0x7146, 2, RHD_R500},
    { 0x1002, 0x7147, 2, RHD_R500},
    { 0x1002, 0x7149, 2, RHD_R500},
    { 0x1002, 0x714A, 2, RHD_R500},
    { 0x1002, 0x714B, 2, RHD_R500},
    { 0x1002, 0x714C, 2, RHD_R500},
    { 0x1002, 0x714D, 2, RHD_R500},
    { 0x1002, 0x714E, 2, RHD_R500},
    { 0x1002, 0x714F, 2, RHD_R500},
    { 0x1002, 0x7151, 2, RHD_R500},
    { 0x1002, 0x7152, 2, RHD_R500},
    { 0x1002, 0x7153, 2, RHD_R500},
    { 0x1002, 0x715E, 2, RHD_R500},
    { 0x1002, 0x715F, 2, RHD_R500},
    { 0x1002, 0x7180, 2, RHD_R500},
    { 0x1002, 0x7181, 2, RHD_R500},
    { 0x1002, 0x7183, 2, RHD_R500},
    { 0x1002, 0x7186, 2, RHD_R500},
    { 0x1002, 0x7187, 2, RHD_R500},
    { 0x1002, 0x7188, 2, RHD_R500},
    { 0x1002, 0x718A, 2, RHD_R500},
    { 0x1002, 0x718B, 2, RHD_R500},
    { 0x1002, 0x718C, 2, RHD_R500},
    { 0x1002, 0x718D, 2, RHD_R500},
    { 0x1002, 0x718F, 2, RHD_R500},
    { 0x1002, 0x7193, 2, RHD_R500},
    { 0x1002, 0x7196, 2, RHD_R500},
    { 0x1002, 0x719B, 2, RHD_R500},
    { 0x1002, 0x719F, 2, RHD_R500},
    { 0x1002, 0x71C0, 2, RHD_R500},
    { 0x1002, 0x71C1, 2, RHD_R500},
    { 0x1002, 0x71C2, 2, RHD_R500},
    { 0x1002, 0x71C3, 2, RHD_R500},
    { 0x1002, 0x71C4, 2, RHD_R500},
    { 0x1002, 0x71C5, 2, RHD_R500},
    { 0x1002, 0x71C6, 2, RHD_R500},
    { 0x1002, 0x71C7, 2, RHD_R500},
    { 0x1002, 0x71CD, 2, RHD_R500},
    { 0x1002, 0x71CE, 2, RHD_R500},
    { 0x1002, 0x71D2, 2, RHD_R500},
    { 0x1002, 0x71D4, 2, RHD_R500},
    { 0x1002, 0x71D5, 2, RHD_R500},
    { 0x1002, 0x71D6, 2, RHD_R500},
    { 0x1002, 0x71DA, 2, RHD_R500},
    { 0x1002, 0x71DE, 2, RHD_R500},
    { 0x1002, 0x7200, 2, RHD_R500},
    { 0x1002, 0x7210, 2, RHD_R500},
    { 0x1002, 0x7211, 2, RHD_R500},
    { 0x1002, 0x7240, 2, RHD_R500},
    { 0x1002, 0x7243, 2, RHD_R500},
    { 0x1002, 0x7244, 2, RHD_R500},
    { 0x1002, 0x7245, 2, RHD_R500},
    { 0x1002, 0x7246, 2, RHD_R500},
    { 0x1002, 0x7247, 2, RHD_R500},
    { 0x1002, 0x7248, 2, RHD_R500},
    { 0x1002, 0x7249, 2, RHD_R500},
    { 0x1002, 0x724A, 2, RHD_R500},
    { 0x1002, 0x724B, 2, RHD_R500},
    { 0x1002, 0x724C, 2, RHD_R500},
    { 0x1002, 0x724D, 2, RHD_R500},
    { 0x1002, 0x724E, 2, RHD_R500},
    { 0x1002, 0x724F, 2, RHD_R500},
    { 0x1002, 0x7280, 2, RHD_R500},
    { 0x1002, 0x7281, 2, RHD_R500},
    { 0x1002, 0x7283, 2, RHD_R500},
    { 0x1002, 0x7284, 2, RHD_R500},
    { 0x1002, 0x7287, 2, RHD_R500},
    { 0x1002, 0x7288, 2, RHD_R500},
    { 0x1002, 0x7289, 2, RHD_R500},
    { 0x1002, 0x728B, 2, RHD_R500},
    { 0x1002, 0x728C, 2, RHD_R500},
    { 0x1002, 0x7290, 2, RHD_R500},
    { 0x1002, 0x7291, 2, RHD_R500},
    { 0x1002, 0x7293, 2, RHD_R500},
    { 0x1002, 0x7297, 2, RHD_R500},
    { 0x1002, 0x791E, 2, RHD_RS690},
    { 0x1002, 0x791F, 2, RHD_RS690},
    { 0x1002, 0x793F, 2, RHD_RS690},
    { 0x1002, 0x7941, 2, RHD_RS690},
    { 0x1002, 0x7942, 2, RHD_RS690},
    { 0x1002, 0x796C, 2, RHD_R500},
    { 0x1002, 0x796D, 2, RHD_R500},
    { 0x1002, 0x796E, 2, RHD_R500},
    { 0x1002, 0x796F, 2, RHD_R500},
    { 0x1002, 0x9400, 2, RHD_R600},
    { 0x1002, 0x9401, 2, RHD_R600},
    { 0x1002, 0x9402, 2, RHD_R600},
    { 0x1002, 0x9403, 2, RHD_R600},
    { 0x1002, 0x9405, 2, RHD_R600},
    { 0x1002, 0x940A, 2, RHD_R600},
    { 0x1002, 0x940B, 2, RHD_R600},
    { 0x1002, 0x940F, 2, RHD_R600},
    { 0x1002, 0x94C0, 2, RHD_R600},
    { 0x1002, 0x94C1, 2, RHD_R600},
    { 0x1002, 0x94C3, 2, RHD_R600},
    { 0x1002, 0x94C4, 2, RHD_R600},
    { 0x1002, 0x94C5, 2, RHD_R600},
    { 0x1002, 0x94C6, 2, RHD_R600},
    { 0x1002, 0x94C7, 2, RHD_R600},
    { 0x1002, 0x94C8, 2, RHD_R600},
    { 0x1002, 0x94C9, 2, RHD_R600},
    { 0x1002, 0x94CB, 2, RHD_R600},
    { 0x1002, 0x94CC, 2, RHD_R600},
    { 0x1002, 0x9500, 2, RHD_R600},
    { 0x1002, 0x9501, 2, RHD_R600},
    { 0x1002, 0x9505, 2, RHD_R600},
    { 0x1002, 0x9507, 2, RHD_R600},
    { 0x1002, 0x950F, 2, RHD_R600},
    { 0x1002, 0x9511, 2, RHD_R600},
    { 0x1002, 0x9580, 2, RHD_R600},
    { 0x1002, 0x9581, 2, RHD_R600},
    { 0x1002, 0x9583, 2, RHD_R600},
    { 0x1002, 0x9586, 2, RHD_R600},
    { 0x1002, 0x9587, 2, RHD_R600},
    { 0x1002, 0x9588, 2, RHD_R600},
    { 0x1002, 0x9589, 2, RHD_R600},
    { 0x1002, 0x958A, 2, RHD_R600},
    { 0x1002, 0x958B, 2, RHD_R600},
    { 0x1002, 0x958C, 2, RHD_R600},
    { 0x1002, 0x958D, 2, RHD_R600},
    { 0x1002, 0x958E, 2, RHD_R600},
    { 0x1002, 0x958F, 2, RHD_R600},
    { 0x1002, 0x9590, 2, RHD_RV620},
    { 0x1002, 0x9591, 2, RHD_RV620},
    { 0x1002, 0x9593, 2, RHD_RV620},
    { 0x1002, 0x9594, 2, RHD_RV620},
    { 0x1002, 0x9596, 2, RHD_RV620},
    { 0x1002, 0x9597, 2, RHD_RV620},
    { 0x1002, 0x9598, 2, RHD_RV620},
    { 0x1002, 0x9599, 2, RHD_RV620},
    { 0x1002, 0x959B, 2, RHD_RV620},
    { 0x1002, 0x95C0, 2, RHD_RV620},
    { 0x1002, 0x95C2, 2, RHD_RV620},
    { 0x1002, 0x95C4, 2, RHD_RV620},
    { 0x1002, 0x95C5, 2, RHD_RV620},
    { 0x1002, 0x95C7, 2, RHD_RV620},
    { 0x1002, 0x95CC, 2, RHD_RV620},
    { 0x1002, 0x95CD, 2, RHD_RV620},
    { 0x1002, 0x95CE, 2, RHD_RV620},
    { 0x1002, 0x95CF, 2, RHD_RV620},
    { 0x1002, 0x9610, 2, RHD_RV620},
    { 0x1002, 0x9611, 2, RHD_RV620},
    { 0x1002, 0x9612, 2, RHD_RV620},
    { 0x1002, 0x9613, 2, RHD_RV620},
    { 0x1002, 0x9614, 2, RHD_RV620},
    { 0x1002, 0x9440, 2, RHD_RV620},
    { 0x1002, 0x9441, 2, RHD_RV620},
    { 0x1002, 0x9442, 2, RHD_RV620},
    { 0x1002, 0x9444, 2, RHD_RV620},
    { 0x1002, 0x9446, 2, RHD_RV620},
    { 0x1002, 0x944E, 2, RHD_RV620},
    { 0x1002, 0x9456, 2, RHD_RV620},
    { 0x1002, 0x9590, 2, RHD_RV620},
    { 0x1002, 0x954F, 2, RHD_RV620},
    { 0, 0, 0, 0 }
};

/*
 *
 */
#ifndef XSERVER_LIBPCIACCESS
/* Only for libpci use */
static struct pci_dev *
DeviceLocate(struct pci_dev *devices, int bus, int dev, int func)
{
    struct pci_dev *device;

    for (device = devices; device; device = device->next)
	if ((device->bus == bus) && (device->dev == dev) &&
	    (device->func == func))
	    return device;
    return NULL;
}
#endif

/*
 *
 */
static struct RHDDevice *
#ifdef XSERVER_LIBPCIACCESS
DeviceMatch(struct pci_device *device)
#else
DeviceMatch(struct pci_dev *device)
#endif
{
    int i;

    for (i = 0; rhdDevices[i].vendor; i++)
	if ((rhdDevices[i].vendor == device->vendor_id) &&
	    (rhdDevices[i].device == device->device_id))
	    return (rhdDevices + i);

    return NULL;
}

/*
 *
 */
#ifndef XSERVER_LIBPCIACCESS
/* Only used by pci */
static void *
MapBar(struct pci_dev *device, int ioBar, int devMem)
{
    void *map;

    if (!device->base_addr[ioBar] || !device->size[ioBar])
	return NULL;

    map = mmap(0, device->size[ioBar], PROT_WRITE | PROT_READ, MAP_SHARED,
	       devMem, device->base_addr[ioBar]);
    /* printf("Mapped IO at 0x%08llX (BAR %1d: 0x%08llX)\n",
       device->base_addr[io_bar], io_bar, device->size[io_bar]); */

    return map;
}
#endif

/*
 *
 */
CARD32
RegRead(void *map, int offset)
{
    CARD32 ret = *(volatile CARD32 *)((CARD8 *) map + offset);
#ifdef DEBUG
    fprintf(stderr, "0x%x = RegRead(0x%x)\n",ret,offset);
#endif
    return ret;
}

/*
 *
 */
void
RegWrite(void *map, int offset, CARD32 value)
{
#ifdef DEBUG
    fprintf(stderr, "RegWrite(0x%x, 0x%x)\n",offset,value);
#endif
    *(volatile CARD32 *)((CARD8 *) map + offset) = value;
}

/*
 *
 */
void
RegMask(void *map, int offset, CARD32 value, CARD32 mask)
{
    CARD32 tmp;

    tmp = RegRead(map, offset);
    tmp &= ~mask;
    tmp |= (value & mask);
    RegWrite(map, offset, tmp);
}

void
print_help(const char* progname, const char* message, const char* msgarg)
{
	if (message != NULL)
	    fprintf(stderr, "%s %s\n", message, msgarg);
	fprintf(stderr, "Usage: %s "
#if defined(XSERVER_LIBPCIACCESS) && defined (HAVE_PCI_DEVICE_ENABLE)
			"[-e] "
#endif
			"[-r start,end | -w addr val | -l {0|1}] PCI-tag\n"
#if defined(XSERVER_LIBPCIACCESS) && defined (HAVE_PCI_DEVICE_ENABLE)
		        "       -e: enable PCI card (not normally needed)\n"
#endif
			"       PCI-tag: bus:dev.func\n\n",
		progname);
}


/*
 *
 */
int
main(int argc, char *argv[])
{
#ifdef XSERVER_LIBPCIACCESS
    struct pci_device *device = NULL;
# if HAVE_PCI_DEVICE_ENABLE
    int enable_device = FALSE;
# endif
#else
    struct pci_dev *device = NULL;
    struct pci_access *pciAccess;
    int devMem;
    int saved_errno;
#endif
    struct RHDDevice *rhdDevice = NULL;
    void *io;
    int bus, dev, func;
    int ret;
    Bool deviceSet = FALSE;
    CARD32 start = DEFAULT_START, end = DEFAULT_END;
    CARD32 addr, val;
    enum {
	NONE,
	READ,
	WRITE,
	LUT
    } action = READ; /* default */

    int i;
    unsigned int j;


    printf("%s: v%s, %s\n",
	   "rhd_dump", PACKAGE_VERSION, GIT_MESSAGE);


#ifdef XSERVER_LIBPCIACCESS
    /* Initialise pciaccess */
    if ((i = pci_system_init())) {
	fprintf(stderr, "ERROR: pciaccess failed to initialise PCI bus"
		        " (error %d)\n", i);
	return 1;
    }
#else
    /* init libpci */
    pciAccess = pci_alloc();
    pci_init(pciAccess);
    pci_scan_bus(pciAccess);
#endif

    if (argc < 2) {
	print_help(argv[0], "Missing argument: please provide a PCI tag\n",
		   "");
	return 1;
    }

    for (i = 1; i < argc; i++) {
#ifdef XSERVER_LIBPCIACCESS
# if HAVE_PCI_DEVICE_ENABLE
	if (!strncmp("-e", argv[i], 3)) {
	    enable_device = TRUE;
	}else
# endif
#endif
	if (!strncmp("-r",argv[i],3)) {
	    action = READ;

	    if (++i < argc)
		ret = sscanf(argv[i], "%x,%x", &start, &end);
	    else {
		i--;
		ret = 0;
	    }
	    if (ret != 2) {
		print_help(argv[0], "Unable to read range: ", argv[i]);
		return 1;
	    }
	    if (start & 0x3) {
		fprintf(stderr, "%s: range start 0x%4.4X not four byte aligned\n",argv[0], start);
		return 1;
	    }
	} else 	if (!strncmp("-w",argv[i],3)) {
	    action = WRITE;

	    if (++i < argc)
		ret = sscanf(argv[i++], "%x", &addr);
	    else {
		i--;
		ret = 0;
	    }
	    if (ret != 1) {
		print_help(argv[0], "Unable to read addr: ", argv[i]);
		return 1;
	    }
	    if (addr & 0x3) {
		fprintf(stderr, "%s: addr 0x%4.4X not four byte aligned\n",argv[0], addr);
		return 1;
	    }
	    if (i < argc)
		ret = sscanf(argv[i], "%x", &val);
	    else {
		i--;
		ret = 0;
	    }
	} else if (!strncmp("-l", argv[i], 3)) {
	    action = LUT;

	    if (++i < argc)
		ret = sscanf(argv[i], "%d", &addr);
	    else {
		i--;
		ret = 0;
	    }

	    if (addr > 1) ret = 0;
	    if (ret != 1) {
		print_help(argv[0], "Invalid LUT id:", argv[i]);
		return 1;
	    }
	} else if (!strncmp("-",argv[i],1)) {
	    print_help(argv[0], "Unknown option", argv[i]);
	    return 1;
	} else {
	    ret = sscanf(argv[i], "%x:%x.%x", &bus, &dev, &func);
	    if (ret != 3) {
		ret = sscanf(argv[i], "%x:%x:%x", &bus, &dev, &func);
		if (ret != 3) {
		    ret = sscanf(argv[i], "%d:%d.%d", &bus, &dev, &func);
		    if (ret != 3)
			ret = sscanf(argv[i], "%d:%d:%d", &bus, &dev, &func);
		}
	    }
	    if (ret != 3) {
	        print_help(argv[0], "Unable to parse the PCI tag argument: ",
			   argv[i]);
	        return 1;
	    }
	    deviceSet = TRUE;
	}
    }

    if (deviceSet) {
#ifdef XSERVER_LIBPCIACCESS
	/* Find the toy using pciaccess */
	if ((device = pci_device_find_by_slot(0, bus, dev, func)) == NULL) {
	    fprintf(stderr, "ERROR: Unable to find PCI device at %02X:%02X.%02X.\n",
		    bus, dev, func);
	    return 1;
	}
# if HAVE_PCI_DEVICE_ENABLE
	if (enable_device)
	    pci_device_enable(device);
# endif
#else
	/* find our toy using pci */
	device = DeviceLocate(pciAccess->devices, bus, dev, func);
	if (!device) {
	    fprintf(stderr, "Unable to find PCI device at %02X:%02X.%02X.\n",
		    bus, dev, func);
	    return 1;
	}
#endif

	rhdDevice = DeviceMatch(device);
	if (!rhdDevice) {
	    fprintf(stderr,
		    "Unknown device: 0x%04X:0x%04X (%02X:%02X.%02X).\n",
		    device->vendor_id, device->device_id, bus, dev, func);
	    return 1;
	}
    } else {
	fprintf(stderr, "No PCI tag provided. "
		"Please provide a pci tag of the form xx:xx.x.\n");
	return 1;
    }

    /* Map into CPU memory space the required PCI memory */

#ifdef XSERVER_LIBPCIACCESS
    pci_device_probe(device);

    if (device->regions[rhdDevice->bar].base_addr == 0) {
	fprintf(stderr, "ERROR: Failed to find required resource on PCI card.\n");
	return 1;
    }

    if ((i = pci_device_map_range(device,device->regions[rhdDevice->bar].base_addr,
					 device->regions[rhdDevice->bar].size,
					 PCI_DEV_MAP_FLAG_WRITABLE, &io))) {
	fprintf(stderr, "ERROR: Couldn't map IO memory: %s.\n", strerror(i));
	return i;
    }

#else

    /* make sure we can actually read DEV_MEM before we do anything else */
    devMem = open(DEV_MEM, O_RDWR);
    if (devMem < 0) {
	fprintf(stderr, "Unable to open "DEV_MEM": %s.\n", strerror(errno));
	return errno;
    }

    io = MapBar(device, rhdDevice->bar, devMem);
    saved_errno = errno;
    close (devMem);
    if (io == (void *) -1) {
	fprintf(stderr, "Unable to map IO memory: %s.\n",
		strerror(saved_errno));
	return 1;
    }
#endif

    ChipType = rhdDevice->type;
    if (action == READ) {
	for (j = start; j <= end; j+=4) {
	    CARD32 val = RegRead(io, j);
	    printf("0x%4.4X: 0x%8.8X\n",j, val);
	}
    } else if (action == WRITE) {
	printf("Writing value: 0x%4.4X: 0x%8.8X\n",addr, val);
	RegWrite(io, addr, val);
	val = RegRead(io, addr);
	printf("New value: 0x%4.4X: 0x%8.8X\n",addr, val);
    }
    else if (action == LUT) {
	RegWrite(io, 0x6480, addr); /* lower half of LUT */
	RegWrite(io, 0x6484, 0);    /* table mode */
	RegWrite(io, 0x6488, 0);    /* start at 0 */

	CARD32 r, g, b;

	printf("Printing LUT %d\n", addr);
	printf("     R   G   B\n");
	for (j = 0; j < 256; j++) {
	    r = RegRead(io, 0x648C) >> 6;
	    g = RegRead(io, 0x648C) >> 6;
	    b = RegRead(io, 0x648C) >> 6;
	    printf("%02X: %3X %3X %3X\n", j, r, g, b);
	}
    }

#ifdef XSERVER_LIBPCIACCESS
    pci_device_unmap_range(device, io, device->regions[rhdDevice->bar].size);
    pci_system_cleanup();
#endif

    return 0;
}
