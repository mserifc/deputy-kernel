#include "drv/usb.h"

#include "kernel.h"
#include "hw/devbus.h"

typedef struct {
    uint8_t CAPLENGTH;
    uint8_t Rsvd;
    uint16_t HCIVERSION;
    uint32_t HCSPARAMS1;
    uint32_t HCSPARAMS2;
    uint32_t HCSPARAMS3;
    uint32_t HCCPARAMS1;
    uint32_t DBOFF;
    uint32_t RTSOFF;
    uint32_t HCCPARAMS2;
    uint32_t VTIOSOFF;
} PACKED usb_Capregs_t;

#define USB_CAPREG_HCSP1_MAXSLOTS 0xFF
#define USB_CAPREG_HCSP1_MAXINTRS 0x7FF00
#define USB_CAPREG_HCSP1_MAXPORTS 0xFF000000

#define USB_CAPREG_HCSP2_IST 0xF
#define USB_CAPREG_HCSP2_ERSTMAX 0xF0
#define USB_CAPREG_HCSP2_MAXSPBHI 0x3E00000
#define USB_CAPREG_HCSP2_SPR (1 << 26)
#define USB_CAPREG_HCSP2_MAXSPBLO 0xF8000000

#define USB_CAPREG_HCSP3_U1EXITLT 0xFF
#define USB_CAPREG_HCSP3_U2EXITLT 0xFF00

#define USB_CAPREG_HCCP1_AC64 (1 << 0)
#define USB_CAPREG_HCCP1_BNC (1 << 1)
#define USB_CAPREG_HCCP1_CSZ (1 << 2)
#define USB_CAPREG_HCCP1_PPC (1 << 3)
#define USB_CAPREG_HCCP1_PIND (1 << 4)
#define USB_CAPREG_HCCP1_LHRC (1 << 5)
#define USB_CAPREG_HCCP1_LTC (1 << 6)
#define USB_CAPREG_HCCP1_NSS (1 << 7)
#define USB_CAPREG_HCCP1_PAE (1 << 8)
#define USB_CAPREG_HCCP1_SPC (1 << 9)
#define USB_CAPREG_HCCP1_SEC (1 << 10)
#define USB_CAPREG_HCCP1_CFC (1 << 11)
#define USB_CAPREG_HCCP1_MAXPSA 0xF000
#define USB_CAPREG_HCCP1_XECP 0xFFFF0000

#define USB_CAPREG_DBOFF 0xFFFFFFFC

#define USB_CAPREG_RTSOFF 0xFFFFFFE0

#define USB_CAPREG_HCCP2_U3C (1 << 0)
#define USB_CAPREG_HCCP2_CMC (1 << 1)
#define USB_CAPREG_HCCP2_FSC (1 << 2)
#define USB_CAPREG_HCCP2_CTC (1 << 3)
#define USB_CAPREG_HCCP2_LEC (1 << 4)
#define USB_CAPREG_HCCP2_CIC (1 << 5)
#define USB_CAPREG_HCCP2_ETC (1 << 6)
#define USB_CAPREG_HCCP2_TSC (1 << 7)
#define USB_CAPREG_HCCP2_GSC (1 << 8)
#define USB_CAPREG_HCCP2_VTC (1 << 9)

#define USB_CAPREG_VTIOSOFF 0xFFFFF000

typedef struct {
    uint32_t PORTSC;
    uint32_t PORTPMSC;
    uint32_t PORTLI;
    uint32_t PORTHLPMC;
} PACKED usb_Portregs_t;

typedef struct {
    uint32_t USBCMD;
    uint32_t USBSTS;
    uint32_t PAGESIZE;
    uint8_t RsvdZ1[0x14-0x0C];
    uint32_t DNCTRL;
    uint64_t CRCR;
    uint8_t RsvdZ2[0x30-0x20];
    uint64_t DCBAAP;
    uint32_t CONFIG;
    uint8_t RsvdZ3[0x400-0x3C];
    usb_Portregs_t port[(0x1400-0x400)/sizeof(usb_Portregs_t)];
} PACKED usb_Opregs_t;

#define USB_OPREG_CMD_RS (1 << 0)
#define USB_OPREG_CMD_HCRST (1 << 1)
#define USB_OPREG_CMD_INTE (1 << 2)
#define USB_OPREG_CMD_HSEE (1 << 3)
#define USB_OPREG_CMD_RSVDP1 0x70
#define USB_OPREG_CMD_LHCRST (1 << 7)
#define USB_OPREG_CMD_CSS (1 << 8)
#define USB_OPREG_CMD_CRS (1 << 9)
#define USB_OPREG_CMD_EWE (1 << 10)
#define USB_OPREG_CMD_EU3S (1 << 11)
#define USB_OPREG_CMD_RSVDP2 (1 << 12)
#define USB_OPREG_CMD_CME (1 << 13)
#define USB_OPREG_CMD_ETE (1 << 14)
#define USB_OPREG_CMD_TSCEN (1 << 15)
#define USB_OPREG_CMD_VTIOEN (1 << 16)
#define USB_OPREG_CMD_RSVDP3 0xFFFE0000

