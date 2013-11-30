#include <defs.h>
#include <sys/ahci.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/paging.h>
#include <sys/types.h>
#include <sys/virt_mm.h>
#include <sys/phys_mm.h>
#include <sys/kstring.h>

void start_cmd(HBA_PORT *port);
void stop_cmd(HBA_PORT *port);
int find_cmdslot(HBA_PORT *port);

HBA_MEM *abar;
uint64_t *pages_for_ahci_start;
uint64_t *pages_for_ahci_end;
uint64_t *pages_for_ahci_start_virtual;
uint64_t *pages_for_ahci_end_virtual;


int write(HBA_PORT *port, DWORD startl, DWORD starth, DWORD count, QWORD buf)  
{
    port->is = 0xffff;              // Clear pending interrupt bits
    
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;
    
    uint64_t clb_addr = 0;
    clb_addr = (((clb_addr | port->clbu) << 32) | port->clb);

    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(AHCI_KERN_BASE + clb_addr);

    cmdheader += slot;
    
    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(DWORD);     // Command FIS size
    cmdheader->w = 1;               // Write to device
    cmdheader->c = 1;               
    
    // 8K bytes (16 sectors) per PRDT
    cmdheader->prdtl = 1;    // PRDT entries count

    uint64_t ctb_addr=0;
    ctb_addr =(((ctb_addr | cmdheader->ctbau)<<32)|cmdheader->ctba);
    
    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(AHCI_KERN_BASE + ctb_addr);
    memset((uint64_t *)(AHCI_KERN_BASE + ctb_addr), 0, sizeof(HBA_CMD_TBL));
    
    
    // 8K bytes (16 sectors) per PRDT
    // Last entry
    cmdtbl->prdt_entry[0].dbau = (DWORD)((buf >> 32) & 0xFFFFFFFF);
    cmdtbl->prdt_entry[0].dbc = 4096-1;   // 512 bytes per sector
    cmdtbl->prdt_entry[0].i = 0;


    // Setup command
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_WRITE_DMA_EXT;

    cmdfis->lba0 = (BYTE)startl;
    cmdfis->lba1 = (BYTE)(startl>>8);
    cmdfis->lba2 = (BYTE)(startl>>16);
    cmdfis->device = 1<<6;  // LBA mode

    cmdfis->lba3 = (BYTE)(startl>>24);
    cmdfis->lba4 = (BYTE)starth;
    cmdfis->lba5 = (BYTE)(starth>>8);

    cmdfis->countl = 1;
    cmdfis->counth = 0;

    kprintf("[slot]{%d}", slot);
    port->ci = 1 << slot;    // Issue command
    
    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0) 
            {
                break;
            }
        if (port->is & HBA_PxIS_TFES)   // Task file error
        {
            kprintf("Write disk error\n");
            return 0;
        }
    }
    
    // Check again
    if (port->is & HBA_PxIS_TFES)
    {
        kprintf("Write disk error\n");
        return 0;
    }
    return 1;
}


int read(HBA_PORT *port, DWORD startl, DWORD starth, DWORD count, QWORD buf)  
{
    
    port->is = 0xffff;              // Clear pending interrupt bits
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;
    
    uint64_t clb_addr = 0;
    clb_addr = (((clb_addr | port->clbu) << 32) | port->clb);
    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(AHCI_KERN_BASE + clb_addr);

    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(DWORD);     // Command FIS size
    cmdheader->w = 0;               // Read from device
    cmdheader->c = 1;           
    
    cmdheader->prdtl = 1;    // PRDT entries count

    uint64_t ctb_addr=0;
    ctb_addr=(((ctb_addr | cmdheader->ctbau)<<32)|cmdheader->ctba);
    
    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(AHCI_KERN_BASE + ctb_addr);
    memset((uint64_t *)(AHCI_KERN_BASE + ctb_addr), 0, sizeof(HBA_CMD_TBL));
    
    
    // 8K bytes (16 sectors) per PRDT
    // Last entry
    cmdtbl->prdt_entry[0].dba = (DWORD)(buf & 0xFFFFFFFF);
    cmdtbl->prdt_entry[0].dbau = (DWORD)((buf >> 32) & 0xFFFFFFFF);
    cmdtbl->prdt_entry[0].dbc = 4096-1;   // 512 bytes per sector
    cmdtbl->prdt_entry[0].i = 0;

    // Setup command
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_READ_DMA_EXT;

    cmdfis->lba0 = (BYTE)startl;
    cmdfis->lba1 = (BYTE)(startl>>8);
    cmdfis->lba2 = (BYTE)(startl>>16);
    cmdfis->device = 1<<6;  // LBA mode

    cmdfis->lba3 = (BYTE)(startl>>24);
    cmdfis->lba4 = (BYTE)starth;
    cmdfis->lba5 = (BYTE)(starth>>8);

    cmdfis->countl = 1;
    cmdfis->counth = 0;

    kprintf("[slot]{%d}", slot);
    port->ci = 1 << slot;    // Issue command
    
    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0) 
            {
                break;
            }
        if (port->is & HBA_PxIS_TFES)   // Task file error
        {
            kprintf("Read disk error\n");
            return 0;
        }
    }
    
    // Check again
    if (port->is & HBA_PxIS_TFES)
    {
        kprintf("Read disk error\n");
        return 0;
    }
    return 1;
}


