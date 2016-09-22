/* Process model C form file: gmac_sink.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char gmac_sink_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 4C0B02F4 4C0B02F4 1 china-0f9728557 Administrator 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                            ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

//Define packet type
#define grade_pk  1
#define rts_pk    2
#define cts_pk    3
#define ack_pk    4
#define data_pk   5

//Define stream in-out NO.
#define SRC_STRM 		1
#define RCV_STRM 		0
#define SEND_STRM 		0
#define DISCARD_STRM 	1

#define START_GRADE_CODE				1000
#define SEND_CTS_CODE					2000
#define NO_DATA_CODE					3000
#define SEND_ACK_CODE					4000

#define START_GRADE					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == START_GRADE_CODE))
#define SEND_CTS					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_CTS_CODE))
#define NO_DATA						((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == NO_DATA_CODE))
#define SEND_ACK					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_ACK_CODE))

#define FROM_LOWER					((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == RCV_STRM))
#define END	        		    	(op_intrpt_type() == OPC_INTRPT_ENDSIM)

//function prototype
static void rcv_pk_proc(void);

/* End of Header Block */

#if !defined (VOSD_NO_FIN)
#undef	BIN
#undef	BOUT
#define	BIN		FIN_LOCAL_FIELD(_op_last_line_passed) = __LINE__ - _op_block_origin;
#define	BOUT	BIN
#define	BINIT	FIN_LOCAL_FIELD(_op_last_line_passed) = 0; _op_block_origin = __LINE__;
#else
#define	BINIT
#endif /* #if !defined (VOSD_NO_FIN) */



/* State variable definitions */
typedef struct
	{
	/* Internal state tracking for FSM */
	FSM_SYS_STATE
	/* State Variables */
	Objid	                  		process_id                                      ;
	Objid	                  		node_id                                         ;
	int	                    		node_address                                    ;
	double	                 		start_grade_timer                               ;
	int	                    		grade                                           ;
	Boolean	                		RTS_flag                                        ;
	int	                    		dest                                            ;
	double	                 		SIFS                                            ;
	Evhandle	               		wait_DATA_event                                 ;
	double	                 		durDATA                                         ;
	double	                 		durCTS                                          ;
	} gmac_sink_state;

