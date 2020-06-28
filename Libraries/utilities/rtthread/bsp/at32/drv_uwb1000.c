#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>

#include "drivers/rt_sensor.h"



#define DW_DEBUG		1

#if ( DW_DEBUG == 1 )
#include <stdio.h>
#define DW_TRACE	printf
#else
#define DW_TRACE(...)
#endif


/* device ID */
#define DW1000_DEVICE_ID        ((uint32_t)0xDECA0130)

/* dw1000 reg */
#define DW1000_DEV_ID           ((uint8_t)0x00)     // Lens = 4,    RO,  Device Identifier – includes device type and revision info
#define DW1000_EUI              ((uint8_t)0x01)     // Lens = 8,    RW,  Extended Unique Identifier
#define DW1000_PANADR           ((uint8_t)0x03)     // Lens = 4,    RW,  PAN Identifier and Short Address
#define DW1000_SYS_CFG          ((uint8_t)0x04)     // Lens = 4,    RW,  System Configuration bitmap
#define DW1000_SYS_TIME         ((uint8_t)0x06)     // Lens = 5,    RO,  System Time Counter (40-bit)
#define DW1000_TX_FCTRL         ((uint8_t)0x08)     // Lens = 5,    RW,  Transmit Frame Control
#define DW1000_TX_BUFFER        ((uint8_t)0x09)     // Lens = 1024, WO,  Transmit Data Buffer
#define DW1000_DX_TIME          ((uint8_t)0x0A)     // Lens = 5,    RW,  Delayed Send or Receive Time (40-bit)
#define DW1000_RX_FWTO          ((uint8_t)0x0C)     // Lens = 2,    RW,  Receive Frame Wait Timeout Period
#define DW1000_SYS_CTRL         ((uint8_t)0x0D)     // Lens = 4,    SRW, System Control Register
#define DW1000_SYS_MASK         ((uint8_t)0x0E)     // Lens = 4,    RW,  System Event Mask Register
#define DW1000_SYS_STATUS       ((uint8_t)0x0F)     // Lens = 5,    SRW, System Event Status Register
#define DW1000_RX_FINFO         ((uint8_t)0x10)     // Lens = 4,    ROD, RX Frame Information (in double buffer set)
#define DW1000_RX_BUFFER        ((uint8_t)0x11)     // Lens = 1024, ROD, Receive Data (in double buffer set)
#define DW1000_RX_FQUAL         ((uint8_t)0x12)     // Lens = 8,    ROD, Rx Frame Quality information (in double buffer set)
#define DW1000_RX_TTCKI         ((uint8_t)0x13)     // Lens = 4,    ROD, Receiver Time Tracking Interval (in double buffer set)
#define DW1000_RX_TTCKO         ((uint8_t)0x14)     // Lens = 5,    ROD, Receiver Time Tracking Offset (in double buffer set)
#define DW1000_RX_TIME          ((uint8_t)0x15)     // Lens = 14,   ROD, Receive Message Time of Arrival (in double buffer set)
#define DW1000_TX_TIME          ((uint8_t)0x17)     // Lens = 10,   RO,  Transmit Message Time of Sending
#define DW1000_TX_ANTD          ((uint8_t)0x18)     // Lens = 2,    RW,  16-bit Delay from Transmit to Antenna
#define DW1000_SYS_STATE        ((uint8_t)0x19)     // Lens = 5,    RO,  System State information
#define DW1000_ACK_RESP_T       ((uint8_t)0x1A)     // Lens = 4,    RW,  Acknowledgement Time and Response Time
#define DW1000_RX_SNIFF         ((uint8_t)0x1D)     // Lens = 4,    RW,  Pulsed Preamble Reception Configuration
#define DW1000_TX_POWER         ((uint8_t)0x1E)     // Lens = 4,    RW,  TX Power Control
#define DW1000_CHAN_CTRL        ((uint8_t)0x1F)     // Lens = 4,    RW,  Channel Control
#define DW1000_USR_SFD          ((uint8_t)0x21)     // Lens = 41,   RW,  User-specified short/long TX/RX SFD sequences

#define DW1000_AGC_CTRL         ((uint8_t)0x23)     // Lens = 32,   RW,  Automatic Gain Control configuration
#define DW1000_SUB_AGC_CTRL1    ((uint8_t)0x02)     // Lens = 2,    RW,  AGC Control #1
#define DW1000_SUB_AGC_TUNE1    ((uint8_t)0x04)     // Lens = 2,    RW,  AGC Tuning register 1
#define DW1000_SUB_AGC_TUNE2    ((uint8_t)0x0C)     // Lens = 4,    RW,  AGC Tuning register 2
#define DW1000_SUB_AGC_TUNE3    ((uint8_t)0x12)     // Lens = 2,    RW,  AGC Tuning register 3
#define DW1000_SUB_AGC_STAT1    ((uint8_t)0x1E)     // Lens = 3,    RO,  AGC Status

#define DW1000_EXT_SYNC         ((uint8_t)0x24)     // Lens = 12,   RW,  External synchronisation control
#define DW1000_SUB_EC_CTRL      ((uint8_t)0x00)     // Lens = 4,    RW,  External clock synchronisation counter configuration
#define DW1000_SUB_EC_RXTC      ((uint8_t)0x04)     // Lens = 4,    RO,  External clock counter captured on RMARKER
#define DW1000_SUB_EC_GOLP      ((uint8_t)0x08)     // Lens = 4,    RO,  External clock offset to first path 1 GHz counter

#define DW1000_ACC_MEM          ((uint8_t)0x25)     // Lens = 4064, RO,  Read access to accumulator data

#define DW1000_GPIO_CTRL        ((uint8_t)0x26)     // Lens = 44,   RW,  Peripheral register bus 1 access - GPIO control
#define DW1000_SUB_GPIO_MODE    ((uint8_t)0x00)     // Lens = 4,    RW,  GPIO Mode Control Register
#define DW1000_SUB_GPIO_DIR     ((uint8_t)0x08)     // Lens = 4,    RW,  GPIO Direction Control Register
#define DW1000_SUB_GPIO_DOUT    ((uint8_t)0x0C)     // Lens = 4,    RW,  GPIO Data Output register
#define DW1000_SUB_GPIO_IRQE    ((uint8_t)0x10)     // Lens = 4,    RW,  GPIO Interrupt Enable
#define DW1000_SUB_GPIO_ISEN    ((uint8_t)0x14)     // Lens = 4,    RW,  GPIO Interrupt Sense Selection
#define DW1000_SUB_GPIO_IMODE   ((uint8_t)0x18)     // Lens = 4,    RW,  GPIO Interrupt Mode (Level / Edge)
#define DW1000_SUB_GPIO_IBES    ((uint8_t)0x1C)     // Lens = 4,    RW,  GPIO Interrupt “Both Edge” Select
#define DW1000_SUB_GPIO_ICLR    ((uint8_t)0x20)     // Lens = 4,    RW,  GPIO Interrupt Latch Clear
#define DW1000_SUB_GPIO_IDBE    ((uint8_t)0x24)     // Lens = 4,    RW,  GPIO Interrupt De-bounce Enable
#define DW1000_SUB_GPIO_RAW     ((uint8_t)0x28)     // Lens = 4,    RO,  GPIO raw state

#define DW1000_DRX_CONF         ((uint8_t)0x27)     // Lens = 44,   RW,  Digital Receiver configuration
#define DW1000_SUB_DRX_TUNE0b   ((uint8_t)0x02)     // Lens = 2,    RW,  Digital Tuning Register 0b
#define DW1000_SUB_DRX_TUNE1a   ((uint8_t)0x04)     // Lens = 2,    RW,  Digital Tuning Register 1a
#define DW1000_SUB_DRX_TUNE1b   ((uint8_t)0x06)     // Lens = 2,    RW,  Digital Tuning Register 1b
#define DW1000_SUB_DRX_TUNE2    ((uint8_t)0x08)     // Lens = 4,    RW,  Digital Tuning Register 2
#define DW1000_SUB_DRX_SFDTOC   ((uint8_t)0x20)     // Lens = 2,    RW,  SFD timeout
#define DW1000_SUB_DRX_PRETOC   ((uint8_t)0x24)     // Lens = 2,    RW,  Preamble detection timeout
#define DW1000_SUB_DRX_TUNE4H   ((uint8_t)0x26)     // Lens = 2,    RW,  Digital Tuning Register 4H
#define DW1000_SUB_RXPACC_NOSAT ((uint8_t)0x2C)     // Lens = 2,    RO,  Unsaturated accumulated preamble symbols

#define DW1000_RF_CONF          ((uint8_t)0x28)     // Lens = 58,   RW,  Analog RF Configuration
#define DW1000_SUB_RF_CONF      ((uint8_t)0x00)     // Lens = 4,    RW,  RF Configuration Register
#define DW1000_SUB_RF_RXCTRLH   ((uint8_t)0x0B)     // Lens = 1,    RW,  Analog RX Control Register
#define DW1000_SUB_RF_TXCTRL    ((uint8_t)0x0C)     // Lens = 4,    RW,  Analog TX Control Register
#define DW1000_SUB_RF_STATUS    ((uint8_t)0x2C)     // Lens = 4,    RO,  RF Status Register
#define DW1000_SUB_LDOTUNE      ((uint8_t)0x30)     // Lens = 5,    RW,  LDO voltage tuning

#define DW1000_TX_CAL           ((uint8_t)0x2A)     // Lens = 52,   RW,  Transmitter calibration block
#define DW1000_SUB_TC_SARC      ((uint8_t)0x00)     // Lens = 2,    RW,  Transmitter Calibration – SAR control
#define DW1000_SUB_TC_SARL      ((uint8_t)0x03)     // Lens = 3,    RO,  Transmitter Calibration – Latest SAR readings
#define DW1000_SUB_TC_SARW      ((uint8_t)0x06)     // Lens = 2,    RO,  Transmitter Calibration – SAR readings at last Wake-Up
#define DW1000_SUB_TC_PGDELAY   ((uint8_t)0x0B)     // Lens = 1,    RW,  Transmitter Calibration – Pulse Generator Delay
#define DW1000_SUB_TC_PGTEST    ((uint8_t)0x0C)     // Lens = 1,    RW,  Transmitter Calibration – Pulse Generator Test

#define DW1000_FS_CTRL          ((uint8_t)0x2B)     // Lens = 21,   RW,  Frequency synthesiser control block
#define DW1000_SUB_FS_PLLCFG    ((uint8_t)0x07)     // Lens = 4,    RW,  Frequency synthesiser – PLL configuration
#define DW1000_SUB_FS_PLLTUNE   ((uint8_t)0x0B)     // Lens = 1,    RW,  Frequency synthesiser – PLL Tuning
#define DW1000_SUB_FS_XTALT     ((uint8_t)0x0E)     // Lens = 1,    RW,  Frequency synthesiser – Crystal trim

#define DW1000_AON              ((uint8_t)0x2C)     // Lens = 12,   RW,  Always-On register set
#define DW1000_SUB_AON_WCFG     ((uint8_t)0x00)     // Lens = 2,    RW,  AON Wakeup Configuration Register
#define DW1000_SUB_AON_CTRL     ((uint8_t)0x02)     // Lens = 1,    RW,  AON Control Register
#define DW1000_SUB_AON_RDAT     ((uint8_t)0x03)     // Lens = 1,    RW,  AON Direct Access Read Data Result
#define DW1000_SUB_AON_ADDR     ((uint8_t)0x04)     // Lens = 1,    RW,  AON Direct Access Address
#define DW1000_SUB_AON_CFG0     ((uint8_t)0x06)     // Lens = 4,    RW,  AON Configuration Register 0
#define DW1000_SUB_AON_CFG1     ((uint8_t)0x0A)     // Lens = 2,    RW,  AON Configuration Register 1