// To setup command fing a free command list slot
int find_cmdslot(HBA_PORT *port)
{
    // An empty command slot has its respective bit cleared to ï¿½0ï¿½ in both the PxCI and PxSACT registers.
    // // If not set in SACT and CI, the slot is free // Checked
    DWORD slots = (port->sact | port->ci);
    int num_of_slots= (abar->cap & 0x0f00)>>8 ; // Bit 8-12
    int i;
    for (i=0; i<num_of_slots; i++)
    {
        if ((slots&1) == 0)
            return i;
        slots >>= 1;
    }
    kprintf("Cannot find free command list entry\n");
    return -1;
}




// Check device type
static int check_type(HBA_PORT *port)
{
    DWORD ssts = port->ssts;

    BYTE ipm = (ssts >> 8) & 0x0F;
    BYTE det = ssts & 0x0F;

//    kprintf ("\n ipm %d det %d sig %d", ipm, det, port->sig); 
    if (det != HBA_PORT_DET_PRESENT)    // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig)
    {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI ;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
    }

    return 0;
}
// Start command engine
void start_cmd(HBA_PORT *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST; 
}

void stop_cmd(HBA_PORT *port)
{
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
    
    // Wait until FR (bit14), CR (bit15) are cleared
    while(1)
    {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }

}

void port_rebase(HBA_PORT *port, int portno){

    int i;
    uint64_t clb_addr, fbu_addr, ctb_addr;
    stop_cmd(port);

    port->clb  = (((uint64_t)pages_for_ahci_start & 0xffffffff));
    port->clbu = 0;
    port->fb   = (((uint64_t)pages_for_ahci_start + (uint64_t) ((32<<10)/8))& 0xffffffff);
    port->fbu  = ((((uint64_t)pages_for_ahci_start + (uint64_t) ((32<<10)/8))>>32)& 0xffffffff);

    port->serr = 1;   //For each implemented port, clear the PxSERR register, by writing 1 to each mplemented location
    port->is   = 0;
    port->ie   = 1;
    
    clb_addr = 0;
    clb_addr = ((( clb_addr | port->clbu ) << 32 ) | port->clb );
    clb_addr =  clb_addr + AHCI_KERN_BASE;
    memset((void *)clb_addr, 0, 1024);
    
    fbu_addr = 0;
    fbu_addr = ((( fbu_addr | port->fbu ) << 32 ) | port->fb );
    fbu_addr = fbu_addr + AHCI_KERN_BASE;
    memset((void*)fbu_addr, 0, 256);
    
    clb_addr = 0;
    clb_addr = ((( clb_addr | port->clbu ) << 32 ) | port->clb );
    clb_addr = ( clb_addr + AHCI_KERN_BASE);
    
    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *) clb_addr;
    for (i = 0; i < 32; i++) {
        cmdheader[i].prdtl = 8; // 8 prdt entries per command table
        // 256 bytes per command table, 64+16+48+16*8
        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmdheader[i].ctba  = (((uint64_t)pages_for_ahci_start + (uint64_t) (( 40 << 10 )/8) + (uint64_t)(( i << 8 ) / 8)) & 0xffffffff);
        cmdheader[i].ctbau = ((((uint64_t)pages_for_ahci_start + (uint64_t) (( 40 << 10)/8) + (uint64_t)(( i << 8 ) / 8)) >> 32)& 0xffffffff);

        ctb_addr = 0;
        ctb_addr = ((( ctb_addr | cmdheader[i].ctbau ) << 32 ) | cmdheader[i].ctba );
        ctb_addr =  ctb_addr + AHCI_KERN_BASE;

        memset((void*)ctb_addr, 0, 256);
    }
    
    start_cmd(port);

    port->is = 0;   
    port->ie = 0xffffffff;

}