#define USB_OPREG_STS_HCH (1 << 0)
#define USB_OPREG_STS_RSVDZ1 (1 << 1)
#define USB_OPREG_STS_HSE (1 << 2)
#define USB_OPREG_STS_EINT (1 << 3)
#define USB_OPREG_STS_PCD (1 << 4)
#define USB_OPREG_STS_RSVDZ2 0xE0
#define USB_OPREG_STS_SSS (1 << 8)
#define USB_OPREG_STS_RSS (1 << 9)
#define USB_OPREG_STS_SRE (1 << 10)
#define USB_OPREG_STS_CNR (1 << 11)
#define USB_OPREG_STS_HCE (1 << 12)
#define USB_OPREG_STS_RSVDZ3 0xFFFFE000

#define USB_OPREG_PAGESIZE 0xFFFF

#define USB_OPREG_DNCTRL_N0 (1 << 0)
#define USB_OPREG_DNCTRL_N1 (1 << 1)
#define USB_OPREG_DNCTRL_N2 (1 << 2)
#define USB_OPREG_DNCTRL_N3 (1 << 3)
#define USB_OPREG_DNCTRL_N4 (1 << 4)
#define USB_OPREG_DNCTRL_N5 (1 << 5)
#define USB_OPREG_DNCTRL_N6 (1 << 6)
#define USB_OPREG_DNCTRL_N7 (1 << 7)
#define USB_OPREG_DNCTRL_N8 (1 << 8)
#define USB_OPREG_DNCTRL_N9 (1 << 9)
#define USB_OPREG_DNCTRL_N10 (1 << 10)
#define USB_OPREG_DNCTRL_N11 (1 << 11)
#define USB_OPREG_DNCTRL_N12 (1 << 12)
#define USB_OPREG_DNCTRL_N13 (1 << 13)
#define USB_OPREG_DNCTRL_N14 (1 << 14)
#define USB_OPREG_DNCTRL_N15 (1 << 15)
#define USB_OPREG_DNCTRL_RSVDP 0xFFFF0000

#define USB_OPREG_CRCR_RCS (1 << 0)
#define USB_OPREG_CRCR_CS (1 << 1)
#define USB_OPREG_CRCR_CA (1 << 2)
#define USB_OPREG_CRCR_CRR (1 << 3)
#define USB_OPREG_CRCR_RSVDP (3 << 4)
#define USB_OPREG_CRCR_RINGPTRLO 0xFFFFFFC0
#define USB_OPREG_CRCR_RINGPTRHI 0xFFFFFFFF

#define USB_OPREG_DCBAAP_RSVDZ 0x3F
#define USB_OPREG_DCBAAP_LOW 0xFFFFFFC0
#define USB_OPREG_DCBAAP_HIGH 0xFFFFFFFF

#define USB_OPREG_CONFIG_MAXSLOTSEN 0xFF
#define USB_OPREG_CONFIG_U3E (1 << 8)
#define USB_OPREG_CONFIG_CIE (1 << 9)
#define USB_OPREG_CONFIG_RSVDP 0xFFFFFC00

#define USB_OPREG_PORTSC_CCS (1 << 0)
#define USB_OPREG_PORTSC_PED (1 << 1)
#define USB_OPREG_PORTSC_RSVDZ1 (1 << 2)
#define USB_OPREG_PORTSC_OCA (1 << 3)
#define USB_OPREG_PORTSC_PR (1 << 4)
#define USB_OPREG_PORTSC_PLS (0xF << 5)
#define USB_OPREG_PORTSC_PP (1 << 9)
#define USB_OPREG_PORTSC_SPEED (0xF << 10)
#define USB_OPREG_PORTSC_PIC (3 << 14)
#define USB_OPREG_PORTSC_LWS (1 << 16)
#define USB_OPREG_PORTSC_CSC (1 << 17)
#define USB_OPREG_PORTSC_PEC (1 << 18)
#define USB_OPREG_PORTSC_WRC (1 << 19)
#define USB_OPREG_PORTSC_OCC (1 << 20)
#define USB_OPREG_PORTSC_PRC (1 << 21)
#define USB_OPREG_PORTSC_PLC (1 << 22)
#define USB_OPREG_PORTSC_CEC (1 << 23)
#define USB_OPREG_PORTSC_CAS (1 << 24)
#define USB_OPREG_PORTSC_WCE (1 << 25)
#define USB_OPREG_PORTSC_WDE (1 << 26)
#define USB_OPREG_PORTSC_WOE (1 << 27)
#define USB_OPREG_PORTSC_RSVDZ2 (3 << 28)
#define USB_OPREG_PORTSC_DR (1 << 30)
#define USB_OPREG_PORTSC_WPR (1 << 31)

#define USB_OPREG_PORTPMSC_L1S (7 << 0)
#define USB_OPREG_PORTPMSC_RWE (1 << 3)
#define USB_OPREG_PORTPMSC_BESL (0xF << 4)
#define USB_OPREG_PORTPMSC_L1SLOT (0xF << 8)
#define USB_OPREG_PORTPMSC_HLE (1 << 16)
#define USB_OPREG_PORTPMSC_RSVDP 0xFFE0000
#define USB_OPREG_PORTPMSC_TEST (0xF << 28)

