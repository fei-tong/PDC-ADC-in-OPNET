/* dra_inoise_wsn.ps.c */                                                       
/* Default interference noise model for radio link Transceiver Pipeline */

/****************************************/
/*		  Copyright (c) 1993-2008		*/
/*		by OPNET Technologies, Inc.		*/
/*		(A Delaware Corporation)		*/
/*	7255 Woodmont Av., Suite 250  		*/
/*     Bethesda, MD 20814, U.S.A.       */
/*			All Rights Reserved.		*/
/****************************************/

#include "opnet.h"


#if defined (__cplusplus)
extern "C"
#endif
void
dra_inoise_wsn_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr_prev, Packet * pkptr_arriv)
	{
	int			arriv_match, prev_match;
	double		prev_rcvd_power, arriv_rcvd_power;

//additional var begin
	Objid		tx_objid, prev_pk_id, arriv_pk_id;
	Boolean		debug = OPC_FALSE;
	char		line0_buf [512],line1_buf [512];
//additional var end
	
	/** Evaluate a collision due to arrival of 'pkptr_arriv'	**/
	/** where 'pkptr_prev' is the packet that is currently		**/
	/** being received.											**/
	FIN_MT (dra_inoise_wsn (pkptr_prev, pkptr_arriv));

//additional code begin
	//added by Fei: obtain the id of the transmitter
	tx_objid = op_td_get_int(pkptr_arriv, OPC_TDA_RA_TX_OBJID);
	if(op_ima_obj_attr_exists(tx_objid,"debug"))
	{
		op_ima_obj_attr_get(tx_objid,"debug",&debug);
	}
	
	prev_pk_id = op_pk_id(pkptr_prev);
	arriv_pk_id = op_pk_id(pkptr_arriv);
	
	
//additional code end
	
	/* If the previous packet ends just as the new one begins, this is not	*/
	/* a collision (just a near miss, or perhaps back-to-back packets).		*/ 
	if (op_td_get_dbl (pkptr_prev, OPC_TDA_RA_END_RX) != op_sim_time ())
		{
		/* Increment the number of collisions in previous packet. */
		op_td_increment_int (pkptr_prev, OPC_TDA_RA_NUM_COLLS, 1);

		/* Increment number of collisions in arriving packet. */
		op_td_increment_int (pkptr_arriv, OPC_TDA_RA_NUM_COLLS, 1);

		/* Determine if previous packet is valid or noise. */
		prev_match = op_td_get_int (pkptr_prev, OPC_TDA_RA_MATCH_STATUS);

		/* Determine if arriving packet is valid or noise. */
		arriv_match = op_td_get_int (pkptr_arriv, OPC_TDA_RA_MATCH_STATUS);
		
//additional code begin	
		if(debug){
			sprintf(line0_buf,"in 8. arriv_pk_id=%d, arriv_match=%d. OPC_TDA_RA_MATCH_VALID=%d\n",\
				arriv_pk_id, arriv_match,OPC_TDA_RA_MATCH_VALID);
		
			sprintf(line1_buf,"in 8. prev_pk_id=%d, prev_match=%d.\n",prev_pk_id,prev_match);
		
			op_sim_message(line0_buf,line1_buf);
		}
//additional code end
		
		/* If the arriving packet is valid, calculate		*/
		/* interference of previous packet on arriving one.	*/
		if (arriv_match == OPC_TDA_RA_MATCH_VALID)
			{
				
			prev_rcvd_power   = op_td_get_dbl (pkptr_prev, OPC_TDA_RA_RCVD_POWER);
			op_td_increment_dbl (pkptr_arriv, OPC_TDA_RA_NOISE_ACCUM, prev_rcvd_power);
			}

		/* And vice-versa. */
		if (prev_match == OPC_TDA_RA_MATCH_VALID)
			{
					
			arriv_rcvd_power = op_td_get_dbl (pkptr_arriv, OPC_TDA_RA_RCVD_POWER);
			op_td_increment_dbl (pkptr_prev, OPC_TDA_RA_NOISE_ACCUM, arriv_rcvd_power);
			}
		}

	FOUT
	}                
