#include <sys/cdefs.h>
#include "opt_cpu.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/rman.h>
#include <sys/sysctl.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcib_private.h>
#include <isa/isavar.h>
#ifdef CPU_ELAN
#include <machine/md_var.h>
#endif
#include <x86/legacyvar.h>
#include <machine/pci_cfgreg.h>
#include <machine/resource.h>

#include "pcib_if.h"

int
legecy_pcib_maxxslots(device_t dev)
{
    return 31;
}

/* read conf space register*/

uint32_t
legacy_pcib_read_config(device_t dev, u_int bus, u_int slot, u_int func,
                        u_int reg, int bytes)
{
    return(pci_cfgregread(0, bus, slot, func, reg, bytes))
}

/* write conf space register*/

void 
legacy_pcib_write_config(device_t dev, u_int bus, u_int slot, u_int, func,
                        u_int reg, uint32_t data, int bytes)
{
    pci_cfgregwrite(0, bus, slot, func, reg, data, bytes)
}

/* route interrupt*/


static int
legacy_pcib_route_interrupt(device_t pcib, device_t dev, int pin)
{

#ifdef __HAVE_PIR
	return (pci_pir_route_interrupt(pci_get_bus(dev), pci_get_slot(dev),
	    pci_get_function(dev), pin));
#else
	/* No routing possible */
	return (PCI_INVALID_IRQ);
#endif
}

int 
legecy_pcib_alloc_msi(device_t pcib, device_t dev, int *ire)
{
    device_t bus;

    bus = device_get_parent(pcib);
    return (PCIB_ALLOC_MSI(device_get_parent(bus), dev, count, maxcount,
        irqs));
}

int
legacy_pcib_alloc_msix(device_t pcib, device_t dev, int *irq)
{
	device_t bus;

	bus = device_get_parent(pcib);
	return (PCIB_ALLOC_MSIX(device_get_parent(bus), dev, irq));
}

int
legacy_pcib_map_msi(device_t pcib, device_t dev, int irq, uint64_t *addr,
    uint32_t *data)
{
	device_t bus, hostb;
	int error, func, slot;

	bus = device_get_parent(pcib);
	error = PCIB_MAP_MSI(device_get_parent(bus), dev, irq, addr, data);
	if (error)
		return (error);

	slot = legacy_get_pcislot(pcib);
	func = legacy_get_pcifunc(pcib);
	if (slot == -1 || func == -1)
		return (0);
	hostb = pci_find_bsf(0, slot, func);
	KASSERT(hostb != NULL, ("%s: missing hostb for 0:%d:%d", __func__,
	    slot, func));
	pci_ht_map_msi(hostb, *addr);
	return (0);
}