#define USB_OPREG_PORTLI_LIERRC 0xFFFF
#define USB_OPREG_PORTLI_RLC (0xF << 16)
#define USB_OPREG_PORTLI_TLC (0xF << 20)
#define USB_OPREG_PORTLI_RSVDP (0xFF << 24)

#define USB_OPREG_PORTEXSC_USB3_LSERRC (0xFFFF << 0)
#define USB_OPREG_PORTEXSC_USB3_RSVDP (0xFFFF << 16)

#define USB_OPREG_PORTEXSC_USB2_HIRDM (3 << 0)
#define USB_OPREG_PORTEXSC_USB2_L1TOUT (0xFF << 2)
#define USB_OPREG_PORTEXSC_USB2_BESLD (0xFF << 10)
#define USB_OPREG_PORTEXSC_USB2_RSVDP 0xFFFFC000

#define USB_OPREG_PORTHLPMC_HIRDM (3 << 0)
#define USB_OPREG_PORTHLPMC_L1TOUT (0xFF << 2)
#define USB_OPREG_PORTHLPMC_BESLD (0xF << 10)
#define USB_OPREG_PORTHLPMC_RSVDP 0xFFFFC000

typedef struct {
    uint32_t IMAN;
    uint32_t IMOD;
    uint32_t ERSTSZ;
    uint32_t RsvdP;
    uint64_t ERSTBA;
    uint64_t ERDP;
} PACKED usb_IRset_t;

#define USB_IRSET_IMAN_IP (1 << 0)
#define USB_IRSET_IMAN_IE (1 << 1)
#define USB_IRSET_IMAN_RSVDP 0xFFFFFFFC

#define USB_IRSET_IMOD_INTERVAL (0xFFFF << 0)
#define USB_IRSET_IMOD_COUNTER (0xFFFF << 16)

#define USB_IRSET_ERSTSZ_SIZE (0xFFFF << 0)
#define USB_IRSET_ERSTSZ_RSVDP (0xFFFF << 16)

#define USB_IRSET_ERSTBA_RSVDP 0x3F
#define USB_IRSET_ERSTBA_LOW 0xFFFFFFC0

#define USB_IRSET_ERDP_DESI (7 << 0)
#define USB_IRSET_ERDP_EHB (1 << 3)
#define USB_IRSET_ERDP_LOW 0xFFFFFFF0

typedef struct {
    uint32_t MFINDEX;
    uint8_t RsvdZ[0x20-0x04];
    usb_IRset_t IR[1024];
} PACKED usb_RTregs_t;

#define USB_RTREG_MFINDEX_MFI 0x3FFF
#define USB_RTREG_MFINDEX_RSVDP 0xFFFFC000

typedef struct {
    uint8_t Target;
    uint8_t RsvdZ;
    uint16_t StreamID;
} PACKED usb_DBregs_t;

typedef struct {
    uint32_t rsmhc;
    uint16_t max_exit_lat;
    uint8_t root_hub_port_num;
    uint8_t num_ports;
    uint8_t tthub_slotid;
    uint8_t ttportnum;
    uint16_t ttt_intrtarget;
    uint8_t usbdevaddr;
    uint8_t rsvd[2];
    uint8_t slotstate;
    uint32_t rsvd2[4];  // that will be change if USB_CAPREG_HCCP1_CSZ bit set
} PACKED usb_SlotCtx_t;

#define USB_SLOTCTX_2_TTT_BIT           0
#define USB_SLOTCTX_2_TTT_BITS          2
#define USB_SLOTCTX_2_INTRTARGET_BIT    6
#define USB_SLOTCTX_2_INTRTARGET_BITS   10

#define USB_SLOTCTX_2_TTT_MASK    ((1U<<USB_SLOTCTX_2_TTT_BITS)-1)
#define USB_SLOTCTX_2_TTT_n(n)    ((n)<<USB_SLOTCTX_2_TTT_BIT)
#define USB_SLOTCTX_2_TTT \
    (USB_SLOTCTX_2_TTT_MASK<<USB_SLOTCTX_2_TTT_BIT)

#define USB_SLOTCTX_2_INTRTARGET_MASK \
    ((1U<<USB_SLOTCTX_2_INTRTARGET_BITS)-1)
#define USB_SLOTCTX_2_INTRTARGET_n(n) \
    ((n)<<USB_SLOTCTX_2_INTRTARGET_BIT)
#define USB_SLOTCTX_2_INTRTARGET \
    (USB_SLOTCTX_2_INTRTARGET_MASK<<USB_SLOTCTX_2_INTRTARGET_BIT)

#define USB_SLOTCTX_3_SLOTSTATE_BIT     4
#define USB_SLOTCTX_3_SLOTSTATE_BITS    4

#define USB_SLOTCTX_3_SLOTSTATE_MASK \
    ((1U<<USB_SLOTCTX_3_SLOTSTATE_BITS)-1)
#define USB_SLOTCTX_3_SLOTSTATE_n(n) \
    ((n)<<USB_SLOTCTX_3_SLOTSTATE_BIT)
#define USB_SLOTCTX_3_SLOTSTATE \
    (USB_SLOTCTX_3_SLOTSTATE_MASK<<USB_SLOTCTX_3_SLOTSTATE_BIT)

