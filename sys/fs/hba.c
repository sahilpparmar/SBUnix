#include <defs.h>
#include <sys/hba.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/paging.h>
#include <sys/types.h>
#include <sys/virt_mm.h>


#define AHCI_BASE    0x400000    // 4M

uint64_t ahci_start = 0xfebf0000;
uint64_t virtAddr;
HBA_MEM *dev;

int check_type(HBA_PORT *port)
{
      uint32_t ssts = port->ssts;

      BYTE ipm = (ssts >> 8) & 0x0F;
      BYTE det = ssts & 0x0F;
      //kprintf("Ipm = %d Det = %d",ipm,det);

      if (det != 0x03) // Check drive status
          return AHCI_DEV_NULL;
      if (ipm != 0x01)
          return AHCI_DEV_NULL;
    
    switch (port->sig)
    {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
    }
}

void probe_port(HBA_MEM *abar)
{
    //Search disk in impelemented ports
    uint32_t pi = abar->pi;
    int i = 0;
    while (i<32)
    {
        if (pi & 1)
        {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                kprintf("SATA drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                kprintf("SATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB)
            {
                kprintf("SEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM)
            {
                kprintf("PM drive found at port %d\n", i);
            }
            else
            {
                kprintf("No drive found at port %d\n", i);
            }
        }

        pi >>= 1;
        i ++;
    }
}

#define AHCI_BASE    0x400000    // 4M
 
// Start command engine
void start_cmd(HBA_PORT *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST; 
}

// Stop command engine
void stop_cmd(HBA_PORT *port)
{
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Wait until FR (bit14), CR (bit15) are cleared
    while(1)
    {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
}

void port_rebase(HBA_PORT *port, int portno)
{
    //stop_cmd(port); // Stop command engine
    
    port->cmd = port->cmd & ~HBA_PxCMD_CR;
    port->cmd = port->cmd & ~HBA_PxCMD_FR;
    port->cmd = port->cmd & ~HBA_PxCMD_FRE;
    port->cmd = port->cmd & ~HBA_PxCMD_ST;

    int no_of_slots= (dev->cap & 0x0f00) >> 8 ; // Bit 8-12
    kprintf("\n NUK SLOTS [%d]",no_of_slots);
    
    
    // Command list offset: 1K*portno
    // Command list entry size = 32
    // Command list entry maxim count = 32
    // Command list maxim size = 32*32 = 1K per port
    if( (dev->cap )>>31 )
    {
        port->clb = (((uint64_t)ahci_start & 0xffffffff));
        port->clbu = ((uint64_t)ahci_start >> 32);
        port->fb =  (((uint64_t)ahci_start + (uint64_t) ((32<<10)/8))& 0xffffffff);
        port->fbu =  ((((uint64_t)ahci_start + (uint64_t) ((32<<10)/8))>>32)& 0xffffffff);
    }
   
    port->serr = 1;

    port->is = 0;
    kprintf("\n AHCI Start [%p]", ahci_start);
    
    //port->clb = AHCI_BASE + (portno << 10);
    //port->clbu = 0;
    // memset8((void *)port->clb, 0x0, 1024);

    // FIS offset: 32K+256*portno
    // FIS entry size = 256 bytes per port
    //port->fb = AHCI_BASE + (32<<10) + (portno<<8);
    //port->fbu = 0;
    //memset8((void *)port->fb, 0x0, 256);

    
    
    // Command table offset: 40K + 8K*portno
    // Command table size = 256*32 = 8K per port


    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)virtAddr;//(((uint64_t)ahci_start & 0xffffffff));
    for (int i=0; i<32; i++)
    {
        cmdheader[i].prdtl = 8; // 8 prdt entries per command table
        // 256 bytes per command table, 64+16+48+16*8
        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmdheader[i].ctba = (uint64_t)virtAddr + ((40<<10)/8) + ((i<<8)/8);
        cmdheader[i].ctbau = 0;
        //memset((void*)cmdheader[i].ctba, 0, 256);
    }
    kprintf("Reached End of Initialization");
    //start_cmd(port);    // Start command engine
}

void init_ahci()
{
    virtAddr = get_top_virtaddr();
    set_top_virtaddr(virtAddr + PAGESIZE);
    map_virt_phys_addr(virtAddr, ahci_start, 3);
    dev = (HBA_MEM *)virtAddr;
    kprintf("\n Capablity : %p  PI : %p VI : %p Interrupt Status : %p", dev->cap, dev->pi,dev->vs,dev->is);
    probe_port(dev);
    port_rebase(dev->ports, 0);
}
