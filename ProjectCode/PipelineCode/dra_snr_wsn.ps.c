/* dra_snr.ps.c */                                                       
/* Default Signal-to-Noise-Ratio (SNR) model for radio link Transceiver Pipeline */

/****************************************/
/*		  Copyright (c) 1993-2008		*/
/*		by OPNET Technologies, Inc.		*/
/*		(A Delaware Corporation)		*/
/*	7255 Woodmont Av., Suite 250  		*/
/*     Bethesda, MD 20814, U.S.A.       */
/*			All Rights Reserved.		*/
/****************************************/

#include "opnet.h"
#include <math.h>


#if defined (__cplusplus)
extern "C"
#endif
void
dra_snr_wsn_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr)
{
	double		bkg_noise, accum_noise, rcvd_power;
	
//自加变量开始
	Objid				tx_objid;
	Boolean				debug = OPC_FALSE;
	int					pk_id;
	char				line0_buf [512];
//	char				line1_buf [512];
//自加变量结束

	/** Compute the signal-to-noise ratio for the given packet. **/
	FIN_MT (dra_snr_wsn (pkptr));

	/* Get the packet's received power level. */
	rcvd_power = op_td_get_dbl (pkptr, OPC_TDA_RA_RCVD_POWER);

	/* Get the packet's accumulated noise levels calculated by the */
	/* interference and background noise stages. */
	accum_noise = op_td_get_dbl (pkptr, OPC_TDA_RA_NOISE_ACCUM);
	bkg_noise = op_td_get_dbl (pkptr, OPC_TDA_RA_BKGNOISE);

//自加代码开始
	//added by Fei: obtain the id of the transmitter
	pk_id = op_pk_id(pkptr);
	tx_objid = op_td_get_int(pkptr, OPC_TDA_RA_TX_OBJID);
	if(op_ima_obj_attr_exists(tx_objid,"debug"))
	{
		op_ima_obj_attr_get(tx_objid,"debug",&debug);
	}
	
	if(debug){
		sprintf(line0_buf,"in 10. snr stage:\n\
	       	    pk_id=%d.\n\
				bkg_noise=%E,accum_noise=%E,sum=%E\n\
				rcvd_power=%E, \n\
				rcvd_power(dBm)=%E, snr=%E,\n",pk_id, bkg_noise, accum_noise,(bkg_noise+accum_noise),rcvd_power,\
				10.0*log10(rcvd_power*1000.0),10.0 * log10 (rcvd_power / (accum_noise + bkg_noise)));
		op_sim_message (line0_buf, "");
	}
//自加代码结束
	
	/* Assign the SNR in dB. */
	op_td_set_dbl (pkptr, OPC_TDA_RA_SNR, 
		10.0 * log10 (rcvd_power / (accum_noise + bkg_noise)));

	/* Set field indicating the time at which SNR was calculated. */
	op_td_set_dbl (pkptr, OPC_TDA_RA_SNR_CALC_TIME, op_sim_time ());

	FOUT
}
