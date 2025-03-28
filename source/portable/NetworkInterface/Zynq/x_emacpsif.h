/*
 * Copyright (c) 2010-2013 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef __NETIF_XEMACPSIF_H__
    #define __NETIF_XEMACPSIF_H__

    #include <stdint.h>

    #include "xstatus.h"
    #include "xparameters.h"
    #include "xparameters_ps.h" /* defines XPAR values */
    #include "xil_types.h"
    #include "xil_assert.h"
    #include "xil_io.h"
    #include "xil_exception.h"
    #include "xpseudo_asm.h"
    #include "xil_cache.h"
    #include "xuartps.h"
    #include "xscugic.h"
    #include "xemacps.h" /* defines XEmacPs API */

    #ifdef __cplusplus
    extern "C" {
    #endif

    #ifdef SDT
        #define ZYNQ_SCUGIC_0_BASEADDR          XPAR_XSCUGIC_0_BASEADDR
        #define ZYNQ_ETHERNET_0_BASEADDR        XPAR_XEMACPS_0_BASEADDR
        #if ( XPAR_XEMACPS_NUM_INSTANCES > 1 )
            #define ZYNQ_ETHERNET_1_BASEADDR    XPAR_XEMACPS_1_BASEADDR
        #endif
    #else
        #ifndef XPAR_PS7_ETHERNET_1_DEVICE_ID
            #define XPAR_PS7_ETHERNET_1_DEVICE_ID    1
        #endif
        #ifndef XPAR_PS7_ETHERNET_1_BASEADDR
            #define XPAR_PS7_ETHERNET_1_BASEADDR     0xE000C000
        #endif
        #define ZYNQ_SCUGIC_0_BASEADDR               XPAR_PS7_SCUGIC_0_BASEADDR
        #define ZYNQ_ETHERNET_0_BASEADDR             XPAR_PS7_ETHERNET_0_BASEADDR
        #if ( XPAR_XEMACPS_NUM_INSTANCES > 1 )
            #define ZYNQ_ETHERNET_1_BASEADDR         XPAR_PS7_ETHERNET_1_BASEADDR
        #endif
    #endif /* ifdef SDT */

    extern XEmacPs_Config mac_configs[ XPAR_XEMACPS_NUM_INSTANCES ];


    void xemacpsif_setmac( uint32_t index,
                           uint8_t * addr );
    uint8_t * xemacpsif_getmac( uint32_t index );

    #ifdef NOTNOW_BHILL
        unsigned get_IEEE_phy_speed( XLlTemac * xlltemacp );
    #endif

/* xaxiemacif_hw.c */
    void xemacps_error_handler( XEmacPs * Temac );

    struct xBD_TYPE
    {
        uint32_t address;
        uint32_t flags;
    };

/*
 * Missing declaration in 'src/xemacps_hw.h' :
 * When set, the GEM DMA will automatically
 * discard receive packets from the receiver packet
 * buffer memory when no AHB resource is
 * available.
 * When low, then received packets will remain to be
 * stored in the SRAM based packet buffer until
 * AHB buffer resource next becomes available.
 */
    #define XEMACPS_DMACR_DISC_WHEN_NO_AHB_MASK    0x01000000

    #define EMAC_IF_RX_EVENT                       1
    #define EMAC_IF_TX_EVENT                       2
    #define EMAC_IF_ERR_EVENT                      4
    #define EMAC_IF_ALL_EVENT                      7

/* structure within each netif, encapsulating all information required for
 * using a particular temac instance
 */
    typedef struct
    {
        XEmacPs emacps;

        /* pointers to memory holding buffer descriptors (used only with SDMA) */
        struct xBD_TYPE * rxSegments;
        struct xBD_TYPE * txSegments;

        unsigned char * tx_space;
        unsigned uTxUnitSize;

        char * remain_mem;
        unsigned remain_siz;

        volatile int rxHead, rxTail;
        volatile int txHead, txTail;

        volatile int txBusy;

        volatile uint32_t isr_events;

        unsigned int last_rx_frms_cntr;
    } xemacpsif_s;

/*extern xemacpsif_s xemacpsif; */

    int is_tx_space_available( xemacpsif_s * emac );

/* xaxiemacif_dma.c */

    struct xNETWORK_BUFFER;

    int emacps_check_rx( xemacpsif_s * xemacpsif,
                         NetworkInterface_t * pxInterface );
    void emacps_check_tx( xemacpsif_s * xemacpsif );
    int emacps_check_errors( xemacpsif_s * xemacps );
    void emacps_set_rx_buffers( xemacpsif_s * xemacpsif,
                                u32 ulCount );

    extern XStatus emacps_send_message( xemacpsif_s * xemacpsif,
                                        struct xNETWORK_BUFFER * pxBuffer,
                                        int iReleaseAfterSend );
    extern unsigned Phy_Setup( XEmacPs * xemacpsp );
    extern void setup_isr( xemacpsif_s * xemacpsif );
    extern XStatus init_dma( xemacpsif_s * xemacpsif );
    extern void start_emacps( xemacpsif_s * xemacpsif );

    void EmacEnableIntr( int xEMACIndex );
    void EmacDisableIntr( int xEMACIndex );

    XStatus init_axi_dma( xemacpsif_s * xemacpsif );
    void process_sent_bds( xemacpsif_s * xemacpsif );

    void emacps_send_handler( void * arg );
    void emacps_recv_handler( void * arg );
    void emacps_error_handler( void * arg,
                               u8 Direction,
                               u32 ErrorWord );
    void HandleTxErrors( xemacpsif_s * xemacpsif );
    XEmacPs_Config * xemacps_lookup_config( unsigned mac_base );

    void clean_dma_txdescs( xemacpsif_s * xemacpsif );
    void resetrx_on_no_rxdata( xemacpsif_s * xemacpsif );

    BaseType_t get_xEMACIndex( const XEmacPs * xemacpsp );

/**
 * @brief Initialise the interface number 'xIndex'. Do not call directly.
 */
    void vInitialiseOnIndex( BaseType_t xIndex );

    #ifdef __cplusplus
}     /* extern "C" */
    #endif

#endif /* __NETIF_XAXIEMACIF_H__ */
