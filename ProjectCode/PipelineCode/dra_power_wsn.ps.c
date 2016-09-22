/* dra_power.ps.c */                                                       
/* Default received power model for radio link Transceiver	*/
/* Pipeline. This model uses the receiver channel state		*/
/* information to check and update the signal lock status	*/
/* of the channel. It relies on the rxgroup stage model for	*/
/* the creation and initialization of the channel state		*/
/* information.												*/

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


/***** constants *****/

#define C					3.0E+08								/* speed of light (m/s) */
#define SIXTEEN_PI_SQ		(16.0 * VOSC_NA_PI * VOSC_NA_PI)	/* 16 times pi-squared */


/***** pipeline procedure *****/

#if defined (__cplusplus)
extern "C"
#endif

void
dra_power_wsn_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr)
	{
	double				   	prop_distance, rcvd_power, path_loss;
	double				   	tx_power, tx_base_freq, tx_bandwidth, tx_center_freq;
	double				   	lambda, rx_ant_gain, tx_ant_gain;
	Objid				   	rx_ch_obid;
	double				   	in_band_tx_power, band_max, band_min;
	double				   	rx_base_freq, rx_bandwidth;
	DraT_Rxch_State_Info*	rxch_state_ptr;

//self-defined variables begin
	int						chanmatch_status;
	double					ht,hr;
	double					rcvd_power_threshold, path_loss_threshold;
	double					rcvd_power_cs,path_loss_if; //interference related
	Objid					tx_objid;
	Boolean					debug = OPC_FALSE;
	double					tx_range = 250.0;//transmission range, unit:m
	double					cs_range = 500.0;//carrier sensing range, unit:m
	int						L=1;
	char					line0_buf [512];
	int						pk_id;//packet ID
	Boolean					lock_flag=OPC_FALSE;
//	char				line1_buf [512];
//self-defined variables end

	/** Compute the average power in Watts of the		**/
	/** signal associated with a transmitted packet.	**/
	FIN_MT (dra_power_wsn (pkptr));

//self-defined codes begin
	pk_id=op_pk_id(pkptr);
	//added by Fei: obtain the id of the transmitter
	tx_objid = op_td_get_int(pkptr, OPC_TDA_RA_TX_OBJID);
	if(op_ima_obj_attr_exists(tx_objid,"debug"))
	{
		op_ima_obj_attr_get(tx_objid,"debug",&debug);
	}
//self-defined codes end
	
	/* If the incoming packet is 'valid', it may cause the receiver to	*/
	/* lock onto it. However, if the receiving node is disabled, then	*/
	/* the channel match should be set to noise.						*/
	if (op_td_get_int (pkptr, OPC_TDA_RA_MATCH_STATUS) == OPC_TDA_RA_MATCH_VALID)
		{
		if (op_td_is_set (pkptr, OPC_TDA_RA_ND_FAIL))
			{
			/* The receiving node is disabled.  Change	*/
			/* the channel match status to noise.		*/
			op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_NOISE);
			}
		else
			{
			/* The receiving node is enabled.  Get	*/
			/* the address of the receiver channel.	*/
			rx_ch_obid = op_td_get_int (pkptr, OPC_TDA_RA_RX_CH_OBJID);
	
			/* Access receiver channels state information.		*/
			rxch_state_ptr = (DraT_Rxch_State_Info *) op_ima_obj_state_get (rx_ch_obid);
			
			/* If the receiver channel is already locked,		*/
			/* the packet will now be considered to be noise.	*/
			/* This prevents simultaneous reception of multiple	*/
			/* valid packets on any given radio channel.		*/
			if (rxch_state_ptr->signal_lock){				
				op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_NOISE);
				
//self-defined codes begin
				lock_flag = OPC_FALSE;
				if(debug){
						sprintf(line0_buf,"in 7.0 power stage:\n\
							packet is set as noise. \n");
						op_sim_message(line0_buf,"");
					}
//self-defined codes end
				
				}
			else
				{
				/* Otherwise, the receiver channel will become	*/
				/* locked until the packet reception ends.		*/
				lock_flag = OPC_TRUE;
				rxch_state_ptr->signal_lock = OPC_TRUE;
				}
			}
		}

	/* Get power allotted to transmitter channel. */
	tx_power = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_POWER);

	/* Get transmission frequency in Hz. */
	tx_base_freq = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_FREQ);
	tx_bandwidth = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_BW);
	tx_center_freq = tx_base_freq + (tx_bandwidth / 2.0);

	/* Caclculate wavelength (in meters). */
	lambda = C / tx_center_freq;

	/* Get distance between transmitter and receiver (in meters). */
	prop_distance = op_td_get_dbl (pkptr, OPC_TDA_RA_START_DIST);

	/* When using TMM, the TDA OPC_TDA_RA_RCVD_POWER will already	*/
	/* have a raw value for the path loss. */
	if (op_td_is_set (pkptr, OPC_TDA_RA_RCVD_POWER))
		{
		path_loss = op_td_get_dbl (pkptr, OPC_TDA_RA_RCVD_POWER);
		}
	else
		{
		/* Compute the path loss for this distance and wavelength. */
		if (prop_distance > 0.0)
			{
			path_loss = (lambda * lambda) / (SIXTEEN_PI_SQ * prop_distance * prop_distance);
//self-defined codes begin
			if(debug)
			{
				sprintf(line0_buf,"in 7.1. power stage:\n\
								pk_id=%d,\n\
								lambda*lambda=%f,\n\
								SIXTEEN_PI_SQ=%f,\n\
								path_loss=%f.\n",pk_id,lambda*lambda,SIXTEEN_PI_SQ,path_loss);
				op_sim_message(line0_buf,"");
			}
//self-defined codes end
			}
		else
			path_loss = 1.0;
		}
	