#define DW1000_OTP_IF           ((uint8_t)0x2D)     // Lens = 18,   RW,  One Time Programmable Memory Interface
#define DW1000_SUB_OTP_WDAT     ((uint8_t)0x00)     // Lens = 4,    RW,  OTP Write Data
#define DW1000_SUB_OTP_ADDR     ((uint8_t)0x04)     // Lens = 2,    RW,  OTP Address
#define DW1000_SUB_OTP_CTRL     ((uint8_t)0x06)     // Lens = 2,    RW,  OTP Control
#define DW1000_SUB_OTP_STAT     ((uint8_t)0x08)     // Lens = 2,    RW,  OTP Status
#define DW1000_SUB_OTP_RDAT     ((uint8_t)0x0A)     // Lens = 4,    R,   OTP Read Data
#define DW1000_SUB_OTP_SRDAT    ((uint8_t)0x0E)     // Lens = 4,    RW,  OTP SR Read Data
#define DW1000_SUB_OTP_SF       ((uint8_t)0x12)     // Lens = 1,    RW,  OTP Special Function

#define DW1000_LDE_CTRL         ((uint8_t)0x2E)     // Lens =  -    RW,  Leading edge detection control block
#define DW1000_SUB_LDE_THRESH   ((uint16_t)0x0000)  // Lens = 2,    RO,  LDE Threshold report
#define DW1000_SUB_LDE_CFG1     ((uint16_t)0x0806)  // Lens = 1,    RW,  LDE Configuration Register 1
#define DW1000_SUB_LDE_PPINDX   ((uint16_t)0x1000)  // Lens = 2,    RO,  LDE Peak Path Index
#define DW1000_SUB_LDE_PPAMPL   ((uint16_t)0x1002)  // Lens = 2,    RO,  LDE Peak Path Amplitude
#define DW1000_SUB_LDE_RXANTD   ((uint16_t)0x1804)  // Lens = 2,    RW,  LDE Receive Antenna Delay configuration
#define DW1000_SUB_LDE_CFG2     ((uint16_t)0x1806)  // Lens = 2,    RW,  LDE Configuration Register 2
#define DW1000_SUB_LDE_REPC     ((uint16_t)0x2804)  // Lens = 2,    RW,  LDE Replica Coefficient configuration

#define DW1000_DIG_DIAG         ((uint8_t)0x2F)     // Lens = 41,   RW,  Digital Diagnostics Interface
#define DW1000_SUB_EVC_CTRL     ((uint8_t)0x00)     // Lens = 4,    SRW, Event Counter Control
#define DW1000_SUB_EVC_PHE      ((uint8_t)0x04)     // Lens = 2,    RO,  PHR Error Counter
#define DW1000_SUB_EVC_RSE      ((uint8_t)0x06)     // Lens = 2,    RO,  RSD Error Counter
#define DW1000_SUB_EVC_FCG      ((uint8_t)0x08)     // Lens = 2,    RO,  Frame Check Sequence Good Counter
#define DW1000_SUB_EVC_FCE      ((uint8_t)0x0A)     // Lens = 2,    RO,  Frame Check Sequence Error Counter
#define DW1000_SUB_EVC_FFR      ((uint8_t)0x0C)     // Lens = 2,    RO,  Frame Filter Rejection Counter
#define DW1000_SUB_EVC_OVR      ((uint8_t)0x0E)     // Lens = 2,    RO,  RX Overrun Error Counter
#define DW1000_SUB_EVC_STO      ((uint8_t)0x10)     // Lens = 2,    RO,  SFD Timeout Counter
#define DW1000_SUB_EVC_PTO      ((uint8_t)0x12)     // Lens = 2,    RO,  Preamble Timeout Counter
#define DW1000_SUB_EVC_FWTO     ((uint8_t)0x14)     // Lens = 2,    RO,  RX Frame Wait Timeout Counter
#define DW1000_SUB_EVC_TXFS     ((uint8_t)0x16)     // Lens = 2,    RO,  TX Frame Sent Counter
#define DW1000_SUB_EVC_HPW      ((uint8_t)0x18)     // Lens = 2,    RO,  Half Period Warning Counter
#define DW1000_SUB_EVC_TPW      ((uint8_t)0x1A)     // Lens = 2,    RO,  Transmitter Power-Up Warning Counter
#define DW1000_SUB_DIAG_TMC     ((uint8_t)0x24)     // Lens = 2,    RW,  Test Mode Control Register

#define DW1000_PMSC             ((uint8_t)0x36)     // Lens = 48,   RW,  Power Management System Control Block
#define DW1000_SUB_PMSC_CTRL0   ((uint8_t)0x00)     // Lens = 4,    RW,  PMSC Control Register 0
#define DW1000_SUB_PMSC_CTRL1   ((uint8_t)0x04)     // Lens = 4,    RW,  PMSC Control Register 1
#define DW1000_SUB_PMSC_SNOZT   ((uint8_t)0x0C)     // Lens = 1,    RW,  PMSC Snooze Time Register
#define DW1000_SUB_PMSC_TXFSEQ  ((uint8_t)0x26)     // Lens = 2,    RW,  PMSC fine grain TX sequencing control
#define DW1000_SUB_PMSC_LEDC    ((uint8_t)0x28)     // Lens = 4,    RW,  PMSC LED Control Register



/* System event Status */
/*masks */
#define SYS_STATUS_MASK_32      0xFFF7FFFFUL    /* System event Status Register access mask (all unused fields should always be writen as zero) */