typedef struct {
    uint8_t ep_state;
    uint8_t mml;
    uint8_t interval;
    uint8_t max_eist_pl_hi;
    uint8_t ceh;
    uint8_t max_burst;
    uint16_t max_packet;
    uint64_t tr_dq_ptr;
    uint16_t avg_trb_len;
    uint16_t max_eist_pl_lo;
    uint32_t rsvd[3];   // that will be change if USB_CAPREG_HCCP1_CSZ bit set
} PACKED usb_EPCtx_t;

#define USB_EPTYPE_INVALID    0
#define USB_EPTYPE_ISOCHOUT   1
#define USB_EPTYPE_BULKOUT    2
#define USB_EPTYPE_INTROUT    3
#define USB_EPTYPE_CTLBIDIR   4
#define USB_EPTYPE_ISOCHIN    5
#define USB_EPTYPE_BULKIN     6
#define USB_EPTYPE_INTRIN     7

typedef struct {
    usb_EPCtx_t OUT;
    usb_EPCtx_t IN;
} PACKED usb_EPCtxIO_t;

typedef struct {
    uint32_t SlotCtx;
    uint32_t EPCtx0;
    usb_EPCtxIO_t EPCtx1;
    usb_EPCtxIO_t EPCtx2;
    usb_EPCtxIO_t EPCtx3;
    usb_EPCtxIO_t EPCtx4;
    usb_EPCtxIO_t EPCtx5;
    usb_EPCtxIO_t EPCtx6;
    usb_EPCtxIO_t EPCtx7;
    usb_EPCtxIO_t EPCtx8;
    usb_EPCtxIO_t EPCtx9;
    usb_EPCtxIO_t EPCtx10;
    usb_EPCtxIO_t EPCtx11;
    usb_EPCtxIO_t EPCtx12;
    usb_EPCtxIO_t EPCtx13;
    usb_EPCtxIO_t EPCtx14;
    usb_EPCtxIO_t EPCtx15;
} PACKED usb_DeviceCtx_t;

typedef struct {
    uint64_t prm;
    uint32_t sts;
    uint32_t ctrl;
} PACKED usb_TRB_t;

#define USB_TRB_CTRL_CYCLE (1 << 0)
#define USB_TRB_CTRL_ENTRB (1 << 1)
#define USB_TRB_CTRL_TYPE (0x3F << 10)

typedef struct {
    uint64_t databuf;
    uint32_t sts;
    uint16_t ctrl;
    uint16_t RsvdZ;
} PACKED usb_NormalTRB_t;

#define USB_NORMALTRB_STS_TRLEN 0x1FFFF
#define USB_NORMALTRB_STS_TDSIZE (0x1F << 17)
#define USB_NORMALTRB_STS_INTTAR (0x3FF << 22)

#define USB_NORMALTRB_CTRL_CYCLE (1 << 0)
#define USB_NORMALTRB_CTRL_ENTRB (1 << 1)
#define USB_NORMALTRB_CTRL_ISP (1 << 2)
#define USB_NORMALTRB_CTRL_NS (1 << 3)
#define USB_NORMALTRB_CTRL_CH (1 << 4)
#define USB_NORMALTRB_CTRL_IOC (1 << 5)
#define USB_NORMALTRB_CTRL_IDT (1 << 6)
#define USB_NORMALTRB_CTRL_RSVDZ (3 << 7)
#define USB_NORMALTRB_CTRL_BEI (1 << 9)
#define USB_NORMALTRB_CTRL_TYPE (0x3F << 10)

typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
    uint32_t sts;
    uint32_t ctrl;
} PACKED usb_SetupTRB_t;

#define USB_SETUPTRB_STS_TRLEN 0x1FFFF
#define USB_SETUPTRB_STS_RSVDZ (0x1F << 17)
#define USB_SETUPTRB_STS_INTTAR (0x3FF << 22)

#define USB_SETUPTRB_CTRL_CYCLE (1 << 0)
#define USB_SETUPTRB_CTRL_RSVDZ1 (0xF << 1)
#define USB_SETUPTRB_CTRL_IOC (1 << 5)
#define USB_SETUPTRB_CTRL_IDT (1 << 6)
#define USB_SETUPTRB_CTRL_RSVDZ2 (7 << 7)
#define USB_SETUPTRB_CTRL_TYPE (0x3F << 10)
#define USB_SETUPTRB_CTRL_TRT (3 << 16)
#define USB_SETUPTRB_CTRL_RSVDZ3 (0x3FFF << 18)

typedef struct {
    uint64_t databuf;
    uint32_t sts;
    uint32_t ctrl;
} PACKED usb_DataTRB_t;

#define USB_DATATRB_STS_TRLEN 0x1FFFF
#define USB_DATATRB_STS_TDSIZE (0x1F << 17)
#define USB_DATATRB_STS_INTTAR (0x3FF << 22)

