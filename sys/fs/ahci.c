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
#include <sys/fs.h>
#include <sys/dirent.h>
#include <sys/kmalloc.h>

void start_cmd(HBA_PORT *port);
void stop_cmd(HBA_PORT *port);
int find_cmdslot(HBA_PORT *port);

HBA_MEM *abar;

int write(HBA_PORT *port, DWORD startl, DWORD starth, DWORD count, QWORD buf)  
{
    port->is = 0xffff;              // Clear pending interrupt bits

    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    uint64_t clb_addr = 0;
    clb_addr = (((clb_addr | port->clbu) << 32) | port->clb);
    //kprintf("\n clb_addr = %p", clb_addr);
    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(AHCI_KERN_BASE + clb_addr);

    cmdheader += slot;

    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(DWORD);     // Command FIS size
    cmdheader->w = 1;               // Write to device
    cmdheader->c = 1;               

    // 8K bytes (16 sectors) per PRDT
    cmdheader->prdtl = 1;    // PRDT entries count

    uint64_t ctb_addr=0;
    ctb_addr =(((ctb_addr | cmdheader->ctbau)<<32)|cmdheader->ctba);
    //kprintf("\n ctb_addr = %p", ctb_addr);

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
    cmdfis->command = ATA_CMD_WRITE_DMA_EXT;

    cmdfis->lba0 = (BYTE)startl;
    cmdfis->lba1 = (BYTE)(startl>>8);
    cmdfis->lba2 = (BYTE)(startl>>16);
    cmdfis->device = 1<<6;  // LBA mode

    cmdfis->lba3 = (BYTE)(startl>>24);
    cmdfis->lba4 = (BYTE)starth;
    cmdfis->lba5 = (BYTE)(starth>>8);

    cmdfis->countl = 2;
    cmdfis->counth = 0;

    //kprintf("[slot]{%d}", slot);
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
    //kprintf("\n clb_addr = %p", clb_addr);

    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(DWORD);     // Command FIS size
    cmdheader->w = 0;               // Read from device
    cmdheader->c = 1;           

    cmdheader->prdtl = 1;    // PRDT entries count

    uint64_t ctb_addr=0;
    ctb_addr=(((ctb_addr | cmdheader->ctbau)<<32)|cmdheader->ctba);
    //kprintf("\n ctb_addr = %p", ctb_addr);

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

    //kprintf("[slot]{%d}", slot);
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
    // If not set in SACT and CI, the slot is free
    // Checked
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

    port->clb  = (((uint64_t)AHCI_PHYS_BASE & 0xffffffff));
    port->clbu = 0;
    port->fb   = (((uint64_t)AHCI_PHYS_BASE + (uint64_t) ((32<<10)))& 0xffffffff);
    port->fbu  = ((((uint64_t)AHCI_PHYS_BASE + (uint64_t) ((32<<10)))>>32)& 0xffffffff);

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
        cmdheader[i].ctba  = (((uint64_t)AHCI_PHYS_BASE + (uint64_t) (( 40 << 10 )) + (uint64_t)(( i << 8 ))) & 0xffffffff);
        cmdheader[i].ctbau = ((((uint64_t)AHCI_PHYS_BASE + (uint64_t) (( 40 << 10)) + (uint64_t)(( i << 8 ))) >> 32)& 0xffffffff);

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

void ahci_alloc_pages(uint32_t no_of_vpages)
{
    uint64_t vaddr, paddr; 
    int frame;
    int i = 0;

    vaddr = AHCI_VIRT_BASE;
    paddr = AHCI_PHYS_BASE;

    for (i = 0; i < no_of_vpages; ++i) {
        map_virt_phys_addr(vaddr, paddr, RW_USER_FLAGS);
        frame = paddr >> PAGE_2ALIGN;
        mmap_set(frame);

        paddr += PAGESIZE;
        vaddr += PAGESIZE;     
    }
}

void populate_struct(super_block *s_star)
{

    int count = 0;
    ext_inode* inode_e  = (ext_inode *)kmalloc(sizeof(ext_inode));
    char *path, *temp, *temp_tokens[10]; 
    int i, j, k; 

    for (i = 0; i < s_star->s_ninodes; i++) {
        
        if (read_inode(inode_e, i)){
            //kprintf("\t[%d]=>%s", i, inode_e->i_name);
           
            fnode_t *auxnode, *temp_node,*currnode;
            currnode = root_node;
            
            path =  kmalloc(kstrlen(inode_e->i_name));            
            kstrcpy(path, inode_e->i_name); 
            
            count = 0;
            //Tokenize the string path and store all '/' strings
            temp = kstrtok(path, "/");  
            while (temp != NULL) {
                temp_tokens[count] = (char *)temp;                
                count++;
                temp = kstrtok(NULL, "/"); 
            }
      
            //go over all tokens and create filenode structs
            for (j = 0; j < count; ++j){
                 
                auxnode = currnode;

                for(k = 2; k < currnode->end; ++k){
                    
                    if(kstrcmp(temp_tokens[j], currnode->f_child[k]->f_name) == 0) {
                        currnode = (fnode_t *)currnode->f_child[k];
                        break; 
                    }        
                }
                
                //kprintf("\nauxnode %s, currnode %s", auxnode->f_name, currnode->f_name);
                //if auxnode is not equal to currnode mean child has been
                //found so no need to create new node
                
                if (k == auxnode->end) {
                    //if code reaches here create inode with temptokens[j] and set its parent as currnode
                    temp_node = (fnode_t *)kmalloc(sizeof(struct file));
                    
                    //check if we want to create node of type FILE or Directory
                    if (j == count-1){
                        
                        //kprintf("\n creating file: %s, parent : %s, inode no %d", temp_tokens[j],currnode->f_name, i);
                        make_node(temp_node, currnode, temp_tokens[j], 0, 0, FILE, i);
                        currnode->f_child[currnode->end] = temp_node;
                        currnode->end += 1; 
                    
                    } else {
                        //reached last token. Create this node of type FILE
                        //kprintf("\n creating dir: %s, parent %s, inode no %d", temp_tokens[j], currnode->f_name, 0);
                        make_node(temp_node, currnode, temp_tokens[j], 0, 2, DIRECTORY, 0);
                        currnode->f_child[currnode->end] = temp_node;
                        currnode->end += 1; 
                    }
                    
                    currnode = temp_node;
                } 
           } 
        } 
    }

}

void init_disk(bool forceCreate)
{
    uint64_t paddr = 0xFEBF0000, vaddr;
    super_block *s_star = NULL;    

    ahci_alloc_pages(32);

    vaddr = AHCI_VIRT_BASE + paddr; 
    map_virt_phys_addr(vaddr, paddr, RW_USER_FLAGS); 

    abar = (HBA_MEM *)vaddr;
    kprintf("\n Capablity : %p  PI : %p VI : %p Interrupt Status : %p", abar->cap, abar->pi,abar->vs,abar->is);
    probe_port(abar);

    // Reads an existing super block and creates one if not present 
    s_star = read_first_superblock(forceCreate);
    
    // Populates the logical tree structure by parsing inodes
    populate_struct(s_star);
}

// Reads one sector
void read_sector(void* read_addr, uint64_t sector_no, uint64_t sec_off, uint64_t size)
{
    uint64_t vaddr   = VIRT_PAGE(1);
    uint32_t sec_low = sector_no & 0xFFFFFFFF;
    uint32_t sec_hi  = sector_no >> 32;

    // Check for Invalid offset/size
    if (sec_off + size > SIZE_OF_SECTOR)
        return;

    read(&abar->ports[0], sec_low, sec_hi, 1, PHYS_PAGE(1));

    memcpy(read_addr, (void*)vaddr + sec_off, size);
}

// Writes to one sector
void write_sector(void* write_addr, uint64_t sector_no, uint64_t sec_off, uint64_t size)
{
    uint64_t vaddr   = VIRT_PAGE(1) + sec_off;
    uint32_t sec_low = sector_no & 0xFFFFFFFF;
    uint32_t sec_hi  = sector_no >> 32;
    
    // Check for Invalid offset/size
    if (sec_off + size > SIZE_OF_SECTOR)
        return;

    if (size != SIZE_OF_SECTOR) { 
        // Trying to write only few parts in a sector.
        // So need to read whole sector in order to rewrite the remainder part.
        read(&abar->ports[0], sec_low, sec_hi, 1, PHYS_PAGE(1));
    }

    // Memcpy only if write_addr is different then vaddr
    if (vaddr != (uint64_t) write_addr) {
        memcpy((void*)vaddr, write_addr, size);
    }
    //kprintf("\tW[%d]+%d", sec_low, sec_off);

    write(&abar->ports[0], sec_low, sec_hi, 1, PHYS_PAGE(1));
}