/*offset 0 */
#define SYS_STATUS_IRQS         0x00000001UL    /* Interrupt Request Status READ ONLY */
#define SYS_STATUS_CPLOCK       0x00000002UL    /* Clock PLL Lock */
#define SYS_STATUS_ESYNCR       0x00000004UL    /* External Sync Clock Reset */
#define SYS_STATUS_AAT          0x00000008UL    /* Automatic Acknowledge Trigger */
#define SYS_STATUS_TXFRB        0x00000010UL    /* Transmit Frame Begins */
#define SYS_STATUS_TXPRS        0x00000020UL    /* Transmit Preamble Sent */
#define SYS_STATUS_TXPHS        0x00000040UL    /* Transmit PHY Header Sent */
#define SYS_STATUS_TXFRS        0x00000080UL    /* Transmit Frame Sent: This is set when the transmitter has completed the sending of a frame */

/*offset 8 */
#define SYS_STATUS_RXPRD        0x00000100UL    /* Receiver Preamble Detected status */
#define SYS_STATUS_RXSFDD       0x00000200UL    /* Receiver Start Frame Delimiter Detected. */
#define SYS_STATUS_LDEDONE      0x00000400UL    /* LDE processing done */
#define SYS_STATUS_RXPHD        0x00000800UL    /* Receiver PHY Header Detect */
#define SYS_STATUS_RXPHE        0x00001000UL    /* Receiver PHY Header Error */
#define SYS_STATUS_RXDFR        0x00002000UL    /* Receiver Data Frame Ready */
#define SYS_STATUS_RXFCG        0x00004000UL    /* Receiver FCS Good */
#define SYS_STATUS_RXFCE        0x00008000UL    /* Receiver FCS Error */





/* Analog RF Configuration */
#define RF_CONF_TXEN            0x00400000UL   /* TX enable */
#define RF_CONF_RXEN            0x00200000UL   /* RX enable */
#define RF_CONF_TXPOW           0x001F0000UL   /* turn on power all LDOs */
#define RF_CONF_PLLEN           0x0000E000UL   /* enable PLLs */
#define RF_CONF_TXBLOCKSEN      0x00001F00UL   /* enable TX blocks */
#define RF_CONF_TXPLLPOWEN      (RF_CONF_PLLEN | RF_CONF_TXPOW)
#define RF_CONF_TXALLEN         (RF_CONF_TXEN | RF_CONF_TXPOW | RF_CONF_PLLEN | RF_CONF_TXBLOCKSEN)

#define DWT_TX_IMMEDIATE        0
#define DWT_TX_DELAYED          1
#define DWT_RESPONSE            2

#define DWT_RX_IMMEDIATE        0
#define DWT_RX_DELAYED          1   // Set up delayed RX, if "late" error triggers, then the RX will be enabled immediately
#define DWT_IDLE_ON_DLY_ERR     2   // If delayed RX failed due to "late" error then if this
                                    // flag is set the RX will not be re-enabled immediately, and device will be in IDLE when function exits
#define DWT_NO_SYNC_PTRS        4   // Do not try to sync IC side and Host side buffer pointers when enabling RX. This is used to perform manual RX
                                    // re-enabling when receiving a frame in double buffer mode.
                                    
#define DW_PRF_16M  (0x01) // UWB PRF 16 MHz
#define DW_PRF_64M  (0x02) // UWB PRF 64 MHz
#define DW_CH2      (0x02)
           

/* System event Status */
/*masks */
#define SYS_STATUS_MASK_32      0xFFF7FFFFUL    /* System event Status Register access mask (all unused fields should always be writen as zero) */

/*offset 0 */
#define SYS_STATUS_IRQS         0x00000001UL    /* Interrupt Request Status READ ONLY */
#define SYS_STATUS_CPLOCK       0x00000002UL    /* Clock PLL Lock */
#define SYS_STATUS_ESYNCR       0x00000004UL    /* External Sync Clock Reset */
#define SYS_STATUS_AAT          0x00000008UL    /* Automatic Acknowledge Trigger */
#define SYS_STATUS_TXFRB        0x00000010UL    /* Transmit Frame Begins */
#define SYS_STATUS_TXPRS        0x00000020UL    /* Transmit Preamble Sent */
#define SYS_STATUS_TXPHS        0x00000040UL    /* Transmit PHY Header Sent */
#define SYS_STATUS_TXFRS        0x00000080UL    /* Transmit Frame Sent: This is set when the transmitter has completed the sending of a frame */

/*offset 8 */
#define SYS_STATUS_RXPRD        0x00000100UL    /* Receiver Preamble Detected status */
#define SYS_STATUS_RXSFDD       0x00000200UL    /* Receiver Start Frame Delimiter Detected. */
#define SYS_STATUS_LDEDONE      0x00000400UL    /* LDE processing done */
#define SYS_STATUS_RXPHD        0x00000800UL    /* Receiver PHY Header Detect */
#define SYS_STATUS_RXPHE        0x00001000UL    /* Receiver PHY Header Error */
#define SYS_STATUS_RXDFR        0x00002000UL    /* Receiver Data Frame Ready */
#define SYS_STATUS_RXFCG        0x00004000UL    /* Receiver FCS Good */
#define SYS_STATUS_RXFCE        0x00008000UL    /* Receiver FCS Error */

/*offset 16 */
#define SYS_STATUS_RXRFSL       0x00010000UL    /* Receiver Reed Solomon Frame Sync Loss */
#define SYS_STATUS_RXRFTO       0x00020000UL    /* Receive Frame Wait Timeout */
#define SYS_STATUS_LDEERR       0x00040000UL    /* Leading edge detection processing error */
#define SYS_STATUS_reserved     0x00080000UL    /* bit19 reserved */
#define SYS_STATUS_RXOVRR       0x00100000UL    /* Receiver Overrun */
#define SYS_STATUS_RXPTO        0x00200000UL    /* Preamble detection timeout */
#define SYS_STATUS_GPIOIRQ      0x00400000UL    /* GPIO interrupt */
#define SYS_STATUS_SLP2INIT     0x00800000UL    /* SLEEP to INIT */