#define USB_DATATRB_CTRL_CYCLE (1 << 0)
#define USB_DATATRB_CTRL_ENTRB (1 << 1)
#define USB_DATATRB_CTRL_ISP (1 << 2)
#define USB_DATATRB_CTRL_NS (1 << 3)
#define USB_DATATRB_CTRL_CH (1 << 4)
#define USB_DATATRB_CTRL_IOC (1 << 5)
#define USB_DATATRB_CTRL_IDT (1 << 6)
#define USB_DATATRB_CTRL_RSVDZ1 (7 << 7)
#define USB_DATATRB_CTRL_TYPE (0x3F << 10)
#define USB_DATATRB_CTRL_DIR (1 << 16)
#define USB_DATATRB_CTRL_RSVDZ2 (0x7FFF << 17)

typedef struct {
    uint64_t RsvdZ;
    uint32_t sts;
    uint32_t ctrl;
} PACKED usb_StatusTRB_t;

#define USB_STATUSTRB_STS_RSVDZ 0x3FFFFF
#define USB_STATUSTRB_STS_INTTAR (0x3FF << 22)

#define USB_STATUSTRB_STS_CYCLE (1 << 0)
#define USB_STATUSTRB_STS_ENTRB (1 << 1)
#define USB_STATUSTRB_STS_RSVDZ1 (3 << 2)
#define USB_STATUSTRB_STS_CH (1 << 4)
#define USB_STATUSTRB_STS_IOC (1 << 5)
#define USB_STATUSTRB_STS_RSVDZ2 (0xF << 6)
#define USB_STATUSTRB_STS_TYPE (0x3F << 10)
#define USB_STATUSTRB_STS_DIR (1 << 16)
#define USB_STATUSTRB_STS_RSVDZ3 (0x7FFF << 17)

typedef struct {
    uint64_t TRBptr;
    uint32_t sts;
    uint32_t ctrl;
} PACKED usb_TREventTRB_t;

#define USB_TREVENTTRB_STS_TRLEN 0xFFFFFF
#define USB_TREVENTTRB_STS_CODE (0xFF << 24)

#define USB_TREVENTTRB_CTRL_CYCLE (1 << 0)
#define USB_TREVENTTRB_CTRL_RSVDZ1 (1 << 1)
#define USB_TREVENTTRB_CTRL_ED (1 << 2)
#define USB_TREVENTTRB_CTRL_RSVDZ2 (0x7F << 3)
#define USB_TREVENTTRB_CTRL_TYPE (0x3F << 10)
#define USB_TREVENTTRB_CTRL_ENDPID (0x1F << 16)
#define USB_TREVENTTRB_CTRL_RSVDZ3 (7 << 21)
#define USB_TREVENTTRB_CTRL_SLOTID (0xFF << 24)

typedef struct {
    uint64_t cmdTRBptr;
    uint32_t sts;
    uint32_t ctrl;
} PACKED usb_CMDEventTRB_t;

#define USB_CMDEVENTTRB_CMDTRBPTR_RSVDZ 0xFF
#define USB_CMDEVENTTRB_CMDTRBPTR_LO 0xFFFFFF00
#define USB_CMDEVENTTRB_CMDTRBPTR_HI 0xFFFFFFFF

#define USB_CMDEVENTTRB_STS_PARAM 0xFFFFFF
#define USB_CMDEVENTTRB_STS_CODE (0xFF << 24)

#define USB_CMDEVENTTRB_CTRL_CYCLE (1 << 0)
#define USB_CMDEVENTTRB_CTRL_RSVDZ (0x1FF << 1)
#define USB_CMDEVENTTRB_CTRL_TYPE (0x3F << 10)
#define USB_CMDEVENTTRB_CTRL_VFID (0xFF << 16)
#define USB_CMDEVENTTRB_CTRL_SLOTID (0xFF << 24)

typedef struct {
    uint64_t RsvdZ1;
    uint32_t RsvdZ2;
    uint32_t ctrl;
} PACKED usb_SlotCMDTRB_t;

#define USB_ENABLESLOTTRB_CTRL_CYCLE (1 << 0)
#define USB_ENABLESLOTTRB_CTRL_RSVDZ1 (0x1FF << 1)
#define USB_ENABLESLOTTRB_CTRL_TYPE (0x3F << 10)
#define USB_ENABLESLOTTRB_CTRL_SLOTTYPE (0x1F << 16)
#define USB_ENABLESLOTTRB_CTRL_RSVDZ2 (0x7FF << 21)

#define USB_DISABLESLOTTRB_CTRL_CYCLE (1 << 0)
#define USB_DISABLESLOTTRB_CTRL_RSVDZ1 (0x1FF << 1)
#define USB_DISABLESLOTTRB_CTRL_TYPE (0x3F << 10)
#define USB_DISABLESLOTTRB_CTRL_RSVDZ2 (0xFF << 16)
#define USB_DISABLESLOTTRB_CTRL_SLOTID (0xFF << 24)

typedef struct {
    uint32_t drop_bits;
    uint32_t add_bits;
    uint32_t rsvd[5];
    uint8_t cfg;
    uint8_t iface_num;
    uint8_t alternate;
    uint8_t rsvd2;
} usb_inpctlctx_t;

typedef struct {
    usb_inpctlctx_t inpctl;
    usb_SlotCtx_t slotctx;
    usb_EPCtx_t epctx[32];
} usb_inpctx_t;