void probe_port(HBA_MEM *abar_temp)
{
    // Search disk in impelemented ports
    DWORD pi = abar_temp->pi;
    int i = 0;

    while (i<32)
    {
        if (pi & 1)
        {
            int dt = check_type((HBA_PORT *)&abar_temp->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                kprintf("\nSATA drive found at port %d\n", i);
                abar = abar_temp;

                port_rebase(abar_temp->ports, i);
//                kprintf("Address : %p ",(uint64_t)((char*)fs_buf) - AHCI_KERN_BASE);
//                read(&abar_temp->ports[0], 0, 0, 1, (uint64_t)(0x803000));//(uint64_t)((char*)fs_buf) - AHCI_KERN_BASE);
                
                char b[512] = "aaaa";//(char *)0xffffffff00809000;
                //kprintf("\n B Value = %s\n", b);
                //b = "Sohiliszzzzzzzz";
                for(int i =0;i<508;i++)
                    kstrcat(b,"s");
                //memcpy((void *)b,"s",512);
                kprintf("\n B Value = %s\n", b);
                int a = write(&abar_temp->ports[0], 0, 0, 1,(uint64_t)((char*)b) - AHCI_KERN_BASE);
                kprintf(" Write = %d", a);
                //char c[512] = (char *)(0xffffffff0080F000);
                read(&abar_temp->ports[0], 0, 0, 1, (uint64_t)((char*)b) - AHCI_KERN_BASE);//(uint64_t)((char*)fs_buf) - AHCI_KERN_BASE);
                kprintf("\n B Value = %s\n", b);
                //char *buf = (char *)(AHCI_KERN_BASE + (uint64_t)(pages_for_ahci_start + 4096/8));
                //kprintf("\nafter read %d",((HBA_PORT *)&abar_temp->ports[i])->ssts);
            //kprintf("\nRead Data %s", *buf);

                return;
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                //kprintf("\nSATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB)
            {
                //kprintf("\nSEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM)
            {
                //kprintf("\nPM drive found at port %d\n", i);
            }
            else
            {
                //kprintf("\nNo drive found at port %d\n", i);
            }
        }

        pi >>= 1;
        i ++;
    }
}

uint64_t* ahci_alloc_pages(uint32_t no_of_vpages)
{
    uint64_t *ret_addr = NULL, topVirtAddr = NULL; 
    uint64_t* physaddr = (uint64_t *)0x800000; 
    int frame;
    int i = 0;
    
    if (no_of_vpages > phys_get_free_block_count()) {
        return NULL;
    } else {
        ret_addr = (uint64_t *) (AHCI_KERN_BASE + (uint64_t)physaddr); 
    }
    
    topVirtAddr = (uint64_t)ret_addr;
    uint64_t paddr = (uint64_t)physaddr;
    for (i = 0; i < no_of_vpages; ++i) {
        map_virt_phys_addr(topVirtAddr, paddr, RW_USER_FLAGS);
        frame = paddr >> PAGE_2ALIGN;
        mmap_set(frame);
        
        paddr += PAGESIZE;
        topVirtAddr += PAGESIZE;     
    }

    return ret_addr;
}

void init_ahci()
{
    uint64_t paddr = 0xFEBF0000, virtAddr;
    
    pages_for_ahci_start = (uint64_t *)0x800000;
    pages_for_ahci_start_virtual = ahci_alloc_pages(32);
    
    virtAddr = get_top_virtaddr(); //get_top_virtaddr(virtAddr + PAGESIZE);
    set_top_virtaddr(virtAddr + PAGESIZE);  
    virtAddr = (uint64_t)(0xffffffff00000000 + paddr); 
    map_virt_phys_addr(virtAddr, paddr, 7); 
    

    abar = (HBA_MEM *)virtAddr;
    kprintf("\n Capablity : %p  PI : %p VI : %p Interrupt Status : %p", abar->cap, abar->pi,abar->vs,abar->is);
    probe_port(abar);
}
