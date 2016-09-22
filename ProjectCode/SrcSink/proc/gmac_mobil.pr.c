/* Process model C form file: gmac_mobil.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char gmac_mobil_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 4C8C4182 4C8C4182 1 china-0f9728557 Administrator 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                            ";
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

//Define queue type
#define grade_queue		0
#define data_queue		1

//Define stream in-out NO.
#define SRC_STRM 		1
#define RCV_STRM 		0
#define SEND_STRM 		0
#define DISCARD_STRM 	1

//Define node state
/*
#define initial			0
#define waking			1
#define awake			2//go to sending state
#define sleeping		3
#define asleep			4
*/

//Define self-interrupt code and transition condition
#define SEND_GM_CODE					6000
#define START_SCHEDULE_CODE				7000
#define TO_WAKE_UP_CODE					8000
#define WAKING_TO_SEND_RTS_CODE			9000
#define SEND_RTS_CODE					10000
#define SEND_DATA_CODE					20000
#define SEND_CTS_CODE					30000
#define SEND_ACK_CODE					40000
#define SLEEP_FROM_SECOND_CODE 			50000
#define SLEEP_FROM_FIRST_CODE			60000

#define END	        		    	(op_intrpt_type() == OPC_INTRPT_ENDSIM)
#define FROM_LOWER					((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == RCV_STRM))
#define FROM_UPPER					((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == SRC_STRM))
#define SEND_GM                 	((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_GM_CODE))

#define START_SCHEDULE				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == START_SCHEDULE_CODE))
#define WAKING_TO_WAIT_RTS			(START_SCHEDULE && grade>0 && grade%(sleep_factor+2)==1)
#define CONTEND_TO_SEND_RTS			(START_SCHEDULE && grade>0 && grade%(sleep_factor+2)==2)
#define GO_TO_SLEEP					(START_SCHEDULE && grade>0 && (grade%(sleep_factor+2)>=3||grade%(sleep_factor+2)==0))

#define WAKING_UP					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == TO_WAKE_UP_CODE))
#define WAKING_TO_SEND_RTS			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == WAKING_TO_SEND_RTS_CODE))
#define SEND_RTS					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_RTS_CODE))
#define SEND_DATA					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_DATA_CODE))
#define SEND_CTS					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_CTS_CODE))
#define SEND_ACK					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_ACK_CODE))
#define SLEEP_FROM_SECOND			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SLEEP_FROM_SECOND_CODE))
#define SLEEP_FROM_FIRST			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SLEEP_FROM_FIRST_CODE))
#define CHECK_CHANNEL				((op_intrpt_type() == OPC_INTRPT_STAT))

//function prototype
static void grade_pk_proc(Packet * pk);
static void upper_data_pk_proc(void);
static void power_stat(void);
static void record_grade(void);

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
	int	                    		grade                                           ;
	double	                 		send_gm_timer                                   ;
	double	                 		start_schedule_timer                            ;
	int	                    		sleep_factor                                    ;
	Evhandle	               		send_gm_event                                   ;
	double	                 		CW                                              ;	/* Contention Window */
	double	                 		DIFS                                            ;
	double	                 		SIFS                                            ;
	double	                 		durRTS                                          ;
	double	                 		durCTS                                          ;
	double	                 		durDATA                                         ;
	double	                 		durACK                                          ;
	double	                 		durSleep                                        ;
	double	                 		durActive_half                                  ;
	Evhandle	               		wait_RTS_event                                  ;
	Boolean	                		transceiver_state                               ;
	Objid	                  		rx_id                                           ;
	double	                 		waking_time                                     ;
	Evhandle	               		send_RTS_event                                  ;
	double	                 		second_contending_time                          ;
	Evhandle	               		wait_CTS_event                                  ;
	Evhandle	               		wait_ACK_event                                  ;
	int	                    		next_hop_1                                      ;
	int	                    		next_hop_2                                      ;
	Evhandle	               		send_CTS_event                                  ;
	Evhandle	               		wait_DATA_event                                 ;
	double	                 		record_time                                     ;
	double	                 		all_sleeped_time                                ;
	double	                 		all_active_time                                 ;
	double	                 		all_rx_time                                     ;
	double	                 		all_tx_time                                     ;
	} gmac_mobil_state;

#define process_id              		op_sv_ptr->process_id
#define node_id                 		op_sv_ptr->node_id
#define node_address            		op_sv_ptr->node_address
#define grade                   		op_sv_ptr->grade
#define send_gm_timer           		op_sv_ptr->send_gm_timer
#define start_schedule_timer    		op_sv_ptr->start_schedule_timer
#define sleep_factor            		op_sv_ptr->sleep_factor
#define send_gm_event           		op_sv_ptr->send_gm_event
#define CW                      		op_sv_ptr->CW
#define DIFS                    		op_sv_ptr->DIFS
#define SIFS                    		op_sv_ptr->SIFS
#define durRTS                  		op_sv_ptr->durRTS
#define durCTS                  		op_sv_ptr->durCTS
#define durDATA                 		op_sv_ptr->durDATA
#define durACK                  		op_sv_ptr->durACK
#define durSleep                		op_sv_ptr->durSleep
#define durActive_half          		op_sv_ptr->durActive_half
#define wait_RTS_event          		op_sv_ptr->wait_RTS_event
#define transceiver_state       		op_sv_ptr->transceiver_state
#define rx_id                   		op_sv_ptr->rx_id
#define waking_time             		op_sv_ptr->waking_time
#define send_RTS_event          		op_sv_ptr->send_RTS_event
#define second_contending_time  		op_sv_ptr->second_contending_time
#define wait_CTS_event          		op_sv_ptr->wait_CTS_event
#define wait_ACK_event          		op_sv_ptr->wait_ACK_event
#define next_hop_1              		op_sv_ptr->next_hop_1
#define next_hop_2              		op_sv_ptr->next_hop_2
#define send_CTS_event          		op_sv_ptr->send_CTS_event
#define wait_DATA_event         		op_sv_ptr->wait_DATA_event
#define record_time             		op_sv_ptr->record_time
#define all_sleeped_time        		op_sv_ptr->all_sleeped_time
#define all_active_time         		op_sv_ptr->all_active_time
#define all_rx_time             		op_sv_ptr->all_rx_time
#define all_tx_time             		op_sv_ptr->all_tx_time

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	gmac_mobil_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((gmac_mobil_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif


//Deal with grade pk
static void
grade_pk_proc(Packet * pk)
{
//var
	int grade_in_pk;//,sink_id_in_pk;
//in
	FIN(grade_pk_proc(pk));
//body
	op_pk_nfd_get(pk,"Grade",&grade_in_pk);
	//op_pk_nfd_get(pk,"Sink ID",&sink_id_in_pk);
	
	if(grade<0)
	{
		grade = grade_in_pk;
		//sink_id = sink_id_in_pk;
		
		++grade_in_pk;
		op_pk_nfd_set(pk,"Grade",grade_in_pk);
		
		//cancel the previous event if pending
		if(op_ev_valid(send_gm_event))
		{
			op_ev_cancel(send_gm_event);
		}
		//at the same time, clear grade_queue
		if(op_subq_empty(grade_queue) == OPC_FALSE)
		{
			op_subq_flush(grade_queue);
		}
		
		op_subq_pk_insert (grade_queue,pk,OPC_QPOS_TAIL);
		send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer), SEND_GM_CODE);
		
		FOUT;
	}
	
	if(grade_in_pk < grade)
	{
		grade = grade_in_pk;
		//if(sink_id != sink_id_in_pk)
		//{
		//	sink_id = sink_id_in_pk;
		//}
			
		++grade_in_pk;
		op_pk_nfd_set(pk,"Grade",grade_in_pk);
		
		//cancel the previous event if pending
		if(op_ev_valid(send_gm_event))
		{
			op_ev_cancel(send_gm_event);
		}
		//at the same time, clear grade_queue
		if(op_subq_empty(grade_queue) == OPC_FALSE)
		{
			op_subq_flush(grade_queue);
		}
		
		op_subq_pk_insert (grade_queue,pk,OPC_QPOS_TAIL);
		send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer), SEND_GM_CODE);
		
		FOUT;
	}
	
	op_pk_destroy(pk);
	