/*offset 24 */
#define SYS_STATUS_RFPLL_LL     0x01000000UL    /* RF PLL Losing Lock */
#define SYS_STATUS_CLKPLL_LL    0x02000000UL    /* Clock PLL Losing Lock */
#define SYS_STATUS_RXSFDTO      0x04000000UL    /* Receive SFD timeout */
#define SYS_STATUS_HPDWARN      0x08000000UL    /* Half Period Delay Warning */
#define SYS_STATUS_TXBERR       0x10000000UL    /* Transmit Buffer Error */
#define SYS_STATUS_AFFREJ       0x20000000UL    /* Automatic Frame Filtering rejection */
#define SYS_STATUS_HSRBP        0x40000000UL    /* Host Side Receive Buffer Pointer */
#define SYS_STATUS_ICRBP        0x80000000UL    /* IC side Receive Buffer Pointer READ ONLY */

/*offset 32 */
#define SYS_STATUS_RXRSCS       0x0100000000ULL /* Receiver Reed-Solomon Correction Status */
#define SYS_STATUS_RXPREJ       0x0200000000ULL /* Receiver Preamble Rejection */
#define SYS_STATUS_TXPUTE       0x0400000000ULL /* Transmit power up time error */

#define SYS_STATUS_TXERR        (0x0408)        /* These bits are the 16 high bits of status register TXPUTE and HPDWARN flags */

/* All RX events after a correct packet reception mask. */
#define SYS_STATUS_ALL_RX_GOOD  (SYS_STATUS_RXDFR  | SYS_STATUS_RXFCG | SYS_STATUS_RXPRD | \
                                 SYS_STATUS_RXSFDD | SYS_STATUS_RXPHD | SYS_STATUS_LDEDONE)

/* All double buffer events mask. */
#define SYS_STATUS_ALL_DBLBUFF  (SYS_STATUS_RXDFR | SYS_STATUS_RXFCG)

/* All RX errors mask. */
#define SYS_STATUS_ALL_RX_ERR   (SYS_STATUS_RXPHE   | SYS_STATUS_RXFCE  | SYS_STATUS_RXRFSL | \
                                 SYS_STATUS_RXSFDTO | SYS_STATUS_AFFREJ | SYS_STATUS_LDEERR)

/* User defined RX timeouts (frame wait timeout and preamble detect timeout) mask. */
#define SYS_STATUS_ALL_RX_TO    (SYS_STATUS_RXRFTO | SYS_STATUS_RXPTO)

/* All TX events mask. */
#define SYS_STATUS_ALL_TX       (SYS_STATUS_AAT | SYS_STATUS_TXFRB | SYS_STATUS_TXPRS | \
                                 SYS_STATUS_TXPHS | SYS_STATUS_TXFRS )

/* Crystal frequency, in hertz. */
#define DWT_XTAL_FREQ_HZ        38400000

#define DWT_TIME_UNITS          (1.0/499.2e6/128.0)   // 15.65e-12 s

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 ms and 1 ms = 499.2 * 128 dtu. */
#define DWT_UUS_TO_DWT_TIME     65536

/* DW1000 SLEEP and WAKEUP configuration parameters */
#define DWT_PRESRV_SLP          0x0100    // PRES_SLP - on wakeup preserve sleep bit
#define DWT_LOADOPSET           0x0080    // ONW_L64P - on wakeup load operating parameter set for 64 PSR
#define DWT_CONFIG              0x0040    // ONW_LDC  - on wakeup restore (load) the saved configurations (from AON array into HIF)
#define DWT_RX_EN               0x0002    // ONW_RX   - on wakeup activate reception
#define DWT_TANDV               0x0001    // ONW_RADC - on wakeup run ADC to sample temperature and voltage sensor values

#define DWT_XTAL_EN             0x10    // keep XTAL running during sleep
#define DWT_WAKE_SLPCNT         0x08    // wake up after sleep count
#define DWT_WAKE_CS             0x04    // wake up on chip select
#define DWT_WAKE_WK             0x02    // wake up on WAKEUP PIN
#define DWT_SLP_EN              0x01    // enable sleep/deep sleep functionality

/* DW1000 interrupt events */
#define DWT_INT_TFRS            0x00000080    // frame sent
#define DWT_INT_LDED            0x00000400    // micro-code has finished execution
#define DWT_INT_RFCG            0x00004000    // frame received with good CRC
#define DWT_INT_RPHE            0x00001000    // receiver PHY header error
#define DWT_INT_RFCE            0x00008000    // receiver CRC error
#define DWT_INT_RFSL            0x00010000    // receiver sync loss error
#define DWT_INT_RFTO            0x00020000    // frame wait timeout
#define DWT_INT_RXOVRR          0x00100000    // receiver overrun
#define DWT_INT_RXPTO           0x00200000    // preamble detect timeout
#define DWT_INT_SFDT            0x04000000    // SFD timeout
#define DWT_INT_ARFE            0x20000000    // frame rejected (due to frame filtering configuration)

/* Analog RF Configuration */
#define RF_CONF_TXEN            0x00400000UL   /* TX enable */
#define RF_CONF_RXEN            0x00200000UL   /* RX enable */
#define RF_CONF_TXPOW           0x001F0000UL   /* turn on power all LDOs */
#define RF_CONF_PLLEN           0x0000E000UL   /* enable PLLs */
#define RF_CONF_TXBLOCKSEN      0x00001F00UL   /* enable TX blocks */
#define RF_CONF_TXPLLPOWEN      (RF_CONF_PLLEN | RF_CONF_TXPOW)
#define RF_CONF_TXALLEN         (RF_CONF_TXEN | RF_CONF_TXPOW | RF_CONF_PLLEN | RF_CONF_TXBLOCKSEN)

#define DWT_TX_IMMEDIATE        0
#define DWT_TX_DELAYED          1
#define DWT_RESPONSE            2

