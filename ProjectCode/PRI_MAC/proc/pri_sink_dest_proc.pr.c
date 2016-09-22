/* Process model C form file: pri_sink_dest_proc.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char pri_sink_dest_proc_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 52E5D3C7 52E5D3C7 1 wpd FeiTong 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                              ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

#define END	        		    	(op_intrpt_type() == OPC_INTRPT_ENDSIM)
//Define node type
#define sink 	1
#define sensor 	2

//Define remote-interrupt code 
#define REMOTE_STAT_CODE				500	/*remote statistics*/

int data_nums = 0;
int data_nums_throughput = 0;
double avg_delay = 0.0;

typedef struct
{
	int st_data_id;
}DataID;

typedef struct
{
	double pk_num;//packet number from a specific grade (indicated by list index)
	double delay;//packet delay from this grade
}GradeDelay;

static Boolean data_id_exist(int id);

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
	int	                    		node_type                                       ;
	int	                    		data_id                                         ;
	List *	                 		data_id_list                                    ;
	int	                    		node_address                                    ;
	Objid	                  		node_id                                         ;
	Stathandle	             		e2e_delay_per_sink_handle                       ;
	int	                    		data_nums_per_sink                              ;
	double	                 		avg_delay_per_sink                              ;
	double	                 		steady_time                                     ;
	List *	                 		delay_vs_grade_list                             ;
	int	                    		line_hops                                       ;
	Stathandle	             		delay_for_each_grade_handle                     ;
	} pri_sink_dest_proc_state;

