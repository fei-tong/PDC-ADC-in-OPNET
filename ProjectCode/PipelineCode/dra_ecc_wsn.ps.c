/* dra_ecc.ps.c */
/* Default error correction model for radio link			*/
/* Transceiver Pipeline. This model uses the receiver		*/
/* channel state information to update the signal lock		*/
/* status of the channel. It relies on the rxgroup stage	*/
/* model for the creation and initialization of the channel	*/
/* state information.										*/

/****************************************/
/*		 Copyright (c) 1993-2008		*/
/*		by OPNET Technologies, Inc.		*/
/*		 (A Delaware Corporation)		*/
/*	   7255 Woodmont Av., Suite 250  	*/
/*      Bethesda, MD 20814, U.S.A.      */
/*		   All Rights Reserved.			*/
/****************************************/

#include "opnet.h"
#include "dra.h"
#include <math.h>

#if defined (__cplusplus)
extern "C"
#endif
	
void
dra_ecc_wsn_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr)
    {
	int					   	num_errs, accept;
	OpT_Packet_Size		   	pklen;
	double				   	ecc_thresh;
	DraT_Rxch_State_Info*	rxch_state_ptr;
	
//self-defined variables begin
	Boolean                 transceiver_state;
	Objid					rx_ch_objid;
	Objid					tx_objid;
	Boolean					debug = OPC_FALSE;
	
	double					bkg_noise,rcvd_snr,rcvd_power_threshold,rcvd_snr_threshold;
	char 					line_buf[512];
//self-defined variables end
	
	/** Determine acceptability of given packet at receiver. **/
	FIN_MT (dra_ecc_wsn (pkptr));

	/* Do not accept packets that were received	*/
	/* when the node was disabled.				*/
	if (op_td_is_set (pkptr, OPC_TDA_RA_ND_FAIL))
		accept = OPC_FALSE;
	else
		{
//self-defined codes begin
	transceiver_state = OPC_TRUE;	
	//added by Fei: obtain the id of the transmitter
	tx_objid = op_td_get_int(pkptr, OPC_TDA_RA_TX_OBJID);
	if(op_ima_obj_attr_exists(tx_objid,"debug"))
	{
		op_ima_obj_attr_get(tx_objid,"debug",&debug);
	}
	
	//obtain the state of the radio receiver
	rx_ch_objid = op_td_get_int (pkptr, OPC_TDA_RA_RX_CH_OBJID);
	if(op_ima_obj_attr_exists(op_topo_parent (op_topo_parent (rx_ch_objid)),"state"))
	{	
		op_ima_obj_attr_get(op_topo_parent (op_topo_parent (rx_ch_objid)),"state",&transceiver_state);
	}
	if (transceiver_state == OPC_FALSE)
	{
		accept = OPC_FALSE;
	}
	else
	{
//self-defined codes end
		
		/* Obtain the error correction threshold of the receiver. */
		ecc_thresh = op_td_get_dbl (pkptr, OPC_TDA_RA_ECC_THRESH);

		/* Obtain length of packet. */
		pklen = op_pk_total_size_get (pkptr);

		/* Obtain number of errors in packet. */
		num_errs = op_td_get_int (pkptr, OPC_TDA_RA_NUM_ERRORS);
		
		if(debug){
			sprintf(line_buf,"13.1: in ecc 13.num_errs=%f.\n",num_errs);
			op_sim_message(line_buf,"");
		}

		/* Test if bit errors exceed threshold. */
		if (pklen == 0)
			accept = OPC_TRUE;
		else
			accept = ((((double) num_errs) / pklen) <= ecc_thresh) ? OPC_TRUE : OPC_FALSE;		
		
//self-defined codes begin
		bkg_noise = op_td_get_dbl (pkptr, OPC_TDA_RA_BKGNOISE);
		rcvd_snr = op_td_get_dbl (pkptr, OPC_TDA_RA_SNR);
		rcvd_power_threshold = op_td_get_dbl (pkptr, OPC_TDA_RA_MAX_INDEX+1);
		rcvd_snr_threshold = 10.0 * log10 (rcvd_power_threshold/bkg_noise);
		accept = (rcvd_snr >= rcvd_snr_threshold) ? OPC_TRUE : OPC_FALSE;
		if(debug){
			sprintf(line_buf,"13.2: in ecc 13. accept = %d. bkg_noise = %E, rcvd_snr_threshold=%E\n",accept,bkg_noise,rcvd_snr_threshold);
			op_sim_message(line_buf,"");
		}
		
	}
//self-defined codes end	
	
	}

	/* Place flag indicating accept/reject in transmission data block. */
	op_td_set_int (pkptr, OPC_TDA_RA_PK_ACCEPT, accept);

	/* In either case the receiver channel is no longer locked. */
	rxch_state_ptr = (DraT_Rxch_State_Info *) op_ima_obj_state_get (op_td_get_int (pkptr, OPC_TDA_RA_RX_CH_OBJID));
	rxch_state_ptr->signal_lock = OPC_FALSE;
	
	FOUT
	}