#define DWT_RX_IMMEDIATE        0
#define DWT_RX_DELAYED          1   // Set up delayed RX, if "late" error triggers, then the RX will be enabled immediately
#define DWT_IDLE_ON_DLY_ERR     2   // If delayed RX failed due to "late" error then if this
                                    // flag is set the RX will not be re-enabled immediately, and device will be in IDLE when function exits
#define DWT_NO_SYNC_PTRS        4   // Do not try to sync IC side and Host side buffer pointers when enabling RX. This is used to perform manual RX
                                    // re-enabling when receiving a frame in double buffer mode.


typedef struct
{
    struct                  rt_device parent;
    struct rt_spi_device    *spi_dev;
    uint32_t                REG_SYS_CFG;
    uint32_t                REG_TX_FCTRL;
    uint32_t                REG_PMSC_CTRL0;
    uint32_t                REG_GPIO_DOUT;
    uint8_t                 eui[8];
    uint8_t                 irq_pin;
    uint8_t                 rst_pin;
    uint8_t                 data_rate;
    uint8_t                 wait4resp;
}rt_dw1000_t;

static rt_dw1000_t rt_dw1000;
static void dw1000_irq_handler(void *args);

static uint8_t dw1000_read_data(uint32_t addr, uint32_t sub_idx, void *buf, uint32_t len)
{
    uint8_t send_buf[3];
    uint32_t cnt = 0;
    
    if(sub_idx == 0)
    {
        send_buf[cnt++] = addr;
    }
    else
    {
        send_buf[cnt++] = 0x40 | addr;
        if(sub_idx < 0xF0) /* 7-bit, subIndex <= 0x7F */
        {
             send_buf[cnt++] = (uint8_t)sub_idx;
        }
        else    /* 15-bit, subIndex <= 0x7FFF, extended address */
        {
            send_buf[cnt++] = 0x80 | (uint8_t)sub_idx;
            send_buf[cnt++] = (uint8_t)(sub_idx >> 7);
        }
    }
    
    rt_spi_send_then_recv(rt_dw1000.spi_dev, send_buf, cnt, buf, len);
    return RT_EOK;
}


static uint8_t dw1000_write_data(uint32_t addr, uint32_t sub_idx, void *buf, uint32_t len)
{
    uint8_t send_buf[3];
    uint32_t cnt = 0;
    
    if(sub_idx == 0)
    {
        send_buf[cnt++] = 0x80 | addr;
    }
    else
    {
        send_buf[cnt++] = 0xC0 | addr;
        if(sub_idx < 0xF0) /* 7-bit, subIndex <= 0x7F */
        {
             send_buf[cnt++] = (uint8_t)sub_idx;
        }
        else    /* 15-bit, subIndex <= 0x7FFF, extended address */
        {
            send_buf[cnt++] = 0x80 | (uint8_t)sub_idx;
            send_buf[cnt++] = (uint8_t)(sub_idx >> 7);
        }
    }
    
    rt_spi_send_then_send(rt_dw1000.spi_dev, send_buf, cnt, buf, len);
    return RT_EOK;
}


static uint32_t dw1000_read32(uint32_t addr, uint32_t sub_idx)
{
    uint32_t val;
    dw1000_read_data(addr, sub_idx, &val, 4);
    return val;
}
    
static uint32_t dw1000_write32(uint32_t addr, uint32_t sub_idx, uint32_t val)
{
    dw1000_write_data(addr, sub_idx, &val, 4);
    return val;
}

static uint8_t dw1000_read8(uint32_t addr, uint32_t sub_idx)
{
    uint8_t val;
    dw1000_read_data(addr, sub_idx, &val, 1);
    return val;
}

static uint32_t dw1000_write8(uint32_t addr, uint32_t sub_idx, uint8_t val)
{
    dw1000_write_data(addr, sub_idx, &val, 1);
    return val;
}

/* 125M */
uint32_t dw1000_get_sys_timestamp(void)
{
    uint32_t val;
    dw1000_read_data(DW1000_SYS_TIME, 1, &val, 4);
    return val>>1;
}

static void _write_tx(uint16_t ofs, uint8_t *buf, uint32_t len)
{
    /* Write the data to the IC TX buffer, (-2 bytes for auto generated CRC) */
    dw1000_write_data(DW1000_TX_BUFFER, ofs, buf, len-2);
}


static void _write_tx_fctrl(uint16_t len, uint16_t ofs, uint8_t ranging)
{
    uint32_t val = rt_dw1000.REG_TX_FCTRL | len | (ofs << 22) | (ranging << 15);
    dw1000_write32(DW1000_TX_FCTRL, 0, val);
}


uint32_t dwt_tx_timestamp_l32(void)
{
    // uint8_t buf[4];
    /* Read TX TIME as a 32-bit register to get the 4 lower bytes out of 5 */
    //  dw1000_read_data(DW1000_TX_TIME, 0, buf, 4);
    //    DW_TRACE("%X %X %X %X\r\n", buf[3], buf[2], buf[1], buf[0]);
    //    dw1000_read_data(DW1000_TX_TIME, 4, buf, 4);
    //    DW_TRACE("%X %X %X %X\r\n", buf[3], buf[2], buf[1], buf[0]);
    return dw1000_read32(DW1000_TX_TIME, 0);
}

uint32_t dwt_rx_timestamp_l32(void)
{
    return dw1000_read32(DW1000_RX_TIME, 0);
}


/*! ------------------------------------------------------------------------------------------
 * @fn     DWT_StartTx()
 * @brief  This call initiates the transmission, input parameter indicates which TX mode is used see below
 * @param  mode:
 *          - DWT_TX_IMMEDIATE                  0 immediate TX (no response expected)
 *          - DWT_TX_DELAYED                    1 delayed TX   (no response expected)
 *          - DWT_TX_IMMEDIATE | DWT_RESPONSE   2 immediate TX (response expected - so the receiver will be automatically turned on after TX is done)
 *          - DWT_TX_DELAYED   | DWT_RESPONSE   3 delayed TX   (response expected - so the receiver will be automatically turned on after TX is done)
 * @retval HAL_OK for success, or HAL_ERROR for error
 *         (e.g. a delayed transmission will fail if the delayed time has passed)
 */