//out
	FOUT;
}

//Received DATA from upper layer
static void 
upper_data_pk_proc(void)
{
//var
	Packet* upper_pk;
//in
	FIN(upper_data_pk_proc());
//body
	upper_pk = op_pk_get(SRC_STRM);
	op_pk_nfd_set(upper_pk,"Type",data_pk);
	op_pk_nfd_set(upper_pk,"Src",node_address);
	op_pk_nfd_set(upper_pk,"Grade",grade);
	
	op_subq_pk_insert (data_queue,upper_pk,OPC_QPOS_TAIL);
	printf("Have inserted the received DATA from \"gsrc\" into DATA queue\n");
//out
	FOUT;
}

//In "end" state, proceed power statistic
static void
power_stat(void)
{
//var
	FILE *in;
	char temp_file_name[300];
//in
	FIN(power_stat(void));
//vody
	op_ima_obj_attr_get(node_id,"Power Record File",temp_file_name);
	if(strcmp(temp_file_name,"disable")!=0)
	{	
		in = fopen(temp_file_name,"at");
		/*
		fprintf(in,"Begin->Node:%d\r\n",node_address);
		fprintf(in,"Sleep_time	Active_time	Tx_time		Rx_time		Idle_time	duty_cycle\r\n");
		fprintf(in,"%f	%f	%f	%f	%f	%f\r\n",\
			all_sleeped_time,all_active_time,all_tx_time,all_rx_time,all_active_time-(all_tx_time+all_rx_time),all_active_time/(all_active_time+all_sleeped_time));
		
		//fprintf(in,"CE\r\n");
		*/
		fprintf(in,"%f\r\n",all_sleeped_time*0.05+(all_active_time-(all_tx_time+all_rx_time))*0.45+(all_tx_time+all_rx_time)*0.5);
		
		//fprintf(in,"%f\r\n",all_sleeped_time*0.015+(all_active_time-all_tx_time)*13.5+all_tx_time*24.75);
		/*
		fprintf(in,"Simulation_time	CE	Average_power_consumed\r\n");
	
		fprintf(in,"%f	%f	%f\r\n",op_sim_time(),\
				all_sleeped_time*0.05+(all_active_time-(all_tx_time+all_rx_time))*0.45+(all_tx_time+all_rx_time)*0.5,\
				(all_sleeped_time*0.05+(all_active_time-(all_tx_time+all_rx_time))*0.45+(all_tx_time+all_rx_time)*0.5)/(all_sleeped_time+all_active_time));
		
		fprintf(in,"End\r\n\r\n");
		*/
		fclose(in);
	}	
//out
	FOUT;
}

