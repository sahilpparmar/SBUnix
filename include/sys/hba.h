#include <defs.h>

#define SATA_SIG_ATA 0x00000101  // SATA drive
#define SATA_SIG_ATAPI  0xEB140101  // SATAPI drive
#define SATA_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101  // Port multiplier

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1  // SATA drive
#define AHCI_DEV_SEMB  2  // Enclosure management bridge
#define AHCI_DEV_PM 3  // Port multiplier
#define AHCI_DEV_SATAPI  4  // SATAPI drive

enum HBA_PxCMD
{
    HBA_PxCMD_CR = 0x8000,
    HBA_PxCMD_FR = 0x4000,
    HBA_PxCMD_FRE = 0x10,
    HBA_PxCMD_ST = 0x1,
};


typedef volatile struct tagHBA_PORT
{
    uint32_t   clb;        // 0x00, command list base address, 1K-byte aligned
    uint32_t   clbu;       // 0x04, command list base address upper 32 bits
    uint32_t   fb;     // 0x08, FIS base address, 256-byte aligned
    uint32_t   fbu;        // 0x0C, FIS base address upper 32 bits
    uint32_t   is;     // 0x10, interrupt status
    uint32_t   ie;     // 0x14, interrupt enable
    uint32_t   cmd;        // 0x18, command and status
    uint32_t   rsv0;       // 0x1C, Reserved
    uint32_t   tfd;        // 0x20, task file data
    uint32_t   sig;        // 0x24, signature
    uint32_t   ssts;       // 0x28, SATA status (SCR0:SStatus)
    uint32_t   sctl;       // 0x2C, SATA control (SCR2:SControl)
    uint32_t   serr;       // 0x30, SATA error (SCR1:SError)
    uint32_t   sact;       // 0x34, SATA active (SCR3:SActive)
    uint32_t   ci;     // 0x38, command issue
    uint32_t   sntf;       // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t   fbs;        // 0x40, FIS-based switch control
    uint32_t   rsv1[11];   // 0x44 ~ 0x6F, Reserved
    uint32_t   vendor[4];  // 0x70 ~ 0x7F, vendor specific
} HBA_PORT;

typedef volatile struct tagHBA_MEM
{
    // 0x00 - 0x2B, Generic Host Control
     uint32_t   cap;        // 0x00, Host capability
     uint32_t   ghc;        // 0x04, Global host control
     uint32_t   is;     // 0x08, Interrupt status
     uint32_t   pi;     // 0x0C, Port implemented
     uint32_t   vs;     // 0x10, Version
     uint32_t   ccc_ctl;    // 0x14, Command completion coalescing control
     uint32_t   ccc_pts;    // 0x18, Command completion coalescing ports
     uint32_t   em_loc;     // 0x1C, Enclosure management location
     uint32_t   em_ctl;     // 0x20, Enclosure management control
     uint32_t   cap2;       // 0x24, Host capabilities extended
     uint32_t   bohc;       // 0x28, BIOS/OS handoff control and status

     // 0x2C - 0x9F, Reserved
     BYTE    rsv[0xA0-0x2C];

     // 0xA0 - 0xFF, Vendor specific registers
     BYTE    vendor[0x100-0xA0];

     // 0x100 - 0x10FF, Port control registers
     HBA_PORT    ports[1];   // 1 ~ 32
}HBA_MEM;


int check_type(HBA_PORT *port);
void probe_port(HBA_MEM *abar);
void init_ahci();

typedef enum
{
    FIS_TYPE_REG_H2D   = 0x27, // Register FIS - host to device
    FIS_TYPE_REG_D2H    = 0x34, // Register FIS - device to host
    FIS_TYPE_DMA_ACT    = 0x39, // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP  = 0x41, // DMA setup FIS - bidirectional
    FIS_TYPE_DATA       = 0x46, // Data FIS - bidirectional
    FIS_TYPE_BIST       = 0x58, // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP  = 0x5F, // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS   = 0xA1, // Set device bits FIS - device to host
} FIS_TYPE;


typedef struct tagFIS_REG_H2D
{
    //uint32_t 0
    BYTE    fis_type;   // FIS_TYPE_REG_H2D

    BYTE    pmport:4;   // Port multiplier
    BYTE    rsv0:3;     // Reserved
    BYTE    c:1;        // 1: Command, 0: Control

    BYTE    command;    // Command register
    BYTE    featurel;   // Feature register, 7:0

    // uint32_t 1
    BYTE    lba0;       // LBA low register, 7:0
    BYTE    lba1;       // LBA mid register, 15:8
    BYTE    lba2;       // LBA high register, 23:16
    BYTE    device;     // Device register

    // uint32_t 2
    BYTE    lba3;       // LBA register, 31:24
    BYTE    lba4;       // LBA register, 39:32
    BYTE    lba5;       // LBA register, 47:40
    BYTE    featureh;   // Feature register, 15:8

    // uint32_t 3
    BYTE    countl;     // Count register, 7:0
    BYTE    counth;     // Count register, 15:8
    BYTE    icc;        // Isochronous command completion
    BYTE    control;    // Control register

    // uint32_t 4
    BYTE    rsv1[4];    // Reserved
} FIS_REG_H2D;