uint32_t _start_tx(uint8_t mode)
{
    uint32_t ret = 0;
    uint8_t tmp8 = 0x00;
    uint16_t checkTxOK = 0;

    if (mode & DWT_RESPONSE)
    {
        tmp8 = 0x80 ;   /* Set wait4response bit */
        dw1000_write8(DW1000_SYS_CTRL, 0x00, tmp8);
        rt_dw1000.wait4resp = 1;
    }

    if (mode & DWT_TX_DELAYED)
    {
            /* Both SYS_CTRL_TXSTRT and SYS_CTRL_TXDLYS to correctly enable TX */
            tmp8 |= (0x06);
            dw1000_write8(DW1000_SYS_CTRL, 0x00, tmp8);
            dw1000_read_data(DW1000_SYS_STATUS, 3, &checkTxOK, 2); /* Read at offset 3 to get the upper 2 bytes out of 5 */

            /* Transmit Delayed Send set over Half a Period away or Power Up error (there is enough time to send but not to power up individual blocks). */
            if ((checkTxOK & SYS_STATUS_TXERR) == 0)
            {
                ret = RT_EOK;
            }
            else
            {
                // I am taking DSHP set to Indicate that the TXDLYS was set too late for the specified DX_TIME.
                // Remedial Action - (a) cancel delayed send
                // This assumes the bit is in the lowest byte
                dw1000_write8(DW1000_SYS_CTRL, 0x00, 0x40);
                // Note event Delayed TX Time too Late
                // Could fall through to start a normal send (below) just sending late.....
                // ... instead return and assume return value of 1 will be used to detect and recover from the issue.
                rt_dw1000.wait4resp = 0;
                ret = RT_EIO ; // Failed !
            }
    }
    else
    {
        tmp8 |= 0x02;
        dw1000_write8(DW1000_SYS_CTRL, 0x00, tmp8);
    }

  return ret;
}



static uint32_t _get_sys_state(void)
{
    uint32_t val;
    val = dw1000_read32(DW1000_SYS_STATE, 0);
    
    DW_TRACE("TX_STATE:%d    ", val & 0x0F);
    DW_TRACE("RX_STATE:%d    ", (val>>8) & 0x0F);
    DW_TRACE("PMSC_STATE:%d\r\n", (val>>16) & 0x0F);
    
    return val;
}

/* write dw1000 gpio pin (gpio_mode must be configred to GPIO function) */
static void _gpio_pin_write(uint8_t pin, uint8_t val)
{
    if(pin == 4)
    {
        rt_dw1000.REG_GPIO_DOUT |= (1<<12);
        (val == 0)?(rt_dw1000.REG_GPIO_DOUT &= ~(1<<8)):(rt_dw1000.REG_GPIO_DOUT |= (1<<8));
        dw1000_write32(DW1000_GPIO_CTRL, DW1000_SUB_GPIO_DOUT, rt_dw1000.REG_GPIO_DOUT);
    }
}

static rt_err_t rt_dw1000_init(rt_device_t dev)
{    
    return RT_EOK;
}


