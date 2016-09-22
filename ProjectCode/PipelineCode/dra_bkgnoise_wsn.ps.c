/* dra_bkgnoise.ps.c */                                                       
/* Default background noise model for radio link Transceiver Pipeline */

/****************************************/
/*		  Copyright (c) 1993-2008		*/
/*		by OPNET Technologies, Inc.		*/
/*		(A Delaware Corporation)		*/
/*	7255 Woodmont Av., Suite 250  		*/
/*     Bethesda, MD 20814, U.S.A.       */
/*			All Rights Reserved.		*/
/****************************************/

#include "opnet.h"


/***** constants *****/

#define BOLTZMANN			1.379E-23
#define BKG_TEMP			290.0
#define AMB_NOISE_LEVEL		1.0E-26


/***** procedure *****/

#if defined (__cplusplus)
extern "C"
#endif
void
dra_bkgnoise_wsn_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr)
	{
	double		rx_noisefig, rx_temp, rx_bw;
	double		bkg_temp, bkg_noise, amb_noise;
	
//自加变量开始
	Objid				tx_objid;
	Boolean				debug = OPC_FALSE;
	char				line0_buf [512];
	int					pk_id;
//	char				line1_buf [512];
//自加变量结束

	/** Compute noise sources other than transmission interference. **/
	FIN_MT (dra_bkgnoise_wsn (pkptr));

	/* Get receiver noise figure. */
	rx_noisefig = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_NOISEFIG);

	/* Calculate effective receiver temperature. */
	rx_temp = (rx_noisefig - 1.0) * 290.0;

	/* Set the effective background temperature. */
	bkg_temp = BKG_TEMP;

	/* Get receiver channel bandwidth (in Hz). */
	rx_bw = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_BW);

	/* Calculate in-band noise from both background and thermal sources. */
	bkg_noise = (rx_temp + bkg_temp) * rx_bw * BOLTZMANN;

	/* Calculate in-band ambient noise. */
	amb_noise = rx_bw * AMB_NOISE_LEVEL;

//自加代码开始
	pk_id=op_pk_id(pkptr);
	//added by Fei: obtain the id of the transmitter
	tx_objid = op_td_get_int(pkptr, OPC_TDA_RA_TX_OBJID);
	if(op_ima_obj_attr_exists(tx_objid,"debug"))
	{
		op_ima_obj_attr_get(tx_objid,"debug",&debug);
	}
	
	if(debug){
		sprintf(line0_buf,"in 9. pk_id=%d.\n\
				background noise stage:\n\
				bkg_noise=%E.\n",pk_id,(amb_noise + bkg_noise));
		op_sim_message (line0_buf, "");
	}
//自加代码结束
	
	/* Put the sum of both noise sources in the packet transmission data attr.*/
	op_td_set_dbl (pkptr, OPC_TDA_RA_BKGNOISE, (amb_noise + bkg_noise));

	FOUT
	}
