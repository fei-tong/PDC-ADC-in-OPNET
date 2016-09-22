/* dra_rxgroup_wsn.ps.c */                                                       
/* Default receiver group model for radio link Transceiver	*/
/* Pipeline. This model populates the state information of	*/
/* the receiver channels to be used in power and ecc		*/
/* pipeline stage models. If you don't want the receiver	*/
/* channel state information to be set and used by radio	*/
/* pipeline stage models, then use "*_no_rxstate" version	*/
/* of rxgroup, power and ecc stage models in your node		*/
/* models.													*/					

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
	
int
dra_rxgroup_wsn_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Objid PRG_ARG_UNUSED(tx_obid), Objid rx_obid)
	{
	DraT_Rxch_State_Info*	rxch_state_ptr;
	
//self-defined variables begin
		
	//double          		tx_alt, tx_lat, tx_lon, tx_x, tx_y, tx_z; 
	//double		        rx_alt, rx_lat, rx_lon, rx_x, rx_y, rx_z;

	//double 					tx_x, tx_y, rx_x, rx_y;
	//double 					distance, distance_threshold;
	Objid                   tx_node_objid, rx_node_objid;
	Boolean                 transceiver_state;
	
//self-defined variables end
	
	/** Determine the potential for communication between	**/
	/** given transmitter and receiver channel objects.		**/
	/** Also create and initialize the receiver channel's	**/
	/** state information to be used by other pipeline		**/
	/** stages during the simulation.						**/
	FIN_MT (dra_rxgroup_wsn (tx_obid, rx_obid));

	/* Unless it is already done, initialize the receiver	*/
	/* channel's state information.							*/
	if (op_ima_obj_state_get (rx_obid) == OPC_NIL)
		{
#if defined (OPD_PARALLEL)
		/* Channel state information doesn't exist. Lock	*/
		/* the global mutex before continuing.				*/
		op_prg_mt_global_lock ();
		
		/* Check again since another thread may have		*/
		/* already set up the state information.			*/
		if (op_ima_obj_state_get (rx_obid) == OPC_NIL)
			{
#endif /* OPD_PARALLEL */
			/* Create and set the initial state information	*/
			/* for the receiver channel. State information	*/
			/* is used by other pipeline stages to			*/
			/* access/update channel specific data			*/
			/* efficiently.									*/
			rxch_state_ptr = (DraT_Rxch_State_Info *) 
				op_prg_mem_alloc (sizeof (DraT_Rxch_State_Info));
			rxch_state_ptr->signal_lock = OPC_FALSE;
			op_ima_obj_state_set (rx_obid, rxch_state_ptr);
#if defined (OPD_PARALLEL)
			}
		
		/* Unlock the global mutex.							*/
		op_prg_mt_global_unlock ();
#endif /* OPD_PARALLEL */
		}
	
//self-defined codes begin
	tx_node_objid = op_topo_parent (op_topo_parent (op_topo_parent (PRG_ARG_UNUSED(tx_obid))));
	rx_node_objid = op_topo_parent (op_topo_parent (op_topo_parent (rx_obid)));
	
	//station should not hear its own transmission
	if(tx_node_objid == rx_node_objid)
	{
		FRET (OPC_FALSE);
	}
/*	
	if(op_ima_obj_attr_exists(tx_node_objid,"Communication Range"))
	{	
		op_ima_obj_attr_get(tx_node_objid,"Communication Range",&distance_threshold);
	
		//obtain the position of sender & receiver respectively
		//op_ima_obj_pos_get (tx_node_objid,&tx_lat, &tx_lon, &tx_alt, &tx_x, &tx_y, &tx_z);
		//op_ima_obj_pos_get (rx_node_objid,	&rx_lat, &rx_lon, &rx_alt, &rx_x, &rx_y, &rx_z);
		op_ima_obj_attr_get(tx_node_objid,"x position",&tx_x); op_ima_obj_attr_get(tx_node_objid,"y position",&tx_y);
		op_ima_obj_attr_get(rx_node_objid,"x position",&rx_x); op_ima_obj_attr_get(rx_node_objid,"y position",&rx_y);
		// Find the distance between the tx_rx pair.
		//distance = prg_geo_lat_long_distance_get (tx_lat, tx_lon, tx_alt, rx_lat, rx_lon, rx_alt);
		distance = sqrt((tx_x-rx_x)*(tx_x-rx_x)+(tx_y-rx_y)*(tx_y-rx_y));
		
		//printf("in rxgroup: distance=%lf.\n",distance);
		if (distance_threshold < distance)
		{
			//printf("in rxgroup: Distance is too long!\n");
			FRET (OPC_FALSE);
		}
	}	
*/	
	//obtain the state of the radio receiver
	if(op_ima_obj_attr_exists(op_topo_parent (op_topo_parent (rx_obid)),"state"))
	{	
		op_ima_obj_attr_get(op_topo_parent (op_topo_parent (rx_obid)),"state",&transceiver_state);
		if (transceiver_state == OPC_FALSE)
		{
			FRET (OPC_FALSE);
		}
	}
//self-defined codes end
		
	/* By default, all receivers are considered as			*/
	/* potential destinations.								*/
	FRET (OPC_TRUE)
	}                