static rt_err_t rt_dw1000_open(rt_device_t dev, rt_uint16_t oflag)
{
    uint32_t val;
    uint16_t tmp;
    
    /* hardware reset */
    rt_pin_mode(rt_dw1000.rst_pin, PIN_MODE_OUTPUT);
    
    rt_pin_write(rt_dw1000.rst_pin, 0);
    rt_thread_delay(1);
    rt_pin_write(rt_dw1000.rst_pin, 1);
    rt_thread_delay(10);
    
    val = dw1000_read32(DW1000_DEV_ID, 0);
    if(val != DW1000_DEVICE_ID)
    {
        DW_TRACE("%s id failed!\r\n", __FUNCTION__);
    }
    else
    {
        DW_TRACE("dw1000 read id ok\r\n");
    }
        
//    dw1000_read_data(DW1000_EUI, 0, rt_dw1000.eui, 8);
//    for(i=0; i<8; i++)
//    {
//        DW_TRACE("%02X", rt_dw1000.eui[i]);
//    }
//    DW_TRACE("\r\n");
    
    
    /* load register to cache */
    rt_dw1000.REG_SYS_CFG = dw1000_read32(DW1000_SYS_CFG, 0);
    rt_dw1000.REG_TX_FCTRL =    dw1000_read32(DW1000_TX_FCTRL, 0);
    rt_dw1000.REG_PMSC_CTRL0 =  dw1000_read32(DW1000_PMSC, DW1000_SUB_PMSC_CTRL0);
    rt_dw1000.REG_GPIO_DOUT =   dw1000_read32(DW1000_GPIO_CTRL, DW1000_SUB_GPIO_DOUT);
    
    /* enable GPIO clock and kiloHz clock */
    rt_dw1000.REG_PMSC_CTRL0 |= (1<<16) | (1<<17) | (1<<18) | (1<<19) | (1<<23);
    dw1000_write32(DW1000_PMSC, DW1000_SUB_PMSC_CTRL0, rt_dw1000.REG_PMSC_CTRL0);
    
    /* disable fine grain sequencing - this is needed when using PA on the TX */
    tmp = 0x0000;
    dw1000_write_data(DW1000_PMSC, DW1000_SUB_PMSC_TXFSEQ, &tmp, 2);
    
    /* set all GPIO to be hardware functional GPIO */
    val = dw1000_read32(DW1000_GPIO_CTRL, DW1000_SUB_GPIO_MODE);
    dw1000_write32(DW1000_GPIO_CTRL, DW1000_SUB_GPIO_MODE, val | (1<<6) | (1<<8) | (1<<10) | (1<<12) | (1<<16) | (1<<18) | (0<<20) | (0<<22));
    
    /* enable LED blink function and set blink time to short */
    val = dw1000_read32(DW1000_PMSC, DW1000_SUB_PMSC_LEDC);
    val &= ~0xFF;
    val |= (1<<8) | (1<<1);
    dw1000_write32(DW1000_PMSC, DW1000_SUB_PMSC_LEDC, val);
    
    /* set GPIO4 to be output */
    val = dw1000_read32(DW1000_GPIO_CTRL, DW1000_SUB_GPIO_DIR);
    val &= ~(1<<8);
    val |= (1<<12);
    dw1000_write32(DW1000_GPIO_CTRL, DW1000_SUB_GPIO_DIR, val);
    
    /* enable TX interrtups, all RX event */
    dw1000_write32(DW1000_SYS_MASK, 0, (1<<7) | (1<<8) | (1<<9) | (1<<11));

//    /* channel control */
//    val = (0x0000000FUL & (DW_CH2)) |               // Transmit Channel
//            (0x000000F0UL & (DW_CH2 << 4))  |       // Receive Channel
//            (0x00020000UL & (0 << 17)) |            // Use DW nsSFD
//            (0x000C0000UL & (DW_PRF_64M << 18)) |   // RX PRF
//            (0x07C00000UL & (9 << 22)) |            // TX Preamble Code
//            (0xF8000000UL & (9 << 27));             // RX Preamble Code
//     dw1000_write32(DW1000_CHAN_CTRL, 0x00, val);
  

    DW_TRACE("%-32s:0x%X\r\n",  "DW1000_SYS_CFG",               rt_dw1000.REG_SYS_CFG);
    DW_TRACE("%-32s:0x%X\r\n",  "DW1000_TX_FCTRL",              rt_dw1000.REG_TX_FCTRL);
    DW_TRACE("%-32s:0x%X\r\n",  "DW1000_SUB_PMSC_CTRL0",        dw1000_read32(DW1000_PMSC, DW1000_SUB_PMSC_CTRL0));
    DW_TRACE("%-32s:0x%X\r\n",  "DW1000_SUB_PMSC_CTRL1",        dw1000_read32(DW1000_PMSC, DW1000_SUB_PMSC_CTRL1));
    DW_TRACE("%-32s:0x%X\r\n",  "DW1000_SUB_PMSC_SNOZT",        dw1000_read32(DW1000_PMSC, DW1000_SUB_PMSC_SNOZT));
    DW_TRACE("%-32s:0x%X\r\n",  "DW1000_SUB_PMSC_TXFSEQ",       dw1000_read32(DW1000_PMSC, DW1000_SUB_PMSC_TXFSEQ));
    DW_TRACE("%-32s:0x%X\r\n",  "DW1000_SYS_MASK",              dw1000_read32(DW1000_SYS_MASK, 0));
    DW_TRACE("%-32s:0x%X\r\n",  "DW1000_CHAN_CTRL",             dw1000_read32(DW1000_CHAN_CTRL, 0));
    
    
    /* install irq */
    rt_pin_mode(rt_dw1000.irq_pin, PIN_MODE_INPUT);
    rt_pin_attach_irq(rt_dw1000.irq_pin, PIN_IRQ_MODE_RISING, dw1000_irq_handler, RT_NULL);
    rt_pin_irq_enable(rt_dw1000.irq_pin, PIN_IRQ_ENABLE);
    return RT_EOK;
}


static rt_size_t rt_dw1000_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    return size;
}

static rt_size_t rt_dw1000_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    uint32_t val;
    dw1000_write32(DW1000_SYS_STATUS, 0, SYS_STATUS_TXFRS);
        
    uint8_t buf[32];
    uint8_t len = sizeof(buf);
    
    buf[0] = 0;
    buf[1] = 1;
    buf[2] = 2;
    buf[3] = 3;
    buf[4] = 4;
    _write_tx(0, buf, len); 
    _write_tx_fctrl(len, 0, 1);
    _start_tx(DWT_RESPONSE);
    
    rt_thread_delay(1);
    _get_sys_state();
    
    
    DW_TRACE("%-32s:0x%X\r\n", "DW1000_SYS_STATUS:", dw1000_read32(DW1000_SYS_STATUS, 0));
    
    val = dwt_tx_timestamp_l32();
    DW_TRACE("%-32s:%d\r\n", "dwt_read_tx_timestamp_l32:", val);
    
    val = dwt_rx_timestamp_l32();
    DW_TRACE("%-32s:%d\r\n", "dwt_read_rx_timestamp_l32:", val);
//    static uint32_t last_sys_time = 0;
//    uint32_t sys_time = 0;
//    
//    sys_time = dw1000_get_sys_timestamp();
//    
//    printf("%d\r\n", (sys_time) - last_sys_time);
//    
//    last_sys_time = sys_time;
    
    return size;
}

static rt_err_t rt_dw1000_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}


int rt_hw_dw1000_init(const char *name, const char *spid_name, uint32_t rst_pin, uint32_t irq_pin)
{
    rt_dw1000.spi_dev =  (struct rt_spi_device *)rt_device_find(spid_name);
    
    struct rt_device *dev;
    
    dev                     = &rt_dw1000.parent;
	dev->type               = RT_Device_Class_Miscellaneous;
	dev->rx_indicate        = RT_NULL;
	dev->tx_complete        = RT_NULL;
	dev->init               = rt_dw1000_init;
	dev->open               = rt_dw1000_open;
	dev->close              = RT_NULL;
	dev->read               = rt_dw1000_read;
	dev->write              = rt_dw1000_write;
	dev->control            = rt_dw1000_control;
	dev->user_data          = &rt_dw1000;
 
    /* install irq pins */
    rt_dw1000.irq_pin = irq_pin;
    rt_dw1000.rst_pin = rst_pin;
    
    rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


static void dw1000_irq_handler(void *args)
{
    DW_TRACE("%s\r\n", __FUNCTION__);
}
