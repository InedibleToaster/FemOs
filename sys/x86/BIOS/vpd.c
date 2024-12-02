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
    u_int8_t    Signature[3];
    u_int8_t    Length;

    u_int8_t    Reserved[7];

    u_int8_t    BuildID[9];
    
}:
