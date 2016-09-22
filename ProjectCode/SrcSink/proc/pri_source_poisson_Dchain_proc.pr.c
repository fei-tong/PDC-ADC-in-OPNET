/* Process model C form file: pri_source_poisson_Dchain_proc.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char pri_source_poisson_Dchain_proc_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 51CA3533 51CA3533 1 wpd FeiTong 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                              ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

#define SEND_STRM 0
//Define node type
#define sink 	1
#define sensor 	2

#define END	        		    	(op_intrpt_type() == OPC_INTRPT_ENDSIM)

//Self-interrupt code and transition condition
#define SEND_DATA_S_CODE		100
#define SEND_DATA_S				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_DATA_S_CODE))
//Remote-interrupt code and transition condition
#define SEND_DATA_R_CODE		200
#define SEND_DATA_R				((op_intrpt_type() == OPC_INTRPT_REMOTE) && (op_intrpt_code() == SEND_DATA_R_CODE))

int pk_num=0;
//function prototype
static void create_and_send_DATA(void);
data_num=0;

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
	double	                 		start_time                                      ;
	Boolean	                		send_DATA                                       ;
	int	                    		node_address                                    ;
	int	                    		traffic_load                                    ;
	Boolean	                		infinity                                        ;
	int	                    		node_type                                       ;
	int	                    		self_pk_num                                     ;
	Boolean	                		debug                                           ;
	double	                 		arrival_rate                                    ;
	Stathandle	             		traffic_sent_hndl                               ;
	Stathandle	             		packets_sent_hndl                               ;
	Stathandle	             		global_packets_sent_hndl                        ;
	} pri_source_poisson_Dchain_proc_state;

#define process_id              		op_sv_ptr->process_id
#define node_id                 		op_sv_ptr->node_id
#define start_time              		op_sv_ptr->start_time
#define send_DATA               		op_sv_ptr->send_DATA
#define node_address            		op_sv_ptr->node_address
#define traffic_load            		op_sv_ptr->traffic_load
#define infinity                		op_sv_ptr->infinity
#define node_type               		op_sv_ptr->node_type
#define self_pk_num             		op_sv_ptr->self_pk_num
#define debug                   		op_sv_ptr->debug
#define arrival_rate            		op_sv_ptr->arrival_rate
#define traffic_sent_hndl       		op_sv_ptr->traffic_sent_hndl
#define packets_sent_hndl       		op_sv_ptr->packets_sent_hndl
#define global_packets_sent_hndl		op_sv_ptr->global_packets_sent_hndl

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	pri_source_poisson_Dchain_proc_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((pri_source_poisson_Dchain_proc_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

//create DATA pk
static void 
create_and_send_DATA(void)
{
//var
	Packet * pk_DATA;
//in
	FIN(create_and_send_DATA());
//body
	
	data_num++;
	pk_DATA = op_pk_create_fmt("MAC_DATA_PK");
	op_pk_nfd_set(pk_DATA,"Hop Num",0);
	op_pk_nfd_set(pk_DATA,"Create Time",op_sim_time());
	op_pk_nfd_set(pk_DATA,"Data No",data_num);
	op_pk_send(pk_DATA,SEND_STRM);
	
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
	void pri_source_poisson_Dchain_proc (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_pri_source_poisson_Dchain_proc_init (int * init_block_ptr);
	void _op_pri_source_poisson_Dchain_proc_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_pri_source_poisson_Dchain_proc_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_pri_source_poisson_Dchain_proc_alloc (VosT_Obtype, int);
	void _op_pri_source_poisson_Dchain_proc_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
pri_source_poisson_Dchain_proc (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (pri_source_poisson_Dchain_proc ());

		{
		/* Temporary Variables */
		//File
		FILE *in;
		char temp_file_name[300];
		/* End of Temporary Variables */


		FSM_ENTER ("pri_source_poisson_Dchain_proc")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "pri_source_poisson_Dchain_proc [init enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_source_poisson_Dchain_proc [init enter execs]", state0_enter_exec)
				{
				// Obtain related ID
				process_id = op_id_self();
				node_id = op_topo_parent(process_id);
				
				op_ima_obj_attr_get(process_id, "Send DATA",&send_DATA);
				op_ima_obj_attr_get(process_id, "Start Time",&start_time);
				//op_ima_obj_attr_get(process_id, "Interval",&interval);
				op_ima_obj_attr_get(process_id, "Traffic Load",&traffic_load);
				op_ima_obj_attr_get(process_id, "Debug",&debug);
				op_ima_obj_attr_get(process_id,"Packet Arrival Rate",&arrival_rate);
				
				op_ima_obj_attr_get(node_id, "user id", &node_address);
				op_ima_obj_attr_get(node_id, "Node Type",&node_type);
				
				
				if(traffic_load<0)
				{
					infinity = OPC_TRUE;
				}else
				{
					infinity = OPC_FALSE;
				}
				
				
				if(send_DATA&&traffic_load<0)
				{
					op_intrpt_schedule_self(op_sim_time() + start_time,SEND_DATA_S_CODE);
				}
				if(send_DATA&&traffic_load>0)
				{
					op_intrpt_schedule_self(op_sim_time() + start_time,SEND_DATA_S_CODE);
					--traffic_load;
				}
				traffic_sent_hndl = op_stat_reg ("Generator.Traffic Sent (packets/sec)",OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				packets_sent_hndl = op_stat_reg ("Generator.Packets Sent (packets)",OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				global_packets_sent_hndl = op_stat_reg ("Generator.Packets Sent (packets)",OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				//pk_num=0;
				self_pk_num=0;
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "pri_source_poisson_Dchain_proc [init exit execs]")


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_0", "pri_source_poisson_Dchain_proc [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "pri_source_poisson_Dchain_proc [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"pri_source_poisson_Dchain_proc")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "pri_source_poisson_Dchain_proc [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_source_poisson_Dchain_proc [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (SEND_DATA_S)
			FSM_TEST_COND (SEND_DATA_R)
			FSM_TEST_COND (END)
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "SEND_DATA_S", "", "idle", "send DATA S", "tr_3", "pri_source_poisson_Dchain_proc [idle -> send DATA S : SEND_DATA_S / ]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "SEND_DATA_R", "", "idle", "send DATA R", "tr_4", "pri_source_poisson_Dchain_proc [idle -> send DATA R : SEND_DATA_R / ]")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "END", "", "idle", "end", "tr_6", "pri_source_poisson_Dchain_proc [idle -> end : END / ]")
				}
				/*---------------------------------------------------------*/



			/** state (send DATA S) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "send DATA S", state2_enter_exec, "pri_source_poisson_Dchain_proc [send DATA S enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_source_poisson_Dchain_proc [send DATA S enter execs]", state2_enter_exec)
				{
				create_and_send_DATA();
				pk_num++;
				self_pk_num++;
				
				op_stat_write (traffic_sent_hndl, 1.0);
				op_stat_write (traffic_sent_hndl, 0.0);
				op_stat_write (packets_sent_hndl, (double) self_pk_num);
				op_stat_write (global_packets_sent_hndl, (double)pk_num);
				
				
				if(!infinity && traffic_load>0)
				{
					op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(1.0/arrival_rate),SEND_DATA_S_CODE);
					--traffic_load;
				}	
				if(infinity)
				{
					op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(1.0/arrival_rate),SEND_DATA_S_CODE);
				}
				
				if(debug)
				printf("Node: %d. In \"gsrc\",time:%f,\nHave created DATA and sent it to \"gmac\".\n",node_address,op_sim_time());
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (send DATA S) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "send DATA S", "pri_source_poisson_Dchain_proc [send DATA S exit execs]")


			/** state (send DATA S) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "send DATA S", "idle", "tr_2", "pri_source_poisson_Dchain_proc [send DATA S -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (send DATA R) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "send DATA R", state3_enter_exec, "pri_source_poisson_Dchain_proc [send DATA R enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_source_poisson_Dchain_proc [send DATA R enter execs]", state3_enter_exec)
				{
				create_and_send_DATA();
				pk_num++;
				self_pk_num++;
				
				if(debug)
				printf("In \"gsrc\",Remote Interruption,time:%f,\n\
						Have created DATA and sent it to \"gmac\".\n",\
						op_sim_time());
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (send DATA R) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "send DATA R", "pri_source_poisson_Dchain_proc [send DATA R exit execs]")


			/** state (send DATA R) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "send DATA R", "idle", "tr_5", "pri_source_poisson_Dchain_proc [send DATA R -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (end) enter executives **/
			FSM_STATE_ENTER_UNFORCED (4, "end", state4_enter_exec, "pri_source_poisson_Dchain_proc [end enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_source_poisson_Dchain_proc [end enter execs]", state4_enter_exec)
				{
				if(send_DATA)
				{
					if(op_ima_obj_attr_exists(node_id,"Log File"))
					{	
						op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
						in = fopen(temp_file_name,"at");
						fprintf(in,"Node %d sent %d packets.\nAll soruces totally sent %d packets. (in \"gsrc->end\")\r\n",node_address,self_pk_num,pk_num);
						//fprintf(in,"The sending interval is %f seconds.\r\n",interval);
						fprintf(in,"Simulation time: %f s.\r\n",op_sim_time());
						fprintf(in,"End.\r\n\r\n");
						fclose(in);
					}
				}else
				
				if(traffic_load==0 && self_pk_num!=0)
				{
				
					if(op_ima_obj_attr_exists(node_id,"Log File"))
					{	
						op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
						in = fopen(temp_file_name,"at");
						fprintf(in,"Node %d sent %d packets.\nAll sources totally sent %d packets.(in \"gsrc->end\").\r\n",node_address,self_pk_num,pk_num);
						//fprintf(in,"发包间隔为：%f seconds.\r\n",interval);
						fprintf(in,"Simulation time: %f s.\r\n",op_sim_time());
						fprintf(in,"End.\r\n\r\n");
						fclose(in);
					}
				}
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,"pri_source_poisson_Dchain_proc")


			/** state (end) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "end", "pri_source_poisson_Dchain_proc [end exit execs]")


			/** state (end) transition processing **/
			FSM_TRANSIT_MISSING ("end")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"pri_source_poisson_Dchain_proc")
		}
	}




void
_op_pri_source_poisson_Dchain_proc_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_pri_source_poisson_Dchain_proc_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_pri_source_poisson_Dchain_proc_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_pri_source_poisson_Dchain_proc_svar function. */
#undef process_id
#undef node_id
#undef start_time
#undef send_DATA
#undef node_address
#undef traffic_load
#undef infinity
#undef node_type
#undef self_pk_num
#undef debug
#undef arrival_rate
#undef traffic_sent_hndl
#undef packets_sent_hndl
#undef global_packets_sent_hndl

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_pri_source_poisson_Dchain_proc_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_pri_source_poisson_Dchain_proc_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (pri_source_poisson_Dchain_proc)",
		sizeof (pri_source_poisson_Dchain_proc_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_pri_source_poisson_Dchain_proc_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	pri_source_poisson_Dchain_proc_state * ptr;
	FIN_MT (_op_pri_source_poisson_Dchain_proc_alloc (obtype))

	ptr = (pri_source_poisson_Dchain_proc_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "pri_source_poisson_Dchain_proc [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_pri_source_poisson_Dchain_proc_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	pri_source_poisson_Dchain_proc_state		*prs_ptr;

	FIN_MT (_op_pri_source_poisson_Dchain_proc_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (pri_source_poisson_Dchain_proc_state *)gen_ptr;

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
	if (strcmp ("start_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->start_time);
		FOUT
		}
	if (strcmp ("send_DATA" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_DATA);
		FOUT
		}
	if (strcmp ("node_address" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_address);
		FOUT
		}
	if (strcmp ("traffic_load" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->traffic_load);
		FOUT
		}
	if (strcmp ("infinity" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->infinity);
		FOUT
		}
	if (strcmp ("node_type" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_type);
		FOUT
		}
	if (strcmp ("self_pk_num" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->self_pk_num);
		FOUT
		}
	if (strcmp ("debug" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->debug);
		FOUT
		}
	if (strcmp ("arrival_rate" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->arrival_rate);
		FOUT
		}
	if (strcmp ("traffic_sent_hndl" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->traffic_sent_hndl);
		FOUT
		}
	if (strcmp ("packets_sent_hndl" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->packets_sent_hndl);
		FOUT
		}
	if (strcmp ("global_packets_sent_hndl" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->global_packets_sent_hndl);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

