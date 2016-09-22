/* Process model C form file: sink_dest_mobil.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char sink_dest_mobil_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 4C8999AB 4C8999AB 1 china-0f9728557 Administrator 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                            ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

#define END	        		    	(op_intrpt_type() == OPC_INTRPT_ENDSIM)
//Define node type
#define sink 	1
#define sensor 	2

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
	int	                    		data_nums                                       ;
	int	                    		node_type                                       ;
	} sink_dest_mobil_state;

#define process_id              		op_sv_ptr->process_id
#define data_nums               		op_sv_ptr->data_nums
#define node_type               		op_sv_ptr->node_type

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	sink_dest_mobil_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((sink_dest_mobil_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* No Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ };
#endif

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
	void sink_dest_mobil (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_sink_dest_mobil_init (int * init_block_ptr);
	void _op_sink_dest_mobil_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_sink_dest_mobil_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_sink_dest_mobil_alloc (VosT_Obtype, int);
	void _op_sink_dest_mobil_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
sink_dest_mobil (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (sink_dest_mobil ());

		{
		/* Temporary Variables */
		Packet 		* pk_TV;
		double		ete_delay;
		int			hop;
		int 		src_TV;
		//File
		FILE		*in;
		char 		temp_file_name[200];
		/* End of Temporary Variables */


		FSM_ENTER ("sink_dest_mobil")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "sink_dest_mobil [init enter execs]")
				FSM_PROFILE_SECTION_IN ("sink_dest_mobil [init enter execs]", state0_enter_exec)
				{
				process_id = op_id_self();
				data_nums = 0;
				node_type = -1;
				/*
				if(op_ima_obj_attr_exists(op_topo_parent(process_id),"Node Type"))
				{
					op_ima_obj_attr_get(op_topo_parent(process_id),"Node Type",&node_type);
					
					if(node_type == sink)
					{
						op_ima_obj_attr_get(process_id,"DATA Delay Log",temp_file_name);
						in = fopen(temp_file_name,"at");
						fprintf(in,"		delay			hop		src_node\r\n");
						fclose(in);
					}
				}else
				{
					op_ima_obj_attr_get(process_id,"DATA Delay Log",temp_file_name);
					in = fopen(temp_file_name,"at");
					fprintf(in,"		in sink_dest\r\n");
					fprintf(in,"		delay			hop		src_node\r\n");
					fclose(in);
				}	
				*/
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "sink_dest_mobil [init exit execs]")


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_0", "sink_dest_mobil [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "sink_dest_mobil [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"sink_dest_mobil")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "sink_dest_mobil [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("sink_dest_mobil [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (op_intrpt_type() == OPC_INTRPT_STRM)
			FSM_TEST_COND (END)
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "op_intrpt_type() == OPC_INTRPT_STRM", "", "idle", "RCV", "tr_1", "sink_dest_mobil [idle -> RCV : op_intrpt_type() == OPC_INTRPT_STRM / ]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "END", "", "idle", "end", "tr_3", "sink_dest_mobil [idle -> end : END / ]")
				}
				/*---------------------------------------------------------*/



			/** state (RCV) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "RCV", state2_enter_exec, "sink_dest_mobil [RCV enter execs]")
				FSM_PROFILE_SECTION_IN ("sink_dest_mobil [RCV enter execs]", state2_enter_exec)
				{
				pk_TV = op_pk_get (op_intrpt_strm ());
				data_nums++;
				//ete_delay = op_sim_time () - op_pk_creation_time_get (pk_TV);
				op_pk_nfd_get(pk_TV,"Create Time",&ete_delay);
				ete_delay = op_sim_time () - ete_delay;
				
				op_pk_nfd_get(pk_TV,"Hop Num",&hop);
				
				op_pk_nfd_get(pk_TV,"Src",&src_TV);
				op_pk_destroy(pk_TV);
				
				op_ima_obj_attr_get(process_id,"DATA Delay Log",temp_file_name);
				in = fopen(temp_file_name,"at");
				
				//fprintf(in,"%d		%f		%d		%d\r\n",data_nums,ete_delay,hop,src_TV);
				fprintf(in,"%d	%f	%d\r\n",data_nums,ete_delay,src_TV);
				
				fclose(in);
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (RCV) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "RCV", "sink_dest_mobil [RCV exit execs]")


			/** state (RCV) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "RCV", "idle", "tr_2", "sink_dest_mobil [RCV -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (end) enter executives **/
			FSM_STATE_ENTER_UNFORCED (3, "end", state3_enter_exec, "sink_dest_mobil [end enter execs]")
				FSM_PROFILE_SECTION_IN ("sink_dest_mobil [end enter execs]", state3_enter_exec)
				{
				if(op_ima_obj_attr_exists(op_topo_parent(process_id),"Node Type"))
				{
					op_ima_obj_attr_get(op_topo_parent(process_id),"Node Type",&node_type);
					if(node_type == sink)
					{
						op_ima_obj_attr_get(process_id,"DATA Delay Log",temp_file_name);
						in = fopen(temp_file_name,"at");
						fprintf(in,"\r\n\r\n");
						fclose(in);
					}
				}else
				{	
					op_ima_obj_attr_get(process_id,"DATA Delay Log",temp_file_name);
					in = fopen(temp_file_name,"at");
					fprintf(in,"\r\n\r\n");
					fclose(in);
				}
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (7,"sink_dest_mobil")


			/** state (end) exit executives **/
			FSM_STATE_EXIT_UNFORCED (3, "end", "sink_dest_mobil [end exit execs]")


			/** state (end) transition processing **/
			FSM_TRANSIT_MISSING ("end")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"sink_dest_mobil")
		}
	}




void
_op_sink_dest_mobil_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_sink_dest_mobil_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_sink_dest_mobil_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_sink_dest_mobil_svar function. */
#undef process_id
#undef data_nums
#undef node_type

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_sink_dest_mobil_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_sink_dest_mobil_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (sink_dest_mobil)",
		sizeof (sink_dest_mobil_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_sink_dest_mobil_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	sink_dest_mobil_state * ptr;
	FIN_MT (_op_sink_dest_mobil_alloc (obtype))

	ptr = (sink_dest_mobil_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "sink_dest_mobil [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_sink_dest_mobil_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	sink_dest_mobil_state		*prs_ptr;

	FIN_MT (_op_sink_dest_mobil_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (sink_dest_mobil_state *)gen_ptr;

	if (strcmp ("process_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->process_id);
		FOUT
		}
	if (strcmp ("data_nums" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->data_nums);
		FOUT
		}
	if (strcmp ("node_type" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_type);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