typedef struct {
    uint64_t ring_physaddr;
    uint16_t rsvd;
    uint16_t intrtarget;
    uint8_t c_tc_ch_ioc;
    uint8_t trb_type;
    uint16_t rsvd2;
} usb_LinkTRB_t;

// 6.4.6 TRB Types

// Transfer ring
#define USB_TRB_TYPE_NORMAL             1
#define USB_TRB_TYPE_SETUP              2
#define USB_TRB_TYPE_DATA               3
#define USB_TRB_TYPE_STATUS             4
#define USB_TRB_TYPE_ISOCH              5
// Command ring or Transfer ring
#define USB_TRB_TYPE_LINK               6
// Transfer ring
#define USB_TRB_TYPE_EVTDATA            7
#define USB_TRB_TYPE_NOOP               8
// Command ring
#define USB_TRB_TYPE_ENABLESLOTCMD      9
#define USB_TRB_TYPE_DISABLESLOTCMD     10
#define USB_TRB_TYPE_ADDRDEVCMD         11
#define USB_TRB_TYPE_CONFIGUREEPCMD     12
#define USB_TRB_TYPE_EVALCTXCMD         13
#define USB_TRB_TYPE_RESETEPCMD         14
#define USB_TRB_TYPE_STOPEPCMD          15
#define USB_TRB_TYPE_SETTRDEQPTRCMD     16
#define USB_TRB_TYPE_RESETDEVCMD        17
#define USB_TRB_TYPE_FORCEEVTCMD        18
#define USB_TRB_TYPE_NEGOBWCMD          19
#define USB_TRB_TYPE_SETLATTOLVALCMD    20
#define USB_TRB_TYPE_GETPORTBWCMD       21
#define USB_TRB_TYPE_FORCEHDRCMD        22
#define USB_TRB_TYPE_NOOPCMD            23
// Event ring
#define USB_TRB_TYPE_XFEREVT            32
#define USB_TRB_TYPE_CMDCOMPEVT         33
#define USB_TRB_TYPE_PORTSTSCHGEVT      34
#define USB_TRB_TYPE_BWREQEVT           35
#define USB_TRB_TYPE_DBEVT              36
#define USB_TRB_TYPE_HOSTCTLEVT         37
#define USB_TRB_TYPE_DEVNOTIFEVT        38
#define USB_TRB_TYPE_MFINDEXWRAPEVT     39

typedef struct {
    uint64_t base;
    uint16_t trb_count; // minimum 16 trb
    uint16_t resvd;
    uint32_t resvd2;
} usb_evtring_seg_t;

typedef struct {
    uint32_t data[3];
    uint16_t flags;
    uint8_t id;
    uint8_t slotid;
} usb_EventTRB_t;

#define USB_EVT_FLAGS_C_BIT     0
#define USB_EVT_FLAGS_C         (1U<<USB_EVT_FLAGS_C_BIT)

#define USB_EVT_FLAGS_TYPE_BIT  10
#define USB_EVT_FLAGS_TYPE_BITS 6
#define USB_EVT_FLAGS_TYPE_MASK ((1U<<USB_EVT_FLAGS_TYPE_BITS)-1)
#define USB_EVT_FLAGS_TYPE_n(n) ((n)<<USB_EVT_FLAGS_TYPE_BIT)
#define USB_EVT_FLAGS_TYPE \
    (USB_EVT_FLAGS_TYPE_MASK<<USB_EVT_FLAGS_TYPE_BIT)

#define USB_EVENTRING_TRBCOUNT 16

#define USB_DBREG

typedef struct {
    bool active; // Driver açısından cihaz kullanıma hazır mı kontrolü
    uint8_t slot; // Enable Slot Command sonrası DCBAAP dizisindeki index
    uint8_t speed; // PORTSC.Speed okuması ile belirlenir (USB1.x / USB2 / USB3)
    uint8_t port; // Root hub portu takip için
    uint8_t endpointCount; // Device descriptor’dan okunur
    uint32_t packsize; // Endpoint 0 için, setup/control transfer için gerekli
    void* devctx; // DCBAAP dizindeki Device Context pointer’ı
} usb_Port_t;

#define USB_TRBCOUNT 256

bool usb_InitLock = false;

usb_Capregs_t* usb_Capregs;
usb_Opregs_t* usb_Opregs;
usb_RTregs_t* usb_RTregs;
usb_DBregs_t* usb_DBregs;
usb_TRB_t* usb_CmdRing;
usb_evtring_seg_t* usb_ERSTent;
usb_EventTRB_t* usb_EventRing;

usb_Port_t* usb_PortV;

void usb_doorbell(uint8_t slot, uint8_t ring) {
    if (!usb_InitLock) { ERR("USB host controller not initialized"); return; }
    usb_DBregs[slot].Target = ring;
    usb_DBregs[slot].RsvdZ = 0;
    usb_DBregs[slot].StreamID = 0;
}