#define process_id              		op_sv_ptr->process_id
#define node_type               		op_sv_ptr->node_type
#define data_id                 		op_sv_ptr->data_id
#define data_id_list            		op_sv_ptr->data_id_list
#define node_address            		op_sv_ptr->node_address
#define node_id                 		op_sv_ptr->node_id
#define e2e_delay_per_sink_handle		op_sv_ptr->e2e_delay_per_sink_handle
#define data_nums_per_sink      		op_sv_ptr->data_nums_per_sink
#define avg_delay_per_sink      		op_sv_ptr->avg_delay_per_sink
#define steady_time             		op_sv_ptr->steady_time
#define delay_vs_grade_list     		op_sv_ptr->delay_vs_grade_list
#define line_hops               		op_sv_ptr->line_hops
#define delay_for_each_grade_handle		op_sv_ptr->delay_for_each_grade_handle

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	pri_sink_dest_proc_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((pri_sink_dest_proc_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

static Boolean data_id_exist(int id)
{
//var
	int i,list_size;
	DataID* ID;
	Boolean flag=OPC_FALSE;
//in
	FIN(data_id_exist(int id));
//body
	list_size=op_prg_list_size(data_id_list);
	for(i=0;i<list_size;i++){
		ID=(DataID *)op_prg_list_access(data_id_list,i);
		if(ID->st_data_id==id){
			flag=OPC_TRUE;
			break;
		}
	}
//out
	FRET(flag);
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
	void pri_sink_dest_proc (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_pri_sink_dest_proc_init (int * init_block_ptr);
	void _op_pri_sink_dest_proc_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_pri_sink_dest_proc_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_pri_sink_dest_proc_alloc (VosT_Obtype, int);
	void _op_pri_sink_dest_proc_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
pri_sink_dest_proc (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (pri_sink_dest_proc ());

		{
		/* Temporary Variables */
		Packet*		pk_TV;
		double		ete_delay;
		int			hop;
		int 		src_TV;
		//int			data_id_TV;
		int			pk_type;
		//File
		FILE*		in;
		char 		temp_file_name[200];
		int 		nodes_num, sink_num;
		//char message[128];
		
		DataID*		id_TV;
		GradeDelay*	gd_TV; //will be used together with delay_vs_grade_list
		//for remote statistics
		Ici* 		iciptr_TV;
		int			list_size;
		int			steady_flag; //indicate whether the data is generated after the steady state
		/* End of Temporary Variables */


		FSM_ENTER ("pri_sink_dest_proc")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "pri_sink_dest_proc [init enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_sink_dest_proc [init enter execs]", state0_enter_exec)
				{
				process_id = op_id_self();
				node_id = op_topo_parent(process_id);
				//data_nums = 0;
				node_type = -1;
				op_ima_obj_attr_get(node_id, "user id", &node_address);
				
				op_ima_obj_attr_get(node_id, "Node Type",&node_type);
				op_ima_obj_attr_get(node_id, "Steady Time",&steady_time);
				op_ima_obj_attr_get(node_id, "Line Hops",&line_hops);
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
				data_id_list=op_prg_list_create();
				delay_vs_grade_list = op_prg_list_create();
				for(hop=0;hop<line_hops;hop++)
				{
					gd_TV = (GradeDelay *)op_prg_mem_alloc(sizeof(GradeDelay));
					gd_TV->pk_num = 0.0;
					gd_TV->delay = 0.0;
					op_prg_list_insert(delay_vs_grade_list,gd_TV,OPC_LISTPOS_TAIL);
				}
				//avg_delay=0;
				
				//avg_e2e_delay_handle = op_stat_reg("Average E2E Delay", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				e2e_delay_per_sink_handle=op_stat_reg("E2E Delay Per Sink",OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				delay_for_each_grade_handle=op_stat_reg("Delay for Each Grade",OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				
				data_nums_per_sink=0;
				avg_delay_per_sink=0.0;
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "pri_sink_dest_proc [init exit execs]")


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_0", "pri_sink_dest_proc [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "pri_sink_dest_proc [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"pri_sink_dest_proc")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "pri_sink_dest_proc [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_sink_dest_proc [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (op_intrpt_type() == OPC_INTRPT_STRM)
			FSM_TEST_COND (END)
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "op_intrpt_type() == OPC_INTRPT_STRM", "", "idle", "RCV", "tr_1", "pri_sink_dest_proc [idle -> RCV : op_intrpt_type() == OPC_INTRPT_STRM / ]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "END", "", "idle", "end", "tr_3", "pri_sink_dest_proc [idle -> end : END / ]")
				}
				/*---------------------------------------------------------*/



			/** state (RCV) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "RCV", state2_enter_exec, "pri_sink_dest_proc [RCV enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_sink_dest_proc [RCV enter execs]", state2_enter_exec)
				{
				pk_TV = op_pk_get (op_intrpt_strm ());
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				
				//printf("The type of packet is: %d.\n",pk_type);
				
				op_pk_nfd_get(pk_TV,"Data No",&data_id);
				op_pk_nfd_get(pk_TV,"Steady Flag",&steady_flag);
				
				if(op_sim_time() >= steady_time  && !data_id_exist(data_id)){ //comment: steady_flag == 1
					
					id_TV=(DataID *)op_prg_mem_alloc(sizeof(DataID));
					id_TV->st_data_id=data_id;
					op_prg_list_insert(data_id_list,id_TV,OPC_LISTPOS_TAIL);
				
					data_nums++;
					data_nums_per_sink++;
					
					if(op_sim_time()>=steady_time)
					{
						data_nums_throughput++;
						//printf("¸öÊýshi:%d\n\n",data_nums_throughput);
					}
					
					//ete_delay = op_sim_time () - op_pk_creation_time_get (pk_TV);
					//op_pk_nfd_get(pk_TV,"Create Time",&ete_delay);
					//ete_delay=op_pk_creation_time_get(pk_TV);
					ete_delay = op_sim_time () - op_pk_creation_time_get(pk_TV);
					//op_stat_write_t(avg_e2e_delay_handle,ete_delay,(double)data_nums);
					//sprintf(message,"2. the ete delay is: %f.",ete_delay);
					//op_sim_message(message,"");
					
					op_pk_nfd_get(pk_TV,"Hop Num",&hop);
				
					op_pk_nfd_get(pk_TV,"Src",&src_TV);
					
					avg_delay=avg_delay+ete_delay;
					avg_delay_per_sink = avg_delay_per_sink+ete_delay;
				
					op_ima_obj_attr_get(process_id,"DATA Delay Log",temp_file_name);
					in = fopen(temp_file_name,"at");
				
				    fprintf(in,"%d		%f		%d		%d	%d\r\n",data_nums,ete_delay,hop,src_TV,data_id);
					//fprintf(in,"%d	%f	%d\r\n",data_nums,ete_delay,hop);
				
					fclose(in);
					
					// statistic the packet delay fro each hop/grade
					op_pk_nfd_get(pk_TV,"Grade",&hop);
					op_pk_nfd_get(pk_TV,"Create Time",&ete_delay);
					//op_pk_nfd_get(pk_TV,"Create Time Two",&ete_delay);
					
					gd_TV = (GradeDelay *)op_prg_list_access(delay_vs_grade_list,hop-1);
					gd_TV->pk_num += 1.0;
					gd_TV->delay += op_sim_time() - ete_delay;
					
				}
				op_pk_destroy(pk_TV);
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (RCV) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "RCV", "pri_sink_dest_proc [RCV exit execs]")


			/** state (RCV) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "RCV", "idle", "tr_2", "pri_sink_dest_proc [RCV -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (end) enter executives **/
			FSM_STATE_ENTER_UNFORCED (3, "end", state3_enter_exec, "pri_sink_dest_proc [end enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_sink_dest_proc [end enter execs]", state3_enter_exec)
				{
				if(op_ima_obj_attr_exists(op_topo_parent(process_id),"Node Type"))
				{
					if(node_type == sink) //&& node_address == -1)
					{
						op_ima_obj_attr_get(process_id,"DATA Delay Log",temp_file_name);
						in = fopen(temp_file_name,"at");
						fprintf(in,"\r\n\r\n");
						fclose(in);
						
						nodes_num = op_topo_object_count (OPC_OBJTYPE_NDMOB);
						op_ima_obj_attr_get(op_id_from_name(node_id,OPC_OBJMTYPE_QPS,"pmac"),"Sink Num",&sink_num);
						
				//		op_stat_write_t(avg_e2e_delay_handle,(double)avg_delay/(double)data_nums,(double)data_nums);
				//		op_stat_write_t(avg_e2e_delay_handle,(double)avg_delay/(double)data_nums,(double)0.0);
						
						op_stat_write_t(e2e_delay_per_sink_handle,(double)avg_delay_per_sink/(double)data_nums_per_sink,(double)0.0);
						
						op_ima_obj_attr_get(process_id,"Throughput Delay Log",temp_file_name);
						in = fopen(temp_file_name,"at");
						//fprintf(in,"%f %f\r\n",data_nums/(op_sim_time()-60),avg_delay/data_nums);
						fprintf(in,"%f	%d\r\n",avg_delay/data_nums, data_nums);
						fclose(in);
						
						//delay statistics for each grade
						list_size = op_prg_list_size(delay_vs_grade_list);
						for(hop = 0;hop<list_size;hop++)
						{
							gd_TV = (GradeDelay *)op_prg_list_access(delay_vs_grade_list,hop);
							if(gd_TV->pk_num!=0)
							{
								op_stat_write_t(delay_for_each_grade_handle, gd_TV->delay/gd_TV->pk_num,hop+1);
							}
							
						}
						
						
						/****************************************************************/
						/*create ICI for statistic purpose; then set remote interruption*/
						if(node_address==-1){
						iciptr_TV = op_ici_create("dest_rcved_pkt_ici");
						op_ici_attr_set_int32(iciptr_TV, "dest rcved pkt", data_nums);
						op_ici_attr_set_int32(iciptr_TV, "dest rcved pkt throughput", data_nums_throughput);
						op_ici_install (iciptr_TV);
						op_intrpt_force_remote (REMOTE_STAT_CODE,op_id_from_name(node_id,OPC_OBJTYPE_PROC,"statistics"));
						op_ici_install (OPC_NIL);
						}
					}
				}else
				{	
					op_ima_obj_attr_get(process_id,"DATA Delay Log",temp_file_name);
					in = fopen(temp_file_name,"at");
					fprintf(in,"\r\n\r\n");
					fclose(in);
				}
				
				op_prg_list_free (data_id_list);
				op_prg_mem_free (data_id_list);
				
				op_prg_list_free (delay_vs_grade_list);
				op_prg_mem_free (delay_vs_grade_list);
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (7,"pri_sink_dest_proc")


			/** state (end) exit executives **/
			FSM_STATE_EXIT_UNFORCED (3, "end", "pri_sink_dest_proc [end exit execs]")


			/** state (end) transition processing **/
			FSM_TRANSIT_MISSING ("end")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"pri_sink_dest_proc")
		}
	}




void
_op_pri_sink_dest_proc_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_pri_sink_dest_proc_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_pri_sink_dest_proc_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_pri_sink_dest_proc_svar function. */
#undef process_id
#undef node_type
#undef data_id
#undef data_id_list
#undef node_address
#undef node_id
#undef e2e_delay_per_sink_handle
#undef data_nums_per_sink
#undef avg_delay_per_sink
#undef steady_time
#undef delay_vs_grade_list
#undef line_hops
#undef delay_for_each_grade_handle

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_pri_sink_dest_proc_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_pri_sink_dest_proc_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (pri_sink_dest_proc)",
		sizeof (pri_sink_dest_proc_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_pri_sink_dest_proc_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	pri_sink_dest_proc_state * ptr;
	FIN_MT (_op_pri_sink_dest_proc_alloc (obtype))

	ptr = (pri_sink_dest_proc_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "pri_sink_dest_proc [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_pri_sink_dest_proc_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	pri_sink_dest_proc_state		*prs_ptr;

	FIN_MT (_op_pri_sink_dest_proc_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (pri_sink_dest_proc_state *)gen_ptr;

	if (strcmp ("process_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->process_id);
		FOUT
		}
	if (strcmp ("node_type" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_type);
		FOUT
		}
	if (strcmp ("data_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->data_id);
		FOUT
		}
	if (strcmp ("data_id_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->data_id_list);
		FOUT
		}
	if (strcmp ("node_address" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_address);
		FOUT
		}
	if (strcmp ("node_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_id);
		FOUT
		}
	if (strcmp ("e2e_delay_per_sink_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->e2e_delay_per_sink_handle);
		FOUT
		}
	if (strcmp ("data_nums_per_sink" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->data_nums_per_sink);
		FOUT
		}
	if (strcmp ("avg_delay_per_sink" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_delay_per_sink);
		FOUT
		}
	if (strcmp ("steady_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->steady_time);
		FOUT
		}
	if (strcmp ("delay_vs_grade_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->delay_vs_grade_list);
		FOUT
		}
	if (strcmp ("line_hops" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->line_hops);
		FOUT
		}
	if (strcmp ("delay_for_each_grade_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->delay_for_each_grade_handle);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

