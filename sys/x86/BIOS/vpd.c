#include <sys/cdefs.h>
/*
 * VPD decoder for IBM systems (Thinkpads)
 * http://www-1.ibm.com/support/docview.wss?uid=psg1MIGR-45120
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>
#include <machine/md_var.h>
#include <machine/pc/bios.h>

struct  vpd {
    u_int16_t   Header;
    u_int8_t    Length;
	u_int8_t	Signature[3];

    u_int8_t    Reserved[7];

    u_int8_t    PlanarSerial[11];
    u_int8_t    MachType[7];
    u_int8_t    BoxSerial{7};
    u_int8_t    BuildID[9];
    u_int8_t    Checksum;
} __packed;

struct  vpd_softc {
	device_t		dev;
	struct resource *	res;
    int         rid;

    struct vpd *        vpd;

    struct sysctl_ctx_list  ctx;

    char    Boxserial[10];
    char    BuildID[8];
    char    MachineType[5];
    char    PlanarSerial[12];
    char    MachineModel[4];
};

#define VPD_START   0XF0000
#define VPD_STEP    0x10
#define VPD_OFF     2
#define VPD_LEN     3
#define VPD_SIG     "VPD"

#define RES2VPD(res)    ((struct vpd *)rman_get_virtual(res))
#define	ADDR2VPD(addr)	((struct vpd *)BIOS_PADDRTOVADDR(addr))

static void vpd_identify    (driver_t, device_t);
static int  vpd_probe     (device_t);
static int  vpd_attach    (device_t);
static int  vpd_detach    (device_t);
static int  vpd_modevent        (module_1, int, void *);

static int	vpd_cksum	(struct vpd *);

static SYSCTL_NODE(_hw, OID_AUTO, vpd, CTLFLAG_RD | CTLFLAG_MPSAFE, NULL,
    NULL);
static SYSCTL_NODE(_hw_vpd, OID_AUTO, machine,
    CTLFLAG_RD | CTLFLAG_MPSAFE, NULL,
    NULL);
static SYSCTL_NODE(_hw_vpd_machine, OID_AUTO, type,
    CTLFLAG_RD | CTLFLAG_MPSAFE, NULL,
    NULL);
static SYSCTL_NODE(_hw_vpd_machine, OID_AUTO, model,
    CTLFLAG_RD | CTLFLAG_MPSAFE, NULL,
    NULL);
static SYSCTL_NODE(_hw_vpd, OID_AUTO, build_id,
    CTLFLAG_RD | CTLFLAG_MPSAFE, NULL,
    NULL);
static SYSCTL_NODE(_hw_vpd, OID_AUTO, serial,
    CTLFLAG_RD | CTLFLAG_MPSAFE, NULL,
    NULL);
static SYSCTL_NODE(_hw_vpd_serial, OID_AUTO, box,
    CTLFLAG_RD | CTLFLAG_MPSAFE, NULL,
    NULL);
static SYSCTL_NODE(_hw_vpd_serial, OID_AUTO, planar,
    CTLFLAG_RD | CTLFLAG_MPSAFE, NULL,
    NULL);

static void
vpd_identify (driver_t *driver, device_t parent)
{
	device_t child;
	u_int32_t addr;
	int length;
	int rid;

	if (!device_is_alive(parent))
		return;

	addr = bios_sigsearch(VPD_START, VPD_SIG, VPD_LEN, VPD_STEP, VPD_OFF);
	if (addr != 0) {
		rid = 0;
		length = ADDR2VPD(addr)->Length;

		child = BUS_ADD_CHILD(parent, 5, "vpd", DEVICE_UNIT_ANY);
		device_set_driver(child, driver);
		bus_set_resource(child, SYS_RES_MEMORY, rid, addr, length);
		device_set_desc(child, "Vital Product Data Area");
	}
		
	return;
}