int usb_portInit(uint8_t port) {
    if (!usb_InitLock || !(usb_Opregs->port[port].PORTSC & USB_OPREG_PORTSC_CCS)) { return -1; }
    usb_Opregs->port[port].PORTSC |= USB_OPREG_PORTSC_PR;
    uint32_t timeout = 100000; while ((usb_Opregs->port[port].PORTSC & USB_OPREG_PORTSC_PR) && --timeout);
    usb_Opregs->port[port].PORTSC |= USB_OPREG_PORTSC_PRC;
    if (!(usb_Opregs->port[port].PORTSC & USB_OPREG_PORTSC_PED)) { return 1; }
    usb_PortV->active = true;
    usb_PortV->speed = (usb_Opregs->port[port].PORTSC & USB_OPREG_PORTSC_SPEED) >> 10;
    usb_PortV->port = port;
    usb_SlotCMDTRB_t* trb = (usb_SlotCMDTRB_t*)usb_CmdRing;
    trb[0].ctrl = 0;
    trb[0].ctrl |= USB_ENABLESLOTTRB_CTRL_CYCLE;
    trb[0].ctrl |= USB_TRB_TYPE_ENABLESLOTCMD << 10;
    usb_doorbell(0, 0);
    for (int i = 0; i < USB_EVENTRING_TRBCOUNT; ++i) {
        if (usb_EventRing[i].flags != 0) {
            printf(
        "i: %d, data[0]: 0x%x, data[1]: 0x%x, data[2]: 0x%x, flags: 0x%x, id: 0x%x, slotid: 0x%x\n", i,
        usb_EventRing[i].data[0],
        usb_EventRing[i].data[1],
        usb_EventRing[i].data[2],
        usb_EventRing[i].flags,
        usb_EventRing[i].id,
        usb_EventRing[i].slotid
    );
        uint32_t* val = (uint32_t*)&usb_EventRing[i];
        printf("0x%x\n", val[0]);
        printf("0x%x\n", val[1]);
        printf("0x%x\n", val[2]);
        printf("0x%x\n", val[3]);
        }
    }
    return 0;
}

void usb_process() {
    if (!usb_InitLock) { ERR("USB host controller not initialized"); exit(); return; }
    while (true) {
        if (usb_Opregs->USBSTS & USB_OPREG_STS_HSE) { ERR("USB host controller system error"); exit(); return; }
        if (usb_Opregs->USBSTS & USB_OPREG_STS_HCH) { ERR("USB host controller in halt state"); exit(); return; }
        if (usb_Opregs->USBSTS & USB_OPREG_STS_CNR) { ERR("USB host controller not ready"); exit(); return; }
        for (uint32_t i = 0; i < (usb_Opregs->CONFIG & USB_OPREG_CONFIG_MAXSLOTSEN); ++i) {
            if (usb_Opregs->port[i].PORTSC & USB_OPREG_PORTSC_CSC) {
                if (usb_Opregs->port[i].PORTSC & USB_OPREG_PORTSC_CCS) {
                    switch (usb_portInit(i)) {
                        case 0: { INFO("Port %d: Connected successfully", i); break; }
                        case -1: { ERR("Port %d: Unable to start device", i); break; }
                        case 1: { ERR("Port %d: Device cannot start", i); break; }
                        default: { ERR("Port %d: Unknown error", i); }
                    }
                } else {
                    INFO("Port %d: Disconnected", i);
                } usb_Opregs->port[i].PORTSC |= USB_OPREG_PORTSC_CSC;
            }
        } yield();
    }
}