static void record_grade(void)
{
//var
	FILE *file_in;
	char file_name[300];
//in
	FIN(record_grade(void));
//body
	op_ima_obj_attr_get(node_id,"Grade Record File",file_name);
	
	file_in = fopen(file_name,"at");
	fprintf(file_in,"节点：%d, 其级别为：%d\r\n",\
		node_address,grade);
	fclose(file_in);
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
	void gmac_mobil (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_gmac_mobil_init (int * init_block_ptr);
	void _op_gmac_mobil_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_gmac_mobil_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_gmac_mobil_alloc (VosT_Obtype, int);
	void _op_gmac_mobil_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
gmac_mobil (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (gmac_mobil ());

		{
		/* Temporary Variables */
		Packet* pk_TV=NULL;
		int pk_type;
		double time;
		int dest_TV;
		int grade_TV;
		int hop_num_TV;
		//File
		FILE *in;
		char temp_file_name[300];
		//char message[100];
		/* End of Temporary Variables */


		FSM_ENTER ("gmac_mobil")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "gmac_mobil [init enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [init enter execs]", state0_enter_exec)
				{
				// Obtain related ID
				process_id = op_id_self();
				node_id = op_topo_parent(process_id);
				rx_id = op_id_from_name(node_id,OPC_OBJTYPE_RARX,"rx");
					
				//obtain parameter
				//op_ima_obj_attr_get(process_id, "Start Grade Timer", &start_grade_timer);		//1
				op_ima_obj_attr_get(process_id, "Send GM Timer", &send_gm_timer);				//2
				op_ima_obj_attr_get(process_id, "Start Schedule Timer", &start_schedule_timer);	//3
				op_ima_obj_attr_get(process_id, "Sleep Factor", &sleep_factor);					//4
				op_ima_obj_attr_get(process_id, "CW",&CW);										//5
				op_ima_obj_attr_get(process_id, "durRTS",&durRTS);								//6
				op_ima_obj_attr_get(process_id, "durCTS",&durCTS);								//7
				op_ima_obj_attr_get(process_id, "durDATA",&durDATA);							//8
				op_ima_obj_attr_get(process_id, "durACK",&durACK);								//9
				op_ima_obj_attr_get(process_id, "DIFS",&DIFS);									//10
				op_ima_obj_attr_get(process_id, "SIFS",&SIFS);									//11
				
				op_ima_obj_attr_get(node_id, "user id", &node_address);
				op_ima_obj_attr_get(rx_id, "state", &transceiver_state);
				
				grade = -1;
				//sink_id = -1;
				
				all_sleeped_time = 0;
				all_active_time = 0;
				all_tx_time = 0;
				all_rx_time = 0;
				
				//durActive contains two parts: durReceive & durSedn, and durReceive==durSend
				durActive_half = 2*CW+durRTS+durCTS+durDATA+durACK+2*DIFS+2*SIFS;
				durSleep = sleep_factor * durActive_half;
				
				op_intrpt_schedule_self(op_sim_time() + start_schedule_timer, START_SCHEDULE_CODE);
				
				//op_ima_obj_attr_get (process_id, "subqueue", &subq_objid);
				//num_subqs = op_topo_child_count (subq_objid, OPC_OBJTYPE_SUBQ);
				//printf("子队列个数:%d\n",num_subqs);
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "gmac_mobil [init exit execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [init exit execs]", state0_exit_exec)
				{
				/*
				op_ima_obj_attr_get(node_id,"Grade Record File",temp_file_name);
				remove(temp_file_name);
				op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
				remove(temp_file_name);
				*/
				if(node_address == 3)
				{
					op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
					in = fopen(temp_file_name,"at");
					
					fprintf(in,"Begin:\r\ndurActive_half: %f, durSleep: %f.(in \"gmac->init\")\r\n",\
						durActive_half,durSleep);
					fprintf(in,"duty cycle:%f.\r\n",\
							durActive_half*2/(durActive_half*2+durSleep));
					fclose(in);
				}
				/*
				if(strcmp(temp_file_name,"disable")!=0)
				{	
					printf("Node:%d, 此节点可产生记录.\n",node_address);
					in = fopen(temp_file_name,"at");
					fprintf(in,"Simulation_time		Average_power_consumed\r\n");
					fclose(in);
				}
				*/
				}
				FSM_PROFILE_SECTION_OUT (state0_exit_exec)


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_1", "gmac_mobil [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "gmac_mobil [idle enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [idle enter execs]", state1_enter_exec)
				{
				if(END)
				{
				/*
					op_ima_obj_attr_get(node_id,"Grade Record File",temp_file_name);
					
					in = fopen(temp_file_name,"at");
					fprintf(in,"节点：%d, 其级别为：%d\r\n",\
						node_address,grade);
					fclose(in);
				*/
					record_grade();
				}
				
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					printf("In \"idle\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				}
				FSM_PROFILE_SECTION_OUT (state1_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"gmac_mobil")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "gmac_mobil [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("gmac_mobil [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (FROM_LOWER && transceiver_state)
			FSM_TEST_COND (SEND_GM)
			FSM_TEST_COND (CONTEND_TO_SEND_RTS)
			FSM_TEST_COND (GO_TO_SLEEP)
			FSM_TEST_COND (WAKING_TO_WAIT_RTS)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "idle", "GM proc", "tr_5", "gmac_mobil [idle -> GM proc : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "SEND_GM", "", "idle", "send gm", "tr_7", "gmac_mobil [idle -> send gm : SEND_GM / ]")
				FSM_CASE_TRANSIT (2, 6, state6_enter_exec, ;, "CONTEND_TO_SEND_RTS", "", "idle", "second contention", "tr_28", "gmac_mobil [idle -> second contention : CONTEND_TO_SEND_RTS / ]")
				FSM_CASE_TRANSIT (3, 4, state4_enter_exec, ;, "GO_TO_SLEEP", "", "idle", "sleeping", "tr_32", "gmac_mobil [idle -> sleeping : GO_TO_SLEEP / ]")
				FSM_CASE_TRANSIT (4, 5, state5_enter_exec, ;, "WAKING_TO_WAIT_RTS", "", "idle", "waking", "tr_34", "gmac_mobil [idle -> waking : WAKING_TO_WAIT_RTS / ]")
				FSM_CASE_TRANSIT (5, 1, state1_enter_exec, ;, "default", "", "idle", "idle", "tr_88", "gmac_mobil [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (GM proc) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "GM proc", state2_enter_exec, "gmac_mobil [GM proc enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [GM proc enter execs]", state2_enter_exec)
				{
				pk_TV = op_pk_get(RCV_STRM);
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				if(pk_type == grade_pk)
				{
					grade_pk_proc(pk_TV);
				}
				else
				{
					op_pk_destroy(pk_TV);
					op_sim_end("Waiting Grade Message Err: Are you kidding me?","I'm waiting Grade Message","Not you","你个傻×");
				
				}
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (GM proc) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "GM proc", "gmac_mobil [GM proc exit execs]")


			/** state (GM proc) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "GM proc", "idle", "tr_6", "gmac_mobil [GM proc -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (send gm) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "send gm", state3_enter_exec, "gmac_mobil [send gm enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [send gm enter execs]", state3_enter_exec)
				{
				if(op_subq_empty(grade_queue) == OPC_FALSE)
				{
					pk_TV = op_subq_pk_remove(grade_queue, OPC_QPOS_HEAD);
					op_pk_send(pk_TV,SEND_STRM);
				}
				else
				{
					op_sim_end("Err: grade queue is empty, simulation is over!","","","");
				}
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (send gm) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "send gm", "gmac_mobil [send gm exit execs]")


			/** state (send gm) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "send gm", "idle", "tr_87", "gmac_mobil [send gm -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (sleeping) enter executives **/
			FSM_STATE_ENTER_UNFORCED (4, "sleeping", state4_enter_exec, "gmac_mobil [sleeping enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [sleeping enter execs]", state4_enter_exec)
				{
				if(op_intrpt_code() == START_SCHEDULE_CODE)// from "GO_TO_SLEEP"
				{	
					if(grade % (sleep_factor+2)>=3)
					{
						time = durSleep - ((grade%(sleep_factor+2))-3)*durActive_half;			
					}
					else if(grade % (sleep_factor+2)==0)
					{
						time = durSleep - ((sleep_factor+2)-3)*durActive_half;
					}
					else
					{
						op_sim_end("仿真出错","grade%(sleep_factor+2)不大于3或0","请反思","笨蛋、傻×");
					}
					/*
					op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
					in = fopen(temp_file_name,"at");
					fprintf(in,"Node:%d, Grade:%d, Grade%%(%d+2): %d, time:%f, enter into \"sleeping\"state, been \"asleep\" for: %f.\r\n",\
						node_address,grade,sleep_factor,grade%(sleep_factor+2),op_sim_time(),durSleep-time);
					fclose(in);
					*/
					//set timer,when wake up, go to "waking" state
					op_intrpt_schedule_self(op_sim_time() + time, TO_WAKE_UP_CODE);
					
					printf("Node:%d, Grade:%d, time:%f,in sleeping period.\n\
							Initial to sleeping.\n\
							After %f seconds, go to wake up.\n",\
							node_address,grade,op_sim_time(),time);
					
					//time statistic
					record_time = op_sim_time();
				}
				else
				if(op_intrpt_code() == SLEEP_FROM_FIRST_CODE)//sleep from first period
				{
					//time = CW+durCTS+durDATA+durACK+3*SIFS;
					time = waking_time + durActive_half - op_sim_time();
						
					//when wake up, go to "second contention" state
					op_intrpt_schedule_self(op_sim_time() + time, WAKING_TO_SEND_RTS_CODE);
					
					printf("Node:%d, Grade:%d, time:%f,in sleeping period.\n\
							To sleeping from the first period.\n\
							After %f seconds, go to second period.\n",\
							node_address,grade,op_sim_time(),time);
					
					//time statistic
					all_active_time = all_active_time+(op_sim_time()-record_time);
					record_time = op_sim_time();
				}
				else
				if(op_intrpt_code() == SLEEP_FROM_SECOND_CODE)//sleep from second period
				{
					
					time = second_contending_time + durActive_half + durSleep - op_sim_time();
					
					//when wake up, go to "waking" state
					op_intrpt_schedule_self(op_sim_time() + time, TO_WAKE_UP_CODE);
					
					printf("Node:%d, Grade:%d, time:%f,in sleeping period.\n\
							To sleeping from second period.\n\
							After %f seconds, go to \"wakeing\" state.\n",\
							node_address,grade,op_sim_time(),time);
					
					//time statistic
					all_active_time = all_active_time+(op_sim_time()-record_time);
					record_time = op_sim_time();
				}
				
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					printf("In \"sleeping\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				
				if(FROM_LOWER && !transceiver_state)
				{	
					op_pk_destroy(op_pk_get(RCV_STRM));
					printf("Node:%d, time:%f. Node is sleeping, destroy the received pk.\n",\
							node_address,op_sim_time());
				}
				if(END)
				{
					all_sleeped_time = all_sleeped_time + (op_sim_time()-record_time);
					power_stat();
					record_grade();
				}
				//close transceiver
				transceiver_state = OPC_FALSE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,"gmac_mobil")


			/** state (sleeping) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "sleeping", "gmac_mobil [sleeping exit execs]")


			/** state (sleeping) transition processing **/
			FSM_PROFILE_SECTION_IN ("gmac_mobil [sleeping trans conditions]", state4_trans_conds)
			FSM_INIT_COND (WAKING_UP)
			FSM_TEST_COND (WAKING_TO_SEND_RTS)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("sleeping")
			FSM_PROFILE_SECTION_OUT (state4_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 5, state5_enter_exec, ;, "WAKING_UP", "", "sleeping", "waking", "tr_37", "gmac_mobil [sleeping -> waking : WAKING_UP / ]")
				FSM_CASE_TRANSIT (1, 6, state6_enter_exec, ;, "WAKING_TO_SEND_RTS", "", "sleeping", "second contention", "tr_40", "gmac_mobil [sleeping -> second contention : WAKING_TO_SEND_RTS / ]")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "default", "", "sleeping", "sleeping", "tr_91", "gmac_mobil [sleeping -> sleeping : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (waking) enter executives **/
			FSM_STATE_ENTER_FORCED (5, "waking", state5_enter_exec, "gmac_mobil [waking enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [waking enter execs]", state5_enter_exec)
				{
				
				//Log record
				if(WAKING_TO_WAIT_RTS)
				{
					/*
					op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
					in = fopen(temp_file_name,"at");
					fprintf(in,"Node:%d, Grade:%d, Grade%%4: %d, time:%f, enter into \"waking\" state.\r\n",\
						node_address,grade,grade%4, op_sim_time());
					fclose(in);
					*/
					record_time = op_sim_time();
				}	
				
				
				//enable transceiver
				transceiver_state = OPC_TRUE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				
				waking_time = op_sim_time();
				
				time = CW+DIFS+durRTS;
				wait_RTS_event = op_intrpt_schedule_self(op_sim_time() + time, SLEEP_FROM_FIRST_CODE);
				
				printf("Node:%d, Grade:%d, time:%f, entered into first period (\"waking\" state).\n\
					    Waiting RTS.\n\
						If no RTS, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				
				//time statistic
				all_sleeped_time = all_sleeped_time + (op_sim_time() - record_time);
				record_time = op_sim_time();
				}
				FSM_PROFILE_SECTION_OUT (state5_enter_exec)

			/** state (waking) exit executives **/
			FSM_STATE_EXIT_FORCED (5, "waking", "gmac_mobil [waking exit execs]")


			/** state (waking) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "waking", "first", "tr_57", "gmac_mobil [waking -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (second contention) enter executives **/
			FSM_STATE_ENTER_FORCED (6, "second contention", state6_enter_exec, "gmac_mobil [second contention enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [second contention enter execs]", state6_enter_exec)
				{
				
				//Log record
				if(CONTEND_TO_SEND_RTS)
				{
					/*
					op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
					in = fopen(temp_file_name,"at");
					fprintf(in,"Node:%d, Grade:%d, Grade%%4: %d, time:%f, enter into \"second contention\" state.\r\n",\
						node_address,grade,grade%4,op_sim_time());
					fclose(in);
					*/
					record_time = op_sim_time();
				}
				
				
				/*
				1. check data_queue
				   if(Y) open transceiver, contending for sending RTS
				   if(N) go to sleep
				*/
				
				//enable transceiver
				transceiver_state = OPC_TRUE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				
				
				second_contending_time = op_sim_time();
				if(op_subq_empty(data_queue) == OPC_FALSE)//data queue is not empty
				{
					time = op_dist_uniform(CW) + DIFS;
					send_RTS_event = op_intrpt_schedule_self(op_sim_time()+time,SEND_RTS_CODE);
					
					printf("Node:%d, Grade:%d, time:%f, in second period.\n\
							data_queue is not empty, Contending to send RTS.\n\
							Waiting to send RTS.\n",\
							node_address,grade,op_sim_time());
					//sprintf(message,"Contend to send RTS, op_dist_uniform(CW)=%f\n",time-DIFS);
					//op_sim_message("",message);
				}
				else//data queue is empty, go to sleep
				{
					op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
					
					printf("Node:%d, Grade:%d, time:%f, in second period.\n\
							data_queue is empty.\n\
							Go to sleep immediately.\n",\
							node_address,grade,op_sim_time());
				}
				
				//time statistic
				all_sleeped_time = all_sleeped_time + (op_sim_time()-record_time);
				record_time = op_sim_time();
				}
				FSM_PROFILE_SECTION_OUT (state6_enter_exec)

			/** state (second contention) exit executives **/
			FSM_STATE_EXIT_FORCED (6, "second contention", "gmac_mobil [second contention exit execs]")


			/** state (second contention) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "second contention", "second", "tr_59", "gmac_mobil [second contention -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (wait CTS) enter executives **/
			FSM_STATE_ENTER_FORCED (7, "wait CTS", state7_enter_exec, "gmac_mobil [wait CTS enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [wait CTS enter execs]", state7_enter_exec)
				{
				//send RTS
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_create_fmt("MAC_RTS_PK");
				op_pk_nfd_set(pk_TV, "Type", rts_pk);
				op_pk_nfd_set(pk_TV,"Src", node_address);
				op_pk_nfd_set(pk_TV,"Grade",grade);
				
				op_pk_send(pk_TV, SEND_STRM);
				
				//time statistic
				all_tx_time = all_tx_time + durRTS;
				
				//set timer for waiting CTS from lower grade nodes
				time = durRTS+CW+DIFS+durCTS;
				wait_CTS_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_SECOND_CODE);
				
				printf("Node:%d, Grade:%d, time:%f, in second period.\n\
						Have sent RTS.\n\
						Waiting to receive CTS.\n\
						If no CTS, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state7_enter_exec)

			/** state (wait CTS) exit executives **/
			FSM_STATE_EXIT_FORCED (7, "wait CTS", "gmac_mobil [wait CTS exit execs]")


			/** state (wait CTS) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "wait CTS", "second", "tr_89", "gmac_mobil [wait CTS -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (first) enter executives **/
			FSM_STATE_ENTER_UNFORCED (8, "first", state8_enter_exec, "gmac_mobil [first enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [first enter execs]", state8_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					printf("In \"first\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				
				if(END)
				{
					all_active_time = all_active_time+(op_sim_time()-record_time);
					power_stat();
					record_grade();
				}
				}
				FSM_PROFILE_SECTION_OUT (state8_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (17,"gmac_mobil")


			/** state (first) exit executives **/
			FSM_STATE_EXIT_UNFORCED (8, "first", "gmac_mobil [first exit execs]")


			/** state (first) transition processing **/
			FSM_PROFILE_SECTION_IN ("gmac_mobil [first trans conditions]", state8_trans_conds)
			FSM_INIT_COND (SLEEP_FROM_FIRST)
			FSM_TEST_COND (FROM_LOWER && transceiver_state)
			FSM_TEST_COND (SEND_CTS)
			FSM_TEST_COND (SEND_ACK)
			FSM_TEST_COND (CHECK_CHANNEL)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("first")
			FSM_PROFILE_SECTION_OUT (state8_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 4, state4_enter_exec, ;, "SLEEP_FROM_FIRST", "", "first", "sleeping", "tr_56", "gmac_mobil [first -> sleeping : SLEEP_FROM_FIRST / ]")
				FSM_CASE_TRANSIT (1, 12, state12_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "first", "RCV(1)", "tr_74", "gmac_mobil [first -> RCV(1) : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (2, 13, state13_enter_exec, ;, "SEND_CTS", "", "first", "wait DATA", "tr_76", "gmac_mobil [first -> wait DATA : SEND_CTS / ]")
				FSM_CASE_TRANSIT (3, 14, state14_enter_exec, ;, "SEND_ACK", "", "first", "send ACK", "tr_79", "gmac_mobil [first -> send ACK : SEND_ACK / ]")
				FSM_CASE_TRANSIT (4, 15, state15_enter_exec, ;, "CHECK_CHANNEL", "", "first", "carrier sense(1)", "tr_93", "gmac_mobil [first -> carrier sense(1) : CHECK_CHANNEL / ]")
				FSM_CASE_TRANSIT (5, 8, state8_enter_exec, ;, "default", "", "first", "first", "tr_90", "gmac_mobil [first -> first : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (second) enter executives **/
			FSM_STATE_ENTER_UNFORCED (9, "second", state9_enter_exec, "gmac_mobil [second enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [second enter execs]", state9_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					printf("In \"second\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				if(END)
				{
					all_active_time = all_active_time + (op_sim_time()-record_time);
					power_stat();
					record_grade();
				}
				}
				FSM_PROFILE_SECTION_OUT (state9_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (19,"gmac_mobil")


			/** state (second) exit executives **/
			FSM_STATE_EXIT_UNFORCED (9, "second", "gmac_mobil [second exit execs]")


			/** state (second) transition processing **/
			FSM_PROFILE_SECTION_IN ("gmac_mobil [second trans conditions]", state9_trans_conds)
			FSM_INIT_COND (SEND_RTS)
			FSM_TEST_COND (SLEEP_FROM_SECOND)
			FSM_TEST_COND (FROM_LOWER && transceiver_state)
			FSM_TEST_COND (SEND_DATA)
			FSM_TEST_COND (CHECK_CHANNEL)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("second")
			FSM_PROFILE_SECTION_OUT (state9_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 7, state7_enter_exec, ;, "SEND_RTS", "", "second", "wait CTS", "tr_60", "gmac_mobil [second -> wait CTS : SEND_RTS / ]")
				FSM_CASE_TRANSIT (1, 4, state4_enter_exec, ;, "SLEEP_FROM_SECOND", "", "second", "sleeping", "tr_61", "gmac_mobil [second -> sleeping : SLEEP_FROM_SECOND / ]")
				FSM_CASE_TRANSIT (2, 10, state10_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "second", "RCV(2)", "tr_66", "gmac_mobil [second -> RCV(2) : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (3, 11, state11_enter_exec, ;, "SEND_DATA", "", "second", "send data", "tr_72", "gmac_mobil [second -> send data : SEND_DATA / ]")
				FSM_CASE_TRANSIT (4, 16, state16_enter_exec, ;, "CHECK_CHANNEL", "", "second", "carrier sense(2)", "tr_95", "gmac_mobil [second -> carrier sense(2) : CHECK_CHANNEL / ]")
				FSM_CASE_TRANSIT (5, 9, state9_enter_exec, ;, "default", "", "second", "second", "tr_92", "gmac_mobil [second -> second : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (RCV(2)) enter executives **/
			FSM_STATE_ENTER_FORCED (10, "RCV(2)", state10_enter_exec, "gmac_mobil [RCV(2) enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [RCV(2) enter execs]", state10_enter_exec)
				{
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_get(RCV_STRM);
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				
				if(pk_type == rts_pk)
				{
					//time statistic
					all_rx_time = all_rx_time + durRTS;
					
					//RTS listening during contending to send RTS("uniform(CW)+durDIFS")
					if(op_ev_valid(send_RTS_event))
					{
						op_ev_cancel(send_RTS_event);
						op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
						
						printf("Node:%d, Grade:%d, time:%f, in second period.\n\
								Failure to send RTS, because received RTS from other nodes.\n\
								Go to sleep immediately.\n",\
								node_address,grade,op_sim_time());
					}
				}
				
				if(pk_type == cts_pk)
				{
					//time statistic
					all_rx_time = all_rx_time + durCTS;
					
					op_pk_nfd_get(pk_TV,"Grade",&grade_TV);
					op_pk_nfd_get(pk_TV,"Dest",&dest_TV);
					if(grade == grade_TV+1 && dest_TV == node_address)
					{
						if(op_ev_valid(wait_CTS_event))//the first time to receive CTS
						{
							op_ev_cancel(wait_CTS_event);
							op_pk_nfd_get(pk_TV,"Src",&next_hop_2);
							
							//wait SIFS time to send DATA
							time = SIFS;
							op_intrpt_schedule_self(op_sim_time()+time,SEND_DATA_CODE);
							
							printf("Node:%d, Grade:%d, time:%f, in second period.\n\
									Successfully received CTS.\n\
									Waiting to send DATA.\n",\
									node_address,grade,op_sim_time());
						}
					}
				}
				
				if(pk_type == ack_pk)
				{
					//time statistic
					all_rx_time = all_rx_time + durACK;
					
					op_pk_nfd_get(pk_TV,"Dest",&dest_TV);
					if(node_address == dest_TV)
					{
						if(op_ev_valid(wait_ACK_event))
						{
							op_ev_cancel(wait_ACK_event);
							
							//Data has been sent correctly, remove it from temp_queue
							op_pk_destroy(op_subq_pk_remove(data_queue, OPC_QPOS_HEAD));
							//recieved ACK correctly, go to sleep immediately
							op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
							
							printf("Node:%d, Grade:%d, time:%f, in second period.\n\
									Successfully received ACK.\n\
									Go to sleep immediately.\n",\
									node_address,grade,op_sim_time());
						}
					}
				}
				
				op_pk_destroy(pk_TV);
				}
				FSM_PROFILE_SECTION_OUT (state10_enter_exec)

			/** state (RCV(2)) exit executives **/
			FSM_STATE_EXIT_FORCED (10, "RCV(2)", "gmac_mobil [RCV(2) exit execs]")


			/** state (RCV(2)) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "RCV(2)", "second", "tr_71", "gmac_mobil [RCV(2) -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (send data) enter executives **/
			FSM_STATE_ENTER_FORCED (11, "send data", state11_enter_exec, "gmac_mobil [send data enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [send data enter execs]", state11_enter_exec)
				{
				//send DATA
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				//obtain DATA from data_queue
				pk_TV = op_pk_copy(op_subq_pk_access(data_queue, OPC_QPOS_HEAD));
				op_pk_creation_time_set (pk_TV,op_pk_creation_time_get (op_subq_pk_access(data_queue, OPC_QPOS_HEAD)));
					
				op_pk_nfd_set(pk_TV,"Previous Hop",node_address);
				op_pk_nfd_set(pk_TV,"Next Hop",next_hop_2);
				
				op_pk_send(pk_TV,SEND_STRM);
				
				//time statistic
				all_tx_time = all_tx_time + durDATA;
				
				//set timer for waiting ACK from lower grade node
				time = durDATA + SIFS + durACK;
				wait_ACK_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_SECOND_CODE);
				
				printf("Node:%d, Grade:%d, time:%f, in second period.\n\
						Have sent DATA, waiting ACK.\n\
						If no ACK, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state11_enter_exec)

			/** state (send data) exit executives **/
			FSM_STATE_EXIT_FORCED (11, "send data", "gmac_mobil [send data exit execs]")


			/** state (send data) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "send data", "second", "tr_73", "gmac_mobil [send data -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (RCV(1)) enter executives **/
			FSM_STATE_ENTER_FORCED (12, "RCV(1)", state12_enter_exec, "gmac_mobil [RCV(1) enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [RCV(1) enter execs]", state12_enter_exec)
				{
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_get(RCV_STRM);
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				
				if(pk_type == rts_pk)
				{
					//time statistic
					all_rx_time = all_rx_time + durRTS;
				
					//wait for RTS
					op_pk_nfd_get(pk_TV,"Grade",&grade_TV);
					if(grade_TV == grade+1)
					{
						if(op_ev_valid(wait_RTS_event))//the first time to receive RTS
						{
							op_ev_cancel(wait_RTS_event);
							op_pk_nfd_get(pk_TV,"Src",&next_hop_1);
							
							//prepare to send CTS
							time = op_dist_uniform(CW) + DIFS;
							send_CTS_event = op_intrpt_schedule_self(op_sim_time()+time,SEND_CTS_CODE);
							
							printf("Node:%d, Grade:%d, time:%f, in first period.\n\
									Successfully received RTS.\n\
								    Waiting to send CTS.\n",\
									node_address,grade,op_sim_time());
							//sprintf(message,"Contend to send CTS, op_dist_uniform(CW)=%f\n",time-DIFS);
							//op_sim_message("",message);
						}
					}
					op_pk_destroy(pk_TV);
				}
				else
				if(pk_type == cts_pk)
				{
					//time statistic
					all_rx_time = all_rx_time + durCTS;
					
					//CTS listening during contending to send CTS("uniform(CW)+DIFS")
					if(op_ev_valid(send_CTS_event))
					{
						op_ev_cancel(send_CTS_event);
						op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_FIRST_CODE);
						
						printf("Node:%d, Grade:%d, time:%f, in first period.\n\
								Failure to send CTS, because received CTS from other nodes.\n\
								Go to sleep immediately.\n",\
								node_address,grade,op_sim_time());
					}
					op_pk_destroy(pk_TV);
				}
				else
				if(pk_type == data_pk)
				{
					//time statistic
					all_rx_time = all_rx_time + durDATA;
					
					op_pk_nfd_get(pk_TV,"Next Hop",&dest_TV);
					if(dest_TV == node_address)
					{
						if(op_ev_valid(wait_DATA_event))
						{
							op_ev_cancel(wait_DATA_event);
							op_pk_nfd_get(pk_TV,"Previous Hop",&next_hop_1);
							
							op_pk_nfd_get(pk_TV,"Hop Num", &hop_num_TV);
							hop_num_TV++;
							op_pk_nfd_set(pk_TV,"Hop Num",hop_num_TV);
							
							//insert the DATA packet into data_queue
							op_subq_pk_insert (data_queue,pk_TV,OPC_QPOS_TAIL);
							
							//wait SIFS time to send ACK
							time = SIFS;
							op_intrpt_schedule_self(op_sim_time()+time,SEND_ACK_CODE);
							
							printf("Node:%d, Grade:%d, time:%f, in first period.\n\
									Successfully received DATA.\n\
									Waiting to reply ACK.\n",\
									node_address,grade,op_sim_time());
						}else
						{
							op_pk_destroy(pk_TV);
						}
					}else
					{
						op_pk_destroy(pk_TV);
					}
				}
				else
				//if(pk_type != data_pk)
				{	
					printf("Node:%d, Grade:%d, time:%f, in first period.\n\
							The received pk is not RTS, CTS, or DATA. The pk type is: %d.\n\
							Destroy this pk.\n",\
							node_address,grade,op_sim_time(),pk_type);
						
					op_pk_destroy(pk_TV);
				}	
				}
				FSM_PROFILE_SECTION_OUT (state12_enter_exec)

			/** state (RCV(1)) exit executives **/
			FSM_STATE_EXIT_FORCED (12, "RCV(1)", "gmac_mobil [RCV(1) exit execs]")


			/** state (RCV(1)) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "RCV(1)", "first", "tr_75", "gmac_mobil [RCV(1) -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (wait DATA) enter executives **/
			FSM_STATE_ENTER_FORCED (13, "wait DATA", state13_enter_exec, "gmac_mobil [wait DATA enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [wait DATA enter execs]", state13_enter_exec)
				{
				//send CTS
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_create_fmt("MAC_CTS_PK");
				op_pk_nfd_set(pk_TV,"Type", cts_pk);
				op_pk_nfd_set(pk_TV,"Src",node_address);
				op_pk_nfd_set(pk_TV,"Dest",next_hop_1);
				op_pk_nfd_set(pk_TV,"Grade",grade);
				
				op_pk_send(pk_TV, SEND_STRM);
				
				//time statistic
				all_tx_time = all_tx_time + durCTS;
				
				//set timer for waiting DATA from upper grade node
				time = durCTS + SIFS +durDATA;
				wait_DATA_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_FIRST_CODE);
				
				printf("Node:%d, Grade:%d, time:%f, in first period.\n\
						Have sent CTS.\n\
						Wating to receive Data.\n\
						If no data, after %f secondes, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state13_enter_exec)

			/** state (wait DATA) exit executives **/
			FSM_STATE_EXIT_FORCED (13, "wait DATA", "gmac_mobil [wait DATA exit execs]")


			/** state (wait DATA) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "wait DATA", "first", "tr_77", "gmac_mobil [wait DATA -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (send ACK) enter executives **/
			FSM_STATE_ENTER_FORCED (14, "send ACK", state14_enter_exec, "gmac_mobil [send ACK enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [send ACK enter execs]", state14_enter_exec)
				{
				//send ACK
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_create_fmt("MAC_ACK_PK");
				op_pk_nfd_set(pk_TV,"Type",ack_pk);
				op_pk_nfd_set(pk_TV,"Src",node_address);
				op_pk_nfd_set(pk_TV,"Dest",next_hop_1);
				op_pk_nfd_set(pk_TV,"Grade",grade);
				
				op_pk_send(pk_TV,SEND_STRM);
				
				//time statistic
				all_tx_time = all_tx_time + durACK;
				
				op_intrpt_schedule_self(op_sim_time() + durACK,SLEEP_FROM_FIRST_CODE);
				
				printf("Node:%d, Grade:%d, time:%f, in first period.\n\
						Have replied ACK.\n\
						Go to sleep after finishing sending ACK.\n",\
						node_address,grade,op_sim_time());
				}
				FSM_PROFILE_SECTION_OUT (state14_enter_exec)

			/** state (send ACK) exit executives **/
			FSM_STATE_EXIT_FORCED (14, "send ACK", "gmac_mobil [send ACK exit execs]")


			/** state (send ACK) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "send ACK", "first", "tr_80", "gmac_mobil [send ACK -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (carrier sense(1)) enter executives **/
			FSM_STATE_ENTER_FORCED (15, "carrier sense(1)", state15_enter_exec, "gmac_mobil [carrier sense(1) enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [carrier sense(1) enter execs]", state15_enter_exec)
				{
				printf("Node:%d, time:%f. In first period.\n进入 statwire 中断\n",node_address,op_sim_time());
				if(op_ev_valid(send_CTS_event))
				{
					op_ev_cancel(send_CTS_event);
					op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_FIRST_CODE);
					printf("Channel is busy, cancel \"send_CTS_event\", go to sleep immediately.\n");
				}		
				}
				FSM_PROFILE_SECTION_OUT (state15_enter_exec)

			/** state (carrier sense(1)) exit executives **/
			FSM_STATE_EXIT_FORCED (15, "carrier sense(1)", "gmac_mobil [carrier sense(1) exit execs]")


			/** state (carrier sense(1)) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "carrier sense(1)", "first", "tr_94", "gmac_mobil [carrier sense(1) -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (carrier sense(2)) enter executives **/
			FSM_STATE_ENTER_FORCED (16, "carrier sense(2)", state16_enter_exec, "gmac_mobil [carrier sense(2) enter execs]")
				FSM_PROFILE_SECTION_IN ("gmac_mobil [carrier sense(2) enter execs]", state16_enter_exec)
				{
				printf("Node:%d, time:%f. In second period.\n",node_address,op_sim_time());
				printf("进入 statwire 中断\n");
				
				if(op_ev_valid(send_RTS_event))
				{
					op_ev_cancel(send_RTS_event);
					op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
					printf("Channel is busy, cancel \"send_RTS_event\", go to sleep immediately.\n");
				}
				}
				FSM_PROFILE_SECTION_OUT (state16_enter_exec)

			/** state (carrier sense(2)) exit executives **/
			FSM_STATE_EXIT_FORCED (16, "carrier sense(2)", "gmac_mobil [carrier sense(2) exit execs]")


			/** state (carrier sense(2)) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "carrier sense(2)", "second", "tr_96", "gmac_mobil [carrier sense(2) -> second : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"gmac_mobil")
		}
	}




void
_op_gmac_mobil_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_gmac_mobil_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_gmac_mobil_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_gmac_mobil_svar function. */
#undef process_id
#undef node_id
#undef node_address
#undef grade
#undef send_gm_timer
#undef start_schedule_timer
#undef sleep_factor
#undef send_gm_event
#undef CW
#undef DIFS
#undef SIFS
#undef durRTS
#undef durCTS
#undef durDATA
#undef durACK
#undef durSleep
#undef durActive_half
#undef wait_RTS_event
#undef transceiver_state
#undef rx_id
#undef waking_time
#undef send_RTS_event
#undef second_contending_time
#undef wait_CTS_event
#undef wait_ACK_event
#undef next_hop_1
#undef next_hop_2
#undef send_CTS_event
#undef wait_DATA_event
#undef record_time
#undef all_sleeped_time
#undef all_active_time
#undef all_rx_time
#undef all_tx_time

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_gmac_mobil_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_gmac_mobil_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (gmac_mobil)",
		sizeof (gmac_mobil_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_gmac_mobil_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	gmac_mobil_state * ptr;
	FIN_MT (_op_gmac_mobil_alloc (obtype))

	ptr = (gmac_mobil_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "gmac_mobil [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_gmac_mobil_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	gmac_mobil_state		*prs_ptr;

	FIN_MT (_op_gmac_mobil_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (gmac_mobil_state *)gen_ptr;

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
	if (strcmp ("grade" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->grade);
		FOUT
		}
	if (strcmp ("send_gm_timer" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_gm_timer);
		FOUT
		}
	if (strcmp ("start_schedule_timer" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->start_schedule_timer);
		FOUT
		}
	if (strcmp ("sleep_factor" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->sleep_factor);
		FOUT
		}
	if (strcmp ("send_gm_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_gm_event);
		FOUT
		}
	if (strcmp ("CW" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->CW);
		FOUT
		}
	if (strcmp ("DIFS" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->DIFS);
		FOUT
		}
	if (strcmp ("SIFS" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->SIFS);
		FOUT
		}
	if (strcmp ("durRTS" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durRTS);
		FOUT
		}
	if (strcmp ("durCTS" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durCTS);
		FOUT
		}
	if (strcmp ("durDATA" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durDATA);
		FOUT
		}
	if (strcmp ("durACK" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durACK);
		FOUT
		}
	if (strcmp ("durSleep" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durSleep);
		FOUT
		}
	if (strcmp ("durActive_half" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durActive_half);
		FOUT
		}
	if (strcmp ("wait_RTS_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wait_RTS_event);
		FOUT
		}
	if (strcmp ("transceiver_state" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->transceiver_state);
		FOUT
		}
	if (strcmp ("rx_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->rx_id);
		FOUT
		}
	if (strcmp ("waking_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->waking_time);
		FOUT
		}
	if (strcmp ("send_RTS_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_RTS_event);
		FOUT
		}
	if (strcmp ("second_contending_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->second_contending_time);
		FOUT
		}
	if (strcmp ("wait_CTS_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wait_CTS_event);
		FOUT
		}
	if (strcmp ("wait_ACK_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wait_ACK_event);
		FOUT
		}
	if (strcmp ("next_hop_1" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->next_hop_1);
		FOUT
		}
	if (strcmp ("next_hop_2" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->next_hop_2);
		FOUT
		}
	if (strcmp ("send_CTS_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_CTS_event);
		FOUT
		}
	if (strcmp ("wait_DATA_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wait_DATA_event);
		FOUT
		}
	if (strcmp ("record_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->record_time);
		FOUT
		}
	if (strcmp ("all_sleeped_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->all_sleeped_time);
		FOUT
		}
	if (strcmp ("all_active_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->all_active_time);
		FOUT
		}
	if (strcmp ("all_rx_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->all_rx_time);
		FOUT
		}
	if (strcmp ("all_tx_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->all_tx_time);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

