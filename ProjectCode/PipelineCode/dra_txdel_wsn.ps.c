/* dra_txdel_wsn.ps.c */                                                       
/* Default transmission delay model for radio link Transceiver Pipeline */

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
	
/*
//self-defined function begin
static int
check_rxch (Objid txch, Objid rxch)
{
//var
	Objid 		tx_node_id, rx_node_id;
	Boolean		transceiver_state;
	
	//double      tx_alt, tx_lat, tx_lon, tx_x, tx_y, tx_z; 
	//double		rx_alt, rx_lat, rx_lon, rx_x, rx_y, rx_z;
	
	//double		tx_x, tx_y, rx_x, rx_y;
	//double 		distance, distance_threshold;
//in
	FIN(check_rxch(txch,rxch));
//body
	tx_node_id = op_topo_parent (op_topo_parent (op_topo_parent (txch)));
	rx_node_id = op_topo_parent (op_topo_parent (op_topo_parent (rxch)));
	
	// Now we are ready to perform eligible receiver checks.
	// Station should not hear its own transmission.		
	if (tx_node_id == rx_node_id)
	{
		FRET (OPC_FALSE);
    }

	//obtain the state of the radio receiver
	if(op_ima_obj_attr_exists(op_topo_parent (op_topo_parent (rxch)),"state"))
	{	
		op_ima_obj_attr_get(op_topo_parent (op_topo_parent (rxch)),"state",&transceiver_state);
		if (transceiver_state == OPC_FALSE)
		{
			FRET (OPC_FALSE);
		}
	}
//out
	FRET(OPC_TRUE);
}
//self-defined function end
*/

void
dra_txdel_wsn_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr)
	{
	OpT_Packet_Size	pklen;
	double			tx_drate, tx_delay;
	
//self-defined variables begin
	Objid tx_ch_objid; //the object ID of the transmitter channel
//self-defined variables end
	
	/** Compute the transmission delay associated with the	**/
	/** transmission of a packet over a radio link.			**/
	FIN_MT (dra_txdel_wsn (pkptr));

	
//self-defined codes begin
	tx_ch_objid = op_td_get_int (pkptr, OPC_TDA_RA_TX_CH_OBJID);
	
	//The invoked function is set to OPC_TXCH_RXGROUP_PS, which means 
	//the receiver group pipeline stage for the transmitter is used.
	op_radio_txch_rxgroup_compute(tx_ch_objid, OPC_TXCH_RXGROUP_PS);
//self-defined codes end
	
	
	/* Obtain the transmission rate of that channel. */
	tx_drate = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_DRATE);

	/* Obtain length of packet. */
	pklen = op_pk_total_size_get (pkptr);

	/* Compute time required to complete transmission of packet. */
	tx_delay = pklen / tx_drate;

	/* Place transmission delay result in packet's */
	/* reserved transmission data attribute. */
	op_td_set_dbl (pkptr, OPC_TDA_RA_TX_DELAY, tx_delay);

	FOUT
	}