//self-defined codes begin
	/* Compute the in-band transmission power by multiplying	*/
	/* the transmission power with the overlap ratio between	*/
	/* the frequency bands of the transmitter and receiver.		*/
	chanmatch_status = op_td_get_int (pkptr, OPC_TDA_RA_MATCH_STATUS);
	if (chanmatch_status == OPC_TDA_RA_MATCH_VALID)
		{
		/* Since the packet status is not set to noise, there	*/
		/* must be a exact match between the bands (i.e.		*/
		/* overlap ratio = 1.0).								*/
		in_band_tx_power = tx_power;
		}
	else
		{
//self-defined codes end
		
	/* Determine the receiver bandwidth and base frequency. */
	rx_base_freq = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_FREQ);
	rx_bandwidth = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_BW);

	/* Use these values to determine the band overlap with the transmitter.	*/
	/* Note that if there were no overlap at all, the packet would already	*/
	/* have been filtered by the channel match stage.						*/

	/* The base of the overlap band is the highest base frequency. */
	if (rx_base_freq > tx_base_freq) 
		band_min = rx_base_freq;
	else
		band_min = tx_base_freq;
	
	/* The top of the overlap band is the lowest end frequency. */
	if (rx_base_freq + rx_bandwidth > tx_base_freq + tx_bandwidth)
		band_max = tx_base_freq + tx_bandwidth;
	else
		band_max = rx_base_freq + rx_bandwidth;

	/* Compute the amount of in-band transmitter power. */
	in_band_tx_power = tx_power * (band_max - band_min) / tx_bandwidth;
	
//self-defined codes begin
	}
//self-defined codes end
	
	/* Get antenna gains (raw form, not in dB). */
	tx_ant_gain = pow (10.0, op_td_get_dbl (pkptr, OPC_TDA_RA_TX_GAIN) / 10.0);
	rx_ant_gain = pow (10.0, op_td_get_dbl (pkptr, OPC_TDA_RA_RX_GAIN) / 10.0);

//modified
	/* Calculate received power level. */
	ht=1.5;hr=1.5;
	path_loss=ht*ht*hr*hr/(prop_distance*prop_distance*prop_distance*prop_distance*L);
	rcvd_power = in_band_tx_power * tx_ant_gain * path_loss * rx_ant_gain;
	
	
	if(op_ima_obj_attr_exists(tx_objid,"tx range")) //determine the existings of "tx range" attribute
	{
		op_ima_obj_attr_get(tx_objid,"tx range",&tx_range);
	}
	
	
	path_loss_threshold = ht*ht*hr*hr/(tx_range * tx_range * tx_range * tx_range * L);
	rcvd_power_threshold = in_band_tx_power * tx_ant_gain * path_loss_threshold * rx_ant_gain;
	op_td_set_dbl (pkptr, OPC_TDA_RA_MAX_INDEX+1, rcvd_power_threshold);

//self-defined codes begin

	/* if the packet is from a transmitter which is out of        */
	/* the interference range of the current receiving node,      */
	/* then set the packet to be invalid, and unlock the channel. */
	if(op_ima_obj_attr_exists(tx_objid,"cs range")) //determine the existings of "tx range" attribute
	{
		op_ima_obj_attr_get(tx_objid,"cs range",&cs_range);
	}
	path_loss_if = ht*ht*hr*hr/(cs_range * cs_range * cs_range * cs_range * L);
	rcvd_power_cs = in_band_tx_power * tx_ant_gain * path_loss_if * rx_ant_gain;
	
	//self-defined codes begin
			if(debug)
			{
				sprintf(line0_buf,"in 7.2. power stage:\n\
								rcvd_power_cs=%E,\n\
								rcvd_power_threshold = %E,\n\
								cs_range=%f.\n",rcvd_power_cs,rcvd_power_threshold,cs_range);
				op_sim_message(line0_buf,"");
			}
	//self-defined codes end
	
	/* if the received power is smaller than smallest interference received power, */
	/* set the received power to zero. That means the interference is ignored. */
	if(rcvd_power<rcvd_power_cs){
		//rcvd_power = 0.0;
		op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_IGNORE);
		
		if(lock_flag)//the interference can be ignored, unlock the channel
		{
			rxch_state_ptr->signal_lock = OPC_FALSE;
		}
	}
		
	if(debug){
		sprintf(line0_buf,"in 7.2. power stage:\n\
			pk_id=%d,\n\
			tx_base_freq=%f, tx_bandwidth=%f, tx_center_freq=%f,\n\
			in_band_tx_power=%f,\n\
			tx_ant_gain=%f, rx_ant_gain=%f.\n\
			path_loss=%E,\n\
			prop_distance=%f,rcvd_power=%E. tx_range=%f.\n",pk_id,tx_base_freq,tx_bandwidth,tx_center_freq,in_band_tx_power,\
			tx_ant_gain,rx_ant_gain,path_loss,prop_distance,rcvd_power,tx_range);
		op_sim_message(line0_buf,"");
	}
//self-defined codes end
	
	/* Assign the received power level (in Watts) */
	/* to the packet transmission data attribute. */
	op_td_set_dbl (pkptr, OPC_TDA_RA_RCVD_POWER, rcvd_power);

	FOUT
	}