typedef struct tagFIS_REG_D2H
{
    //uint32_t 0
    BYTE    fis_type;    // FIS_TYPE_REG_D2H

    BYTE    pmport:4;    // Port multiplier
    BYTE    rsv0:2;      // Reserved
    BYTE    i:1;         // Interrupt bit
    BYTE    rsv1:1;      // Reserved

    BYTE    status;      // Status register
    BYTE    error;       // Error register

    // uint32_t 1
    BYTE    lba0;        // LBA low register, 7:0
    BYTE    lba1;        // LBA mid register, 15:8
    BYTE    lba2;        // LBA high register, 23:16
    BYTE    device;      // Device register

    // uint32_t 2
    BYTE    lba3;        // LBA register, 31:24
    BYTE    lba4;        // LBA register, 39:32
    BYTE    lba5;        // LBA register, 47:40
    BYTE    rsv2;        // Reserved

    // uint32_t 3
    BYTE    countl;      // Count register, 7:0
    BYTE    counth;      // Count register, 15:8
    BYTE    rsv3[2];     // Reserved

    // uint32_t 4
    BYTE    rsv4[4];     // Reserved
} FIS_REG_D2H;



typedef struct tagFIS_DATA
{
    // uint32_t 0
    BYTE    fis_type;   // FIS_TYPE_DATA

    BYTE    pmport:4;   // Port multiplier
    BYTE    rsv0:4;     // Reserved

    BYTE    rsv1[2];    // Reserved

    // uint32_t 1 ~ N
    uint32_t   data[1];    // Payload
} FIS_DATA;


typedef struct tagFIS_PIO_SETUP
{
        // uint32_t 0
    BYTE    fis_type;   // FIS_TYPE_PIO_SETUP

    BYTE    pmport:4;   // Port multiplier
    BYTE    rsv0:1;     // Reserved
    BYTE    d:1;        // Data transfer direction, 1 - device to host
    BYTE    i:1;        // Interrupt bit
    BYTE    rsv1:1;

    BYTE    status;     // Status register
    BYTE    error;      // Error register

    // uint32_t 1
    BYTE    lba0;       // LBA low register, 7:0
    BYTE    lba1;       // LBA mid register, 15:8
    BYTE    lba2;       // LBA high register, 23:16
    BYTE    device;     // Device register

    // uint32_t 2
    BYTE    lba3;       // LBA register, 31:24
    BYTE    lba4;       // LBA register, 39:32
    BYTE    lba5;       // LBA register, 47:40
    BYTE    rsv2;       // Reserved

    // uint32_t 3
    BYTE    countl;     // Count register, 7:0
    BYTE    counth;     // Count register, 15:8
    BYTE    rsv3;       // Reserved
    BYTE    e_status;   // New value of status register

    // uint32_t 4
    uint16_t    tc;     // Transfer count
    BYTE    rsv4[2];    // Reserved
} FIS_PIO_SETUP;


typedef struct tagFIS_DMA_SETUP
{
    // uint32_t 0
    BYTE    fis_type;   // FIS_TYPE_DMA_SETUP

    BYTE    pmport:4;   // Port multiplier
    BYTE    rsv0:1;     // Reserved
    BYTE    d:1;        // Data transfer direction, 1 - device to host
    BYTE    i:1;        // Interrupt bit
    BYTE    a:1;            // Auto-activate. Specifies if DMA Activate FIS is needed

    BYTE    rsved[2];       // Reserved

    //uint32_t 1&2

    uint64_t   DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.

    //uint32_t 3
    uint32_t   rsvd;           //More reserved

    //uint32_t 4
    uint32_t   DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0

    //uint32_t 5
    uint32_t   TransferCount;  //Number of bytes to transfer. Bit 0 must be 0

    //uint32_t 6
    uint32_t   resvd;          //Reserved

} FIS_DMA_SETUP;


typedef struct tagHBA_FIS
{
    // 0x00
    FIS_DMA_SETUP   dsfis;      // DMA Setup FIS
    BYTE        pad0[4];

    // 0x20
    FIS_PIO_SETUP   psfis;      // PIO Setup FIS
    BYTE        pad1[12];

    // 0x40
    FIS_REG_D2H rfis;       // Register – Device to Host FIS
    BYTE        pad2[4];

    // 0x58
//    FIS_DEV_BITS    sdbfis;     // Set Device Bit FIS

    // 0x60
    BYTE        ufis[64];

    // 0xA0
    BYTE        rsv[0x100-0xA0];
} HBA_FIS;

typedef struct tagHBA_CMD_HEADER
{
    // DW0
    BYTE    cfl:5;      // Command FIS length in uint32_tS, 2 ~ 16
    BYTE    a:1;        // ATAPI
    BYTE    w:1;        // Write, 1: H2D, 0: D2H
    BYTE    p:1;        // Prefetchable

    BYTE    r:1;        // Reset
    BYTE    b:1;        // BIST
    BYTE    c:1;        // Clear busy upon R_OK
    BYTE    rsv0:1;     // Reserved
    BYTE    pmp:4;      // Port multiplier port

    uint16_t    prdtl;      // Physical region descriptor table length in entries

    // DW1
    volatile uint32_t   prdbc;      // Physical region descriptor byte count transferred

    // DW2, 3
    uint32_t   ctba;       // Command table descriptor base address
    uint32_t   ctbau;      // Command table descriptor base address upper 32 bits

    // DW4 - 7
    uint32_t   rsv1[4];    // Reserved
} HBA_CMD_HEADER;

typedef struct tagHBA_PRDT_ENTRY
{
    uint32_t   dba;        // Data base address
    uint32_t   dbau;       // Data base address upper 32 bits
    uint32_t   rsv0;       // Reserved

    // DW3
    uint32_t   dbc:22;     // Byte count, 4M max
    uint32_t   rsv1:9;     // Reserved
    uint32_t   i:1;        // Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL
{
    // 0x00
    BYTE    cfis[64];   // Command FIS

    // 0x40
    BYTE    acmd[16];   // ATAPI command, 12 or 16 bytes

    // 0x50
    BYTE    rsv[48];    // Reserved

    // 0x80
    HBA_PRDT_ENTRY  prdt_entry[1];  // Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;