int usb_init(devbus_Device_t* dev) {
    if (usb_InitLock) { ERR("Multiple USB hosts not supported"); return -1; }
    if (dev->interface != 0x30) { ERR("Only xHCI is supported"); return -1; }
    INFO("xHCI release number: 0x%x", devbus_read(dev->bus, dev->slot, dev->func, 24) & 0xFF);
    devbus_BAR_t bar; devbus_getBAR(&bar, dev->bus, dev->slot, dev->func, 0);
    if (bar.type != DEVBUS_BARTYPE_MM) { ERR("Device BAR is not memory-mapped"); return -1; }
    usb_Capregs = (usb_Capregs_t*)bar.addr;
    INFO("xHCI version: 0x%x", usb_Capregs->HCIVERSION);
    usb_Opregs = (usb_Opregs_t*)(bar.addr + usb_Capregs->CAPLENGTH);
    usb_RTregs = (usb_RTregs_t*)(bar.addr + (usb_Capregs->RTSOFF & USB_CAPREG_RTSOFF));
    usb_DBregs = (usb_DBregs_t*)(bar.addr + (usb_Capregs->DBOFF & USB_CAPREG_DBOFF));
    // --------------------------------------------------------------------------------------
    while (usb_Opregs->USBSTS & USB_OPREG_STS_CNR);
    // --------------------------------------------------------------------------------------
    usb_Opregs->CONFIG &= (uint32_t)~USB_OPREG_CONFIG_MAXSLOTSEN;
    usb_Opregs->CONFIG |= usb_Capregs->HCSPARAMS1 & USB_CAPREG_HCSP1_MAXSLOTS;
    // --------------------------------------------------------------------------------------
    uint32_t pagesize = 0;
    for (int i = 0; i < 32; ++i) { if (usb_Opregs->PAGESIZE & (1 << i)) { pagesize = 1 << (12 + i); break; } }
    uint64_t* dcbaa = (uint64_t*)calloc(usb_Opregs->CONFIG & USB_OPREG_CONFIG_MAXSLOTSEN, sizeof(uint64_t));
    if (dcbaa == NULL) { ERR("Out of memory"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST; return -1; }
    if (((size_t)dcbaa & ~(MEMORY_BLKSIZE-1)) != (size_t)dcbaa)
        { ERR("Memory block not aligned"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST; free(dcbaa); return -1; }
    void* dcbase = calloc(usb_Opregs->CONFIG & USB_OPREG_CONFIG_MAXSLOTSEN, pagesize);
    if (dcbase == NULL) { ERR("Out of memory"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST; free(dcbaa); return -1; }
    if (((size_t)dcbase & ~(MEMORY_BLKSIZE-1)) != (size_t)dcbase) {
        ERR("Memory block not aligned"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST;
        free(dcbaa); free(dcbase); return -1;
    }
    for (uint32_t i = 0; i < (usb_Opregs->CONFIG & USB_OPREG_CONFIG_MAXSLOTSEN); ++i)
        { dcbaa[i] = (size_t)dcbase + (i * pagesize); }
    usb_Opregs->DCBAAP = (uint64_t)(size_t)dcbaa & (uint64_t)~USB_OPREG_DCBAAP_RSVDZ;
    // --------------------------------------------------------------------------------------
    // ! Command ring not working, fix it
    usb_CmdRing = (usb_TRB_t*)calloc(USB_TRBCOUNT, sizeof(usb_TRB_t));
    if (usb_CmdRing == NULL) { ERR("Out of memory"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST;
        free(dcbaa); free(dcbase); return -1; }
    if (((size_t)usb_CmdRing & ~(MEMORY_BLKSIZE-1)) != (size_t)usb_CmdRing) {
        ERR("Memory block not aligned"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST;
        free(dcbaa); free(dcbase); free(usb_CmdRing); return -1;
    }
    // usb_Opregs->CRCR = (uint64_t)((size_t)usb_CmdRing & USB_OPREG_CRCR_RINGPTRLO);
    uint64_t crptr = (uint64_t)(size_t)usb_CmdRing & ~0x3FULL; // [63:6] pointer
    usb_Opregs->CRCR = crptr | 1ULL;                           // bit0 = RCS=1
    // --------------------------------------------------------------------------------------
    usb_PortV = (usb_Port_t*)calloc(usb_Opregs->CONFIG & USB_OPREG_CONFIG_MAXSLOTSEN, sizeof(usb_Port_t));
    if (usb_PortV == NULL) { ERR("Out of memory"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST;
        free(dcbaa); free(dcbase); free(usb_CmdRing); return -1; }
    // --------------------------------------------------------------------------------------
    usb_ERSTent = (usb_evtring_seg_t*)calloc(1, sizeof(usb_evtring_seg_t));
    if (usb_ERSTent == NULL) { ERR("Out of memory"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST;
        free(usb_ERSTent); free(dcbaa); free(dcbase); free(usb_CmdRing); return -1; }
    usb_EventRing = (usb_EventTRB_t*)calloc(USB_EVENTRING_TRBCOUNT, sizeof(usb_EventTRB_t));
    if (usb_EventRing == NULL) { ERR("Out of memory"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST;
        free(usb_EventRing); free(dcbaa); free(dcbase); free(usb_CmdRing); return -1; }
    usb_ERSTent->base = (uint64_t)(size_t)usb_EventRing;
    usb_ERSTent->trb_count = USB_EVENTRING_TRBCOUNT;
    usb_RTregs->IR[0].IMAN = 0;
    usb_RTregs->IR[0].IMOD = 0;
    usb_RTregs->IR[0].ERSTSZ = 1;
    usb_RTregs->IR[0].ERSTBA = (uint64_t)(size_t)usb_ERSTent;
    usb_RTregs->IR[0].ERDP = (uint64_t)(size_t)usb_EventRing;
    // --------------------------------------------------------------------------------------
    usb_Opregs->USBCMD |= USB_OPREG_CMD_RS;
    uint32_t timeout = 100000;
    while ((usb_Opregs->USBSTS & USB_OPREG_STS_HCH) && --timeout);
    if (usb_Opregs->USBSTS & USB_OPREG_STS_HCH) {
        ERR("Host controller activation timed out"); usb_Opregs->USBCMD |= USB_OPREG_CMD_HCRST;
        free(dcbaa); free(dcbase); free(usb_CmdRing); return -1;
    }
    // --------------------------------------------------------------------------------------
    INFO("xHCI driver initialized"); usb_InitLock = true;
    for (uint32_t i = 0; i < (usb_Opregs->CONFIG & USB_OPREG_CONFIG_MAXSLOTSEN); ++i) {
        if (usb_Opregs->port[i].PORTSC & USB_OPREG_PORTSC_CCS) {
            switch (usb_portInit(i)) {
                case 0: { INFO("Port %d: Connected successfully", i); break; }
                case -1: { ERR("Port %d: Unable to start device", i); break; }
                case 1: { ERR("Port %d: Device cannot start", i); break; }
                default: { ERR("Port %d: Unknown error", i); }
            }
        }
    }
    return 0;
}