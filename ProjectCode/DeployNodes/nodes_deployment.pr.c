/* Process model C form file: nodes_deployment.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char nodes_deployment_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 553468E2 553468E2 1 wpd FeiTong 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                              ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

//Define node type
#include <stdlib.h>

#define sink 	1
#define sensor 	2
//code for remote interruption
#define SEND_DATA_R_CODE		200
#define START_APPOINT_CODE		300

#define START_APPOINT		((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == START_APPOINT_CODE))

static Boolean 	load_coordinate_file(void);
static void 	appoint_sources(void);

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
	Objid	                  		network_id                                      ;
	int	                    		num_nodes                                       ;
	int	                    		src_num                                         ;
	double	                 		start_appoint                                   ;
	double	                 		appoint_interval                                ;
	int	                    		initial_userId                                  ;
	char	                   		module_name[20]                                 ;
	Boolean	                		appoint_on_off                                  ;
	int	                    		traffic_load                                    ;
	Boolean	                		infinity                                        ;
	char	                   		node_name[10]                                   ;
	int	                    		chain_number                                    ;	/* get the number of chains from the node attribute, Chain Number */
	} nodes_deployment_state;

#define process_id              		op_sv_ptr->process_id
#define node_id                 		op_sv_ptr->node_id
#define network_id              		op_sv_ptr->network_id
#define num_nodes               		op_sv_ptr->num_nodes
#define src_num                 		op_sv_ptr->src_num
#define start_appoint           		op_sv_ptr->start_appoint
#define appoint_interval        		op_sv_ptr->appoint_interval
#define initial_userId          		op_sv_ptr->initial_userId
#define module_name             		op_sv_ptr->module_name
#define appoint_on_off          		op_sv_ptr->appoint_on_off
#define traffic_load            		op_sv_ptr->traffic_load
#define infinity                		op_sv_ptr->infinity
#define node_name               		op_sv_ptr->node_name
#define chain_number            		op_sv_ptr->chain_number

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	nodes_deployment_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((nodes_deployment_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

static Boolean load_coordinate_file(void)
{
//var
	int i,counter,user_id,node_type;
	double x_pos, y_pos;
	//FILE *in,*sink_in;
	char file_name_tv[300];
	Objid ch_id;
	int sink_id=-1;
	int num_rows;
	List * coordinate_list_ptr;
	List * field_list_ptr;
//in
	FIN(load_coordinate_file(void));
//body
//	printf("The current path : %s\n", system("dir"));
	op_ima_obj_attr_get(process_id,"Coordinate File", file_name_tv);
	coordinate_list_ptr=op_prg_gdf_read(file_name_tv);
	if(coordinate_list_ptr == OPC_NIL)
	{
		op_sim_end("The Coordinate File could not be loaded.\n","","","");
	}
	num_rows=op_prg_list_size(coordinate_list_ptr);
/*	for(i=0;i<num_rows;i++)
	{
		printf("%s\n",(char *)op_prg_list_access(coordinate_list_ptr,i));
		
	}
*/	//op_prg_list_free (coordinate_list);
	//op_prg_mem_free (coordinate_list);
	
/*	if((in = fopen(file_name_tv,"rt"))==NULL)
	{
		printf("The Coordinate File Can't be opened.\n");
		FRET(OPC_FALSE);
	}

	
	op_ima_obj_attr_get(process_id,"Sink Coordinate File", file_name_tv);
	if((sink_in = fopen(file_name_tv,"rt"))==NULL)
	{
		printf("The Sink Coordinate File Can't be opened.\n");
		FRET(OPC_FALSE);
	}
*/
	counter=0;
	num_rows=0;
	for(i=0; i<num_nodes; i++)
	{
		ch_id = op_topo_object(OPC_OBJTYPE_NDMOB, i);
		op_ima_obj_attr_get(ch_id, "Node Type",&node_type);
		
		if(node_type==sink)//sink
		{
			op_ima_obj_attr_get(process_id,"sink_pos_x",&x_pos);
			op_ima_obj_attr_get(process_id,"sink_pos_y",&y_pos);
			op_ima_obj_attr_set(ch_id, "user id",sink_id--);
					
			//fscanf(sink_in,"%lf,%lf ",&x_pos,&y_pos);
			
			//field_list_ptr=op_prg_str_decomp(op_prg_list_access(coordinate_list_ptr,i),",");
			//x_pos = atof(op_prg_list_access (field_list_ptr, 0));
			//y_pos = atof(op_prg_list_access (field_list_ptr, 1));
			op_ima_obj_attr_set(ch_id, "x position",x_pos);
			op_ima_obj_attr_set(ch_id, "y position",y_pos);
		}
		if(node_type==sensor)
		{
		//there are two ways to set user id, we use the second one. The first one is commented, which
		//is based on initial userId and counter
		
			//set user id according to counter and initial_userId
			//user_id = initial_userId+counter;
			
			//set user id according to node name
			op_ima_obj_attr_get(ch_id, "name",node_name);
			user_id = atoi(node_name);
			
			//set user id 
			op_ima_obj_attr_set(ch_id, "user id",user_id);

			//set next hop
			if(user_id - chain_number <= 0)//the next hop is the sink node
			{
				op_ima_obj_attr_set(ch_id, "Next Hop",-1);
			}else
			{
				op_ima_obj_attr_set(ch_id, "Next Hop",user_id - chain_number);
			}
			
			counter++;
		
			//fscanf(in,"%lf,%lf ",&x_pos,&y_pos);
			field_list_ptr=op_prg_str_decomp(op_prg_list_access(coordinate_list_ptr,num_rows),",");
			num_rows++;
			x_pos = atof(op_prg_list_access (field_list_ptr, 0));
			y_pos = atof(op_prg_list_access (field_list_ptr, 1));
			op_ima_obj_attr_set(ch_id, "x position",x_pos);
			op_ima_obj_attr_set(ch_id, "y position",y_pos);
			op_prg_list_free (field_list_ptr); op_prg_mem_free (field_list_ptr);
		}
	}
	op_prg_list_free (coordinate_list_ptr); op_prg_mem_free (coordinate_list_ptr);
/*	
	fclose(in);
	fclose(sink_in);
*/	
//out
	FRET(OPC_TRUE);
}

static void appoint_sources(void)
{
//var
	int i,list_size,num_temp,node_type;
	int random,id_in_list;
	int* user_id_pt;
	List  *userId_list;
	Objid ch_id;
//in
	FIN(appoint_sources(void));
//body
	userId_list = op_prg_list_create();
	num_temp=src_num;
	
	while(num_temp>0)//find src_num user_id
	{
		random = op_dist_outcome(op_dist_load("uniform_int",0,num_nodes-1));
		printf("随机产生的random=%d.\n",random);
		
		op_ima_obj_attr_get(op_topo_object(OPC_OBJTYPE_NDMOB, random), "Node Type",&node_type);
		if(node_type==sink)
		{
			random=-1;
		}else
		{
			list_size = op_prg_list_size(userId_list);
			for(i=0;i<list_size;i++)
			{
				id_in_list = *(int*)(op_prg_list_access(userId_list,i));
				printf("从list中取出的整数是:%d.\n",id_in_list);
				if(random==id_in_list)
				{
					random=-1;
					break;
				}
			}
		}
		
		if(random>=0)
		{
			user_id_pt = (int*)op_prg_mem_alloc(sizeof(int));
			(*user_id_pt)=random;
			op_prg_list_insert(userId_list,user_id_pt,OPC_LISTPOS_TAIL);
			--num_temp;
			printf("已将产生的userid插入list.\n");
		}
	}//end while
	
	list_size = op_prg_list_size(userId_list);
	for(i=0;i<list_size;i++)
	{
		user_id_pt = (int*)op_prg_list_access(userId_list,i);
		ch_id = op_topo_object(OPC_OBJTYPE_NDMOB, *user_id_pt);
		//op_ima_obj_attr_set(op_id_from_name(ch_id,OPC_OBJTYPE_PROC,module_name),"Send DATA",OPC_TRUE);
		op_intrpt_schedule_remote(op_sim_time()+(appoint_interval/src_num)*i,SEND_DATA_R_CODE,op_id_from_name(ch_id,OPC_OBJTYPE_PROC,module_name));
		printf("Have set Send DATA and the remote interruption. Time=%lf.\n",(appoint_interval/src_num)*i);
	}
	
//out
	op_prg_list_free(userId_list);
	op_prg_mem_free(userId_list);

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
	void nodes_deployment (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_nodes_deployment_init (int * init_block_ptr);
	void _op_nodes_deployment_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_nodes_deployment_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_nodes_deployment_alloc (VosT_Obtype, int);
	void _op_nodes_deployment_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
nodes_deployment (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (nodes_deployment ());

		{
		/* Temporary Variables */
		//int i;
		//double x,y;
		//int int_tv;
		//FILE *in;
		//char file_name_tv[300];
		/* End of Temporary Variables */


		FSM_ENTER ("nodes_deployment")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "nodes_deployment [init enter execs]")
				FSM_PROFILE_SECTION_IN ("nodes_deployment [init enter execs]", state0_enter_exec)
				{
				// Obtain related ID
				process_id = op_id_self();
				node_id = op_topo_parent(process_id);
				network_id = op_topo_parent(node_id);
				
				op_ima_obj_attr_get(process_id, "Num of Sources",&src_num);
				op_ima_obj_attr_get(process_id, "Start Appoint Time",&start_appoint);
				op_ima_obj_attr_get(process_id, "Appoint Src Interval",&appoint_interval);
				op_ima_obj_attr_get(process_id, "Initial userid of Sensor",&initial_userId);
				//op_ima_obj_attr_get(process_id, "sink userid",&sink_userid);
				op_ima_obj_attr_get(process_id, "Src Module Name", module_name);
				op_ima_obj_attr_get(process_id, "Appoint ON/OFF",&appoint_on_off);
				op_ima_obj_attr_get(process_id, "Traffic Load",&traffic_load);
				op_ima_obj_attr_get(process_id, "Chain Number", &chain_number);
				
				//Get the number of nodes
				num_nodes = op_topo_object_count (OPC_OBJTYPE_NDMOB);
				
				if(traffic_load<0)
				{
					infinity = OPC_TRUE;
				}else
				{
					infinity = OPC_FALSE;
				}
				
				printf("num_nodes=%d.\n",num_nodes);
				if(load_coordinate_file())
				{
					printf("Loading coordinate file is compeleted.\n");
				}else
				{
					op_sim_end("载入坐标文件失败！","","","");
				}
				
				
				
				//appoint_sources();
				
				if(appoint_on_off)
				{	
					printf("In Deployments_Admin, appoint_on_off==Enable.\n");
					if(traffic_load>0)
					{	
						op_intrpt_schedule_self(op_sim_time() + start_appoint, START_APPOINT_CODE);
						--traffic_load;
					}
					if(traffic_load<0)
					{
						op_intrpt_schedule_self(op_sim_time() + start_appoint, START_APPOINT_CODE);
					}
				}
				
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "nodes_deployment [init exit execs]")


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_3", "nodes_deployment [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "nodes_deployment [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"nodes_deployment")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "nodes_deployment [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("nodes_deployment [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (START_APPOINT)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "START_APPOINT", "", "idle", "Appoint Src", "tr_8", "nodes_deployment [idle -> Appoint Src : START_APPOINT / ]")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, ;, "default", "", "idle", "idle", "tr_2", "nodes_deployment [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (Appoint Src) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "Appoint Src", state2_enter_exec, "nodes_deployment [Appoint Src enter execs]")
				FSM_PROFILE_SECTION_IN ("nodes_deployment [Appoint Src enter execs]", state2_enter_exec)
				{
				appoint_sources();
				printf("Appoint src by Admin!\n");
				if(appoint_interval>0)
				{
					if(!infinity && traffic_load>0)
					{
						op_intrpt_schedule_self(op_sim_time() + appoint_interval, START_APPOINT_CODE);
						--traffic_load;
					}
					if(infinity)
					{
						op_intrpt_schedule_self(op_sim_time() + appoint_interval, START_APPOINT_CODE);
					}
				}
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (Appoint Src) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "Appoint Src", "nodes_deployment [Appoint Src exit execs]")


			/** state (Appoint Src) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "Appoint Src", "idle", "tr_9", "nodes_deployment [Appoint Src -> idle : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"nodes_deployment")
		}
	}




void
_op_nodes_deployment_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_nodes_deployment_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_nodes_deployment_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_nodes_deployment_svar function. */
#undef process_id
#undef node_id
#undef network_id
#undef num_nodes
#undef src_num
#undef start_appoint
#undef appoint_interval
#undef initial_userId
#undef module_name
#undef appoint_on_off
#undef traffic_load
#undef infinity
#undef node_name
#undef chain_number

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_nodes_deployment_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_nodes_deployment_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (nodes_deployment)",
		sizeof (nodes_deployment_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_nodes_deployment_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	nodes_deployment_state * ptr;
	FIN_MT (_op_nodes_deployment_alloc (obtype))

	ptr = (nodes_deployment_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "nodes_deployment [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_nodes_deployment_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	nodes_deployment_state		*prs_ptr;

	FIN_MT (_op_nodes_deployment_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (nodes_deployment_state *)gen_ptr;

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
	if (strcmp ("network_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->network_id);
		FOUT
		}
	if (strcmp ("num_nodes" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->num_nodes);
		FOUT
		}
	if (strcmp ("src_num" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->src_num);
		FOUT
		}
	if (strcmp ("start_appoint" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->start_appoint);
		FOUT
		}
	if (strcmp ("appoint_interval" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->appoint_interval);
		FOUT
		}
	if (strcmp ("initial_userId" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->initial_userId);
		FOUT
		}
	if (strcmp ("module_name" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->module_name);
		FOUT
		}
	if (strcmp ("appoint_on_off" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->appoint_on_off);
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
	if (strcmp ("node_name" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->node_name);
		FOUT
		}
	if (strcmp ("chain_number" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->chain_number);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