#define process_id              		op_sv_ptr->process_id
#define node_id                 		op_sv_ptr->node_id
#define node_address            		op_sv_ptr->node_address
#define start_grade_timer       		op_sv_ptr->start_grade_timer
#define grade                   		op_sv_ptr->grade
#define RTS_flag                		op_sv_ptr->RTS_flag
#define dest                    		op_sv_ptr->dest
#define SIFS                    		op_sv_ptr->SIFS
#define wait_DATA_event         		op_sv_ptr->wait_DATA_event
#define durDATA                 		op_sv_ptr->durDATA
#define durCTS                  		op_sv_ptr->durCTS

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	gmac_sink_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((gmac_sink_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif


//receive data from lower layer
static void
rcv_pk_proc(void)
{
//var
	int pk_type, grade_in_pk;
	int hop_num_TV;
	Packet *pk;
//in
	FIN(rcv_pk_proc(void));
//body
	pk = op_pk_get(RCV_STRM);
	op_pk_nfd_get(pk,"Type",&pk_type);
	op_pk_nfd_get(pk,"Grade",&grade_in_pk);
	switch(pk_type)
	{
		case rts_pk:
		{
			//reply_cts(pk);
			if(grade_in_pk == grade+1 && !RTS_flag)//the first time to receive RTS
			{
				RTS_flag = OPC_TRUE;
				op_pk_nfd_get(pk,"Src",&dest);
				op_intrpt_schedule_self(op_sim_time()+SIFS,SEND_CTS_CODE);
				printf("Have received RTS. Wait to reply CTS.\n");
			}
			op_pk_destroy(pk);
			break;
		}
		case data_pk:
		{
			if(op_ev_valid(wait_DATA_event))
			{
				op_ev_cancel(wait_DATA_event);
				//RTS_flag = OPC_FALSE;
				op_pk_nfd_get(pk,"Previous Hop",&dest);
				
				op_pk_nfd_get(pk,"Hop Num", &hop_num_TV);
				hop_num_TV++;
				op_pk_nfd_set(pk,"Hop Num",hop_num_TV);
				
				op_pk_send(pk,DISCARD_STRM);
				
				op_intrpt_schedule_self(op_sim_time()+SIFS,SEND_ACK_CODE);
				printf("Have received DATA. Wait to reply ACK.\n");
			}
			break;
		}
		default:
		{
			op_pk_destroy(pk);
			printf("The received pk is not RTS or DATA, destroy it.\n");
		}
	}
//out
	FOUT;
}

/* End of Function Block */

/* Undefine optional tracing in FIN/FOUT/FRET */
/* The FSM has its own tracing code and the other */
/* functions should not have any tracing.		  */
#undef FIN_TRACING
#define FIN_TRACING

#undef FOUTRET_TRACING
#define FOUTRET_TRACING

#if defined (__cplusplus)
extern "C" {
#endif
	void gmac_sink (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_gmac_sink_init (int * init_block_ptr);
	void _op_gmac_sink_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_gmac_sink_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_gmac_sink_alloc (VosT_Obtype, int);
	void _op_gmac_sink_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
gmac_sink (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (gmac_sink ());

		{
		/* Temporary Variables */
		Packet* pk_TV;
		//int pk_type;
		/* End of Temporary Variables */


		FSM_ENTER ("gmac_sink")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "gmac_sink [init enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_sink [init enter execs]", state0_enter_exec)
				{
				// Obtain related ID
				process_id = op_id_self();
				node_id = op_topo_parent(process_id);
				
				RTS_flag = OPC_FALSE;
				
				op_ima_obj_attr_get(process_id, "Start Grade Timer", &start_grade_timer);
				op_ima_obj_attr_get(process_id, "SIFS",&SIFS);	
				op_ima_obj_attr_get(process_id, "durDATA",&durDATA);
				op_ima_obj_attr_get(process_id, "durCTS",&durCTS);
				
				op_ima_obj_attr_get(node_id, "user id", &node_address);
				
				op_intrpt_schedule_self(op_sim_time() + start_grade_timer,START_GRADE_CODE);
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "gmac_sink [init exit execs]")


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_0", "gmac_sink [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "gmac_sink [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"gmac_sink")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "gmac_sink [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("gmac_sink [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (START_GRADE)
			FSM_TEST_COND (FROM_LOWER)
			FSM_TEST_COND (SEND_CTS)
			FSM_TEST_COND (NO_DATA)
			FSM_TEST_COND (SEND_ACK)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "START_GRADE", "", "idle", "start grade", "tr_1", "gmac_sink [idle -> start grade : START_GRADE / ]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "FROM_LOWER", "", "idle", "RCV proc", "tr_3", "gmac_sink [idle -> RCV proc : FROM_LOWER / ]")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "SEND_CTS", "", "idle", "send CTS", "tr_5", "gmac_sink [idle -> send CTS : SEND_CTS / ]")
				FSM_CASE_TRANSIT (3, 5, state5_enter_exec, ;, "NO_DATA", "", "idle", "no DATA", "tr_7", "gmac_sink [idle -> no DATA : NO_DATA / ]")
				FSM_CASE_TRANSIT (4, 6, state6_enter_exec, ;, "SEND_ACK", "", "idle", "send ACK", "tr_10", "gmac_sink [idle -> send ACK : SEND_ACK / ]")
				FSM_CASE_TRANSIT (5, 1, state1_enter_exec, ;, "default", "", "idle", "idle", "tr_13", "gmac_sink [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (start grade) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "start grade", state2_enter_exec, "gmac_sink [start grade enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_sink [start grade enter execs]", state2_enter_exec)
				{
				//Sink will execute the following code
				grade = 0;
				//sink_id = node_address;
				
				pk_TV = op_pk_create_fmt("MAC_GRADE_PK");
				op_pk_nfd_set(pk_TV, "Type", grade_pk);
				//op_pk_nfd_set(pk_TV, "Sink ID", sink_id);
				op_pk_nfd_set(pk_TV, "Grade", grade+1);
				
				op_pk_send(pk_TV, SEND_STRM);
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (start grade) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "start grade", "gmac_sink [start grade exit execs]")


			/** state (start grade) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "start grade", "idle", "tr_2", "gmac_sink [start grade -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (RCV proc) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "RCV proc", state3_enter_exec, "gmac_sink [RCV proc enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_sink [RCV proc enter execs]", state3_enter_exec)
				{
				//receive data from lower layer
				rcv_pk_proc();
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (RCV proc) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "RCV proc", "gmac_sink [RCV proc exit execs]")


			/** state (RCV proc) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "RCV proc", "idle", "tr_4", "gmac_sink [RCV proc -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (send CTS) enter executives **/
			FSM_STATE_ENTER_FORCED (4, "send CTS", state4_enter_exec, "gmac_sink [send CTS enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_sink [send CTS enter execs]", state4_enter_exec)
				{
				//send CTS	
				pk_TV = op_pk_create_fmt("MAC_CTS_PK");
				op_pk_nfd_set(pk_TV,"Type", cts_pk);
				op_pk_nfd_set(pk_TV,"Src",node_address);
				op_pk_nfd_set(pk_TV,"Dest",dest);
				op_pk_nfd_set(pk_TV,"Grade",grade);
						
				op_pk_send(pk_TV, SEND_STRM);
				
				//wait DATA
				wait_DATA_event = op_intrpt_schedule_self(op_sim_time() + durCTS+SIFS+durDATA, NO_DATA_CODE);
				printf("Have replied CTS, waiting to receive DATA.\n");
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** state (send CTS) exit executives **/
			FSM_STATE_EXIT_FORCED (4, "send CTS", "gmac_sink [send CTS exit execs]")


			/** state (send CTS) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "send CTS", "idle", "tr_6", "gmac_sink [send CTS -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (no DATA) enter executives **/
			FSM_STATE_ENTER_FORCED (5, "no DATA", state5_enter_exec, "gmac_sink [no DATA enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_sink [no DATA enter execs]", state5_enter_exec)
				{
				RTS_flag = OPC_FALSE;
				}
				FSM_PROFILE_SECTION_OUT (state5_enter_exec)

			/** state (no DATA) exit executives **/
			FSM_STATE_EXIT_FORCED (5, "no DATA", "gmac_sink [no DATA exit execs]")


			/** state (no DATA) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "no DATA", "idle", "tr_8", "gmac_sink [no DATA -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (send ACK) enter executives **/
			FSM_STATE_ENTER_FORCED (6, "send ACK", state6_enter_exec, "gmac_sink [send ACK enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_sink [send ACK enter execs]", state6_enter_exec)
				{
				//send ACK
				pk_TV = op_pk_create_fmt("MAC_ACK_PK");
				op_pk_nfd_set(pk_TV,"Type",ack_pk);
				op_pk_nfd_set(pk_TV,"Src",node_address);
				op_pk_nfd_set(pk_TV,"Dest",dest);
				op_pk_nfd_set(pk_TV,"Grade",grade);
				
				op_pk_send(pk_TV,SEND_STRM);
				printf("Have replied ACK.\n");
				RTS_flag = OPC_FALSE;
				}
				FSM_PROFILE_SECTION_OUT (state6_enter_exec)

			/** state (send ACK) exit executives **/
			FSM_STATE_EXIT_FORCED (6, "send ACK", "gmac_sink [send ACK exit execs]")


			/** state (send ACK) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "send ACK", "idle", "tr_11", "gmac_sink [send ACK -> idle : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"gmac_sink")
		}
	}




void
_op_gmac_sink_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_gmac_sink_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_gmac_sink_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_gmac_sink_svar function. */
#undef process_id
#undef node_id
#undef node_address
#undef start_grade_timer
#undef grade
#undef RTS_flag
#undef dest
#undef SIFS
#undef wait_DATA_event
#undef durDATA
#undef durCTS

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_gmac_sink_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_gmac_sink_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (gmac_sink)",
		sizeof (gmac_sink_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_gmac_sink_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	gmac_sink_state * ptr;
	FIN_MT (_op_gmac_sink_alloc (obtype))

	ptr = (gmac_sink_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "gmac_sink [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_gmac_sink_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	gmac_sink_state		*prs_ptr;

	FIN_MT (_op_gmac_sink_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (gmac_sink_state *)gen_ptr;

	if (strcmp ("process_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->process_id);
		FOUT
		}
	if (strcmp ("node_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_id);
		FOUT
		}
	if (strcmp ("node_address" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_address);
		FOUT
		}
	if (strcmp ("start_grade_timer" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->start_grade_timer);
		FOUT
		}
	if (strcmp ("grade" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->grade);
		FOUT
		}
	if (strcmp ("RTS_flag" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->RTS_flag);
		FOUT
		}
	if (strcmp ("dest" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->dest);
		FOUT
		}
	if (strcmp ("SIFS" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->SIFS);
		FOUT
		}
	if (strcmp ("wait_DATA_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wait_DATA_event);
		FOUT
		}
	if (strcmp ("durDATA" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durDATA);
		FOUT
		}
	if (strcmp ("durCTS" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durCTS);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

