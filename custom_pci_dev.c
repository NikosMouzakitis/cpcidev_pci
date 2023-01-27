
#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/hw.h"
#include "hw/pci/msi.h"
#include "qemu/timer.h"
#include "qom/object.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qemu/module.h"
#include "qapi/visitor.h"

#define TYPE_PCI_CUSTOM_DEVICE "cpcidev"

typedef struct CpcidevState CpcidevState;

//This macro provides the instance type cast functions for a QOM type.
DECLARE_INSTANCE_CHECKER(CpcidevState, CPCIDEV, TYPE_PCI_CUSTOM_DEVICE)

//struct defining/descring the state
//of the custom pci device.
struct CpcidevState {
    PCIDevice pdev;
    MemoryRegion mmio;
    uint32_t op1;
    uint32_t op2;
    uint32_t opcode;
    uint32_t result;
    uint32_t error_calculation; //set when invalid opcode appears.
};

static uint64_t cpcidev_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    CpcidevState *cpcidev = opaque;
    uint64_t val = ~0ULL;
    printf("CPCIDEV: cpcidev_mmio_read() addr %lx \n", addr);
    printf("CPCIDEV: cpcidev_mmio_read() size %x \n", size);

    if (addr < 0x80 && size != 4) {
        return val;
    }

    if (addr >= 0x80 && size != 4 && size != 8) {
        return val;
    }

    switch (addr) {
    case 0x00:
	printf("addr 0x0\n");
	    //explain this one.
        val = 0x01234567u;
        break;
    case 0x10:
	printf("addr 0x10 return op1\n");
	val = cpcidev->op1;
	break;
    case 0x14:
	printf("addr 0x14 return op2\n");
	val = cpcidev->op2;
	break;
    case 0x18:
	printf("addr 0x18 return opcode\n");
	val = cpcidev->opcode;	
	break;
    case 0x30: 
	printf("addr 0x30 return result\n");
	
	//opcode for sum
	if(cpcidev->opcode == 0x1) {
		cpcidev->result = cpcidev->op1 + cpcidev->op2;
		cpcidev->error_calculation = 0x0; //correct operation
	} else if(cpcidev->opcode == 0x2) { //sub
		cpcidev->result = cpcidev->op1 - cpcidev->op2;
		cpcidev->error_calculation = 0x0; //correct operation
	} else if(cpcidev->opcode == 0x3) { //mult
		cpcidev->result = cpcidev->op1 * cpcidev->op2;
		cpcidev->error_calculation = 0x0; //correct operation
	} else {
		printf("INVALID OPCODE return 0xff;\n");
		cpcidev->result = 0xff;
		cpcidev->error_calculation = 1;
	}

	val = cpcidev->result;
	
	break;
    }

    return val;
}

static void cpcidev_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                unsigned size)
{
    printf("CPCIDEV: cpcidev_mmio_write() addr %lx \n", addr);
    printf("CPCIDEV: cpcidev_mmio_write()  val %lx \n", val);
    CpcidevState *cpcidev = opaque;

    if (addr < 0x80 && size != 4) {
        return;
    }

    if (addr >= 0x80 && size != 4 && size != 8) {
        return;
    }

    switch (addr) {
    case 0x10:
	printf("setting op1 to %lx \n", val);
	cpcidev->op1 = val;
	break;
    case 0x14:
	printf("setting op2 to %lx \n", val);
	cpcidev->op2 = val;
	break;
    case 0x18:
	printf("setting opcode to %lx \n", val);
	cpcidev->opcode = val;
	break;
    }
}

///ops for the Memory Region.
static const MemoryRegionOps cpcidev_mmio_ops = {
    .read = cpcidev_mmio_read,
    .write = cpcidev_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 4,
        .max_access_size = 8,
    },

};

//implementation of the realize function.
static void pci_cpcidev_realize(PCIDevice *pdev, Error **errp)
{
    CpcidevState *cpcidev = CPCIDEV(pdev);
    uint8_t *pci_conf = pdev->config;

    pci_config_set_interrupt_pin(pci_conf, 1);
    //supporting msi vectors ( 1 msi vector here of 64bit).
    //Make PCI device @dev MSI-capable.
    /*
    * Non-zero @offset puts capability MSI at that offset in PCI config
	* space.
	* @nr_vectors is the number of MSI vectors (1, 2, 4, 8, 16 or 32).
	* If @msi64bit, make the device capable of sending a 64-bit message
	* address.
	* If @msi_per_vector_mask, make the device support per-vector masking.
	* @errp is for returning errors.
	* Return 0 on success; set @errp and return -errno on error. */
    if (msi_init(pdev, 0, 1, true, false, errp)) {
        return;
    }
	
    ///initial configuration of devices registers.
    cpcidev->op1 = 0x02;
    cpcidev->op2 = 0x04;
    cpcidev->opcode = 0xaa;
    cpcidev->result = 0xbb;
    cpcidev->error_calculation = 0x00;

    // Initialize an I/O memory region(cpcidev->mmio). 
    // Accesses to this region will cause the callbacks 
    // of the cpcidev_mmio_ops to be called.
    memory_region_init_io(&cpcidev->mmio, OBJECT(cpcidev), &cpcidev_mmio_ops, cpcidev, "cpcidev-mmio", 1 * MiB);
    // registering the pdev and all of the above configuration 
    // (actually filling a PCI-IO region with our configuration.
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &cpcidev->mmio);
}

// uninitializing functions performed.
static void pci_cpcidev_uninit(PCIDevice *pdev)
{
    return;
}


///initialization of the device
static void cpcidev_instance_init(Object *obj)
{
    return ;
}

static void cpcidev_class_init(ObjectClass *class, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(class);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(class);
	
    //definition of realize func().
    k->realize = pci_cpcidev_realize;
    //definition of uninit func().
    k->exit = pci_cpcidev_uninit;
    k->vendor_id = PCI_VENDOR_ID_QEMU;
    k->device_id = 0xabcd; //our device id, 'abcd' hexadecimal
    k->revision = 0x10;
    k->class_id = PCI_CLASS_OTHERS;

    /**
    * set_bit - Set a bit in memory
    * @nr: the bit to set
    * @addr: the address to start counting from
    */
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static void pci_custom_device_register_types(void)
{
    static InterfaceInfo interfaces[] = {
        { INTERFACE_CONVENTIONAL_PCI_DEVICE },
        { },
    };
    static const TypeInfo custom_pci_device_info = {
        .name          = TYPE_PCI_CUSTOM_DEVICE,
        .parent        = TYPE_PCI_DEVICE,
        .instance_size = sizeof(CpcidevState),
        .instance_init = cpcidev_instance_init,
        .class_init    = cpcidev_class_init,
        .interfaces = interfaces,
    };
    //registers the new type.
    type_register_static(&custom_pci_device_info);
}

type_init(pci_custom_device_register_types)
