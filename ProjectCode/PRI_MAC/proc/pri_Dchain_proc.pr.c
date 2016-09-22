/* Process model C form file: pri_Dchain_proc.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char pri_Dchain_proc_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 51CCB618 51CCB618 1 wpd FeiTong 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                              ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

//Define node type
#define sink 	1
#define sensor 	2

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
#define START_GRADE_CODE				1000
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
#define START_GRADE					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == START_GRADE_CODE))
#define SEND_GM                 	((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_GM_CODE))

#define START_SCHEDULE				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == START_SCHEDULE_CODE))
#define WAKING_TO_WAIT_RTS			(START_SCHEDULE && grade>=0 && grade%(sleep_factor+2)==0)
#define CONTEND_TO_SEND_RTS			(START_SCHEDULE && grade>=0 && grade%(sleep_factor+2)==1)
#define GO_TO_SLEEP					(START_SCHEDULE && grade>=0 && grade%(sleep_factor+2)>=2)

#define WAKING_UP					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == TO_WAKE_UP_CODE))
#define WAKING_TO_SEND_RTS			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == WAKING_TO_SEND_RTS_CODE))
#define SEND_RTS					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_RTS_CODE))
#define SEND_DATA					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_DATA_CODE))
#define SEND_CTS					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_CTS_CODE))
#define SEND_ACK					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_ACK_CODE))
#define SLEEP_FROM_SECOND			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SLEEP_FROM_SECOND_CODE))
#define SLEEP_FROM_FIRST			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SLEEP_FROM_FIRST_CODE))
#define CHECK_CHANNEL				((op_intrpt_type() == OPC_INTRPT_STAT) && op_intrpt_stat()==0)

typedef struct
{
	int rt_next_hop;
	int rt_grade;
}RoutingTable;


typedef struct PriT_mac_Flags
{
	Boolean queue_full; //the queue is full or not, check every start of T state.
}PriT_Mac_Flags;


typedef struct
{
	int st_data_id;
}DataID;

//function prototype
static Boolean data_id_exist(int id);
static void grade_pk_proc(Packet * pk);
static void upper_data_pk_proc(void);
static void power_stat(void);
static void record_grade(void);
static void update_routing_table(int,int);
static int rt_pos_find_next(void);

int retx=0; //the number of retransmission

double avg_energy_consumption = 0.0;
double avg_duty_cycle=0.0;
int data_nums_1=0;

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
	int	                    		node_type                                       ;
	double	                 		start_grade_timer                               ;
	double	                 		all_sleeped_time                                ;
	double	                 		all_active_time                                 ;
	double	                 		all_tx_time                                     ;
	double	                 		all_rx_time                                     ;
	Objid	                  		tx_id                                           ;
	Objid	                  		tx_channel_objid                                ;
	double	                 		data_rate                                       ;
	Boolean	                		debug                                           ;
	List *	                 		data_id_list                                    ;
	int	                    		one_two_cw                                      ;
	List *	                 		routing_table_list                              ;
	double	                 		queue_length                                    ;
	Objid	                  		queue_id                                        ;
	Objid	                  		subq_comp_attr_id                               ;
	Objid	                  		sub_queue_id                                    ;
	Stathandle	             		QLength_handle                                  ;
	Stathandle	             		avg_dc_handle                                   ;
	} pri_Dchain_proc_state;

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
#define node_type               		op_sv_ptr->node_type
#define start_grade_timer       		op_sv_ptr->start_grade_timer
#define all_sleeped_time        		op_sv_ptr->all_sleeped_time
#define all_active_time         		op_sv_ptr->all_active_time
#define all_tx_time             		op_sv_ptr->all_tx_time
#define all_rx_time             		op_sv_ptr->all_rx_time
#define tx_id                   		op_sv_ptr->tx_id
#define tx_channel_objid        		op_sv_ptr->tx_channel_objid
#define data_rate               		op_sv_ptr->data_rate
#define debug                   		op_sv_ptr->debug
#define data_id_list            		op_sv_ptr->data_id_list
#define one_two_cw              		op_sv_ptr->one_two_cw
#define routing_table_list      		op_sv_ptr->routing_table_list
#define queue_length            		op_sv_ptr->queue_length
#define queue_id                		op_sv_ptr->queue_id
#define subq_comp_attr_id       		op_sv_ptr->subq_comp_attr_id
#define sub_queue_id            		op_sv_ptr->sub_queue_id
#define QLength_handle          		op_sv_ptr->QLength_handle
#define avg_dc_handle           		op_sv_ptr->avg_dc_handle

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	pri_Dchain_proc_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((pri_Dchain_proc_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif


static void update_routing_table(int node_hop,int node_grade)
{
	RoutingTable *rt;
	FIN(update_routing_table(int node_hop,int node_grade));
	
	rt = (RoutingTable *)op_prg_mem_alloc(sizeof(RoutingTable));
	
	rt->rt_next_hop = node_hop;
	rt->rt_grade = node_grade;
	op_prg_list_insert(routing_table_list,rt,OPC_LISTPOS_TAIL);
	
	FOUT;
}

//Deal with grade pk
static void
grade_pk_proc(Packet * pk)
{
//var
	int pre_hop, grade_in_pk;//,sink_id_in_pk;
//in
	FIN(grade_pk_proc(pk));
//body
	op_pk_nfd_get(pk,"Grade",&grade_in_pk);
	op_pk_nfd_get(pk,"Previous Hop",&pre_hop);

	//op_pk_nfd_get(pk,"Sink ID",&sink_id_in_pk);
	
	if(grade<0)
	{
		grade = grade_in_pk;
		op_ima_obj_attr_set(process_id,"grade",grade);
		//sink_id = sink_id_in_pk;
		
		update_routing_table(pre_hop,grade_in_pk-1);
		
		++grade_in_pk;
		op_pk_nfd_set(pk,"Grade",grade_in_pk);
		op_pk_nfd_set(pk,"Previous Hop",node_address);
		
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
		op_ima_obj_attr_set(process_id,"grade",grade);
		//if(sink_id != sink_id_in_pk)
		//{
		//	sink_id = sink_id_in_pk;
		//}
		op_prg_list_free(routing_table_list);
		update_routing_table(pre_hop,grade_in_pk-1);
			
		++grade_in_pk;
		op_pk_nfd_set(pk,"Grade",grade_in_pk);
		op_pk_nfd_set(pk,"Previous Hop",node_address);
		
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
	
	if(grade_in_pk == grade)
	{
		update_routing_table(pre_hop,grade_in_pk-1);		
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
/*
	if(mac_flags->queue_full==OPC_TRUE){
		op_pk_destroy(upper_pk);
		op_sim_message("queue is full, upper layer packet is dropped!","");
		if(debug)
			printf("The data subqueue is full, the upper layer packet is dropped.");
	}else{		
*/
		op_pk_nfd_set(upper_pk,"Type",data_pk);
		op_pk_nfd_set(upper_pk,"Src",node_address);
		op_pk_nfd_set(upper_pk,"Grade",grade);
	
		op_subq_pk_insert (data_queue,upper_pk,OPC_QPOS_TAIL);
	
	op_stat_write(QLength_handle, op_subq_stat(data_queue,OPC_QSTAT_PKSIZE));
		if(debug)
			printf("Have inserted the received DATA from \"gsrc\" into DATA queue\n");
//	}
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
	int nodes_num,sink_num;
//in
	FIN(power_stat(void));
//vody
	
	op_ima_obj_attr_get(node_id,"Power Record File",temp_file_name);
	if(strcmp(temp_file_name,"disabled")!=0)
	{	
		//op_sim_message("power stat","");
		if(node_type == sensor){
			avg_energy_consumption += all_sleeped_time*0.05+(all_active_time-(all_tx_time+all_rx_time))*0.45+(all_tx_time+all_rx_time)*0.5;
			avg_duty_cycle += all_active_time/(all_sleeped_time+all_active_time);
		}
		nodes_num = op_topo_object_count (OPC_OBJTYPE_NDMOB);
		op_ima_obj_attr_get(process_id, "Sink Num",&sink_num);
		if(node_address == nodes_num-sink_num){
			op_stat_write_t(avg_dc_handle,avg_duty_cycle/(nodes_num-sink_num),(double)(nodes_num-sink_num));
			//op_stat_write(avg_dc_handle,0);
			
			in = fopen(temp_file_name,"at");
	
			//fprintf(in,"%d %f %f %f\r\n",node_address,all_sleeped_time*0.05+(all_active_time-(all_tx_time+all_rx_time))*0.45+(all_tx_time+all_rx_time)*0.5,all_active_time/(all_sleeped_time+all_active_time),(all_sleeped_time+all_active_time));
			
		
			//fprintf(in,"%d %f %f %d\r\n",node_address,avg_energy_consumption/((nodes_num-sink_num)*(all_sleeped_time+all_active_time)),(avg_duty_cycle/(nodes_num-sink_num))*100,nodes_num-sink_num);
			fprintf(in,"%f\r\n",(avg_duty_cycle/(nodes_num-sink_num)));
		
			fclose(in);
		}
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
	fprintf(file_in,"%d	%d\r\n",\
		node_address,grade);
	fclose(file_in);
	
	if(node_type==sink && node_address == -1)
	{
		op_ima_obj_attr_get(process_id,"Retx File",file_name);
		file_in = fopen(file_name,"at");
		fprintf(file_in,"%d	%d	%f\r\n",\
		retx,data_nums_1,(double)retx/(double)data_nums_1);
		fclose(file_in);
	}
//out
	FOUT;
}

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

static int rt_pos_find_next()
{
	RoutingTable* rt;
	int i,rt_size;
	
    FIN(rt_find_next());
	
	rt_size = op_prg_list_size(routing_table_list);
	if(rt_size>0)
	{
		i = op_dist_outcome(op_dist_load("uniform_int",0,rt_size-1));
		rt = op_prg_list_access(routing_table_list,i);
		FRET(rt->rt_next_hop);
	}
	FRET(0); //"0" can not be used as the node address---reserved number
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
	void pri_Dchain_proc (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_pri_Dchain_proc_init (int * init_block_ptr);
	void _op_pri_Dchain_proc_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_pri_Dchain_proc_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_pri_Dchain_proc_alloc (VosT_Obtype, int);
	void _op_pri_Dchain_proc_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
pri_Dchain_proc (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (pri_Dchain_proc ());

		{
		/* Temporary Variables */
		Packet* pk_TV=NULL;
		int pk_type;
		double time;
		int dest_TV;
		int grade_TV;
		int hop_num_TV;
		int pk_size;
		//File
		FILE *in;
		char temp_file_name[300];
		//char message[100];
		int data_id;
		
		
		DataID * id_TV;
		/* End of Temporary Variables */


		FSM_ENTER ("pri_Dchain_proc")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "pri_Dchain_proc [init enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [init enter execs]", state0_enter_exec)
				{
				// Obtain related ID
				process_id = op_id_self();
				node_id = op_topo_parent(process_id);
				rx_id = op_id_from_name(node_id,OPC_OBJTYPE_RARX,"rx");
				tx_id = op_id_from_name(node_id,OPC_OBJTYPE_RATX,"tx");//the name of receiver radio in node layer called "tx"
				
				
				//obtain tx channel of compound type
				op_ima_obj_attr_get_objid (tx_id, "channel",&tx_channel_objid);
				
				tx_channel_objid = op_topo_child (tx_channel_objid, OPC_OBJTYPE_RATXCH, 0);
				
				op_ima_obj_attr_get(process_id, "Debug",&debug);
				
				op_ima_obj_attr_get(tx_channel_objid,"data rate",&data_rate);
				
				//debug =OPC_TRUE;
				
				//mac_flags=(PriT_Mac_Flags *) op_prg_mem_alloc (sizeof (PriT_Mac_Flags));
				
				//obtain the queue length
				queue_id = op_topo_child(node_id,OPC_OBJTYPE_QUEUE,0);
				subq_comp_attr_id = op_topo_child(queue_id,OPC_OBJTYPE_COMP,0);
				sub_queue_id = op_topo_child(subq_comp_attr_id,OPC_OBJTYPE_SUBQ,1);
				//queue_length's type is double
				op_ima_obj_attr_get(sub_queue_id,"pk capacity",&queue_length);
				if (debug) {
					printf("queue length:\n");
					printf("The maximum queue length is %f.\n",queue_length);
				}
				
				
				if(debug)
					printf("data_rate=%f\n",data_rate);
				
				pk_TV = op_pk_create_fmt("MAC_DATA_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durDATA=(pk_size+10)/data_rate;
				op_pk_destroy(pk_TV);
				if(debug)
					printf("durDATA=%f\n",durDATA);
				
				pk_TV = op_pk_create_fmt("MAC_RTS_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durRTS=(pk_size+10)/data_rate;
				op_pk_destroy(pk_TV);
				if(debug)
					printf("durRTS=%f\n",durRTS);
				
				pk_TV = op_pk_create_fmt("MAC_CTS_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durCTS=(pk_size+10)/data_rate;
				op_pk_destroy(pk_TV);
				if(debug)
					printf("durCTS=%f\n",durCTS);
				
				pk_TV = op_pk_create_fmt("MAC_ACK_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durACK=(pk_size+10)/data_rate;
				op_pk_destroy(pk_TV);
				if(debug)
					printf("durACK=%f\n",durACK);
				
				//obtain parameter
				op_ima_obj_attr_get(process_id, "Start Grade Timer", &start_grade_timer);		//1
				op_ima_obj_attr_get(process_id, "Send GM Timer", &send_gm_timer);				//2
				op_ima_obj_attr_get(process_id, "Start Schedule Timer", &start_schedule_timer);	//3
				op_ima_obj_attr_get(process_id, "Sleep Factor", &sleep_factor);					//4
				op_ima_obj_attr_get(process_id, "CW",&CW);										//5
				//op_ima_obj_attr_get(process_id, "durRTS",&durRTS);								//6
				//op_ima_obj_attr_get(process_id, "durCTS",&durCTS);								//7
				//op_ima_obj_attr_get(process_id, "durDATA",&durDATA);							//8
				//op_ima_obj_attr_get(process_id, "durACK",&durACK);								//9
				op_ima_obj_attr_get(process_id, "DIFS",&DIFS);									//10
				op_ima_obj_attr_get(process_id, "SIFS",&SIFS);									//11
				op_ima_obj_attr_get(process_id,"one/two CW", &one_two_cw);
				
				
				op_ima_obj_attr_get(node_id, "user id", &node_address);
				op_ima_obj_attr_get(node_id, "Node Type",&node_type);
				
				//op_ima_obj_attr_get(process_id, "Communication Range",&range);
				//op_ima_obj_attr_set(node_id, "Communication Range",range);
				
				op_ima_obj_attr_get(rx_id, "state", &transceiver_state);
				
				routing_table_list=op_prg_list_create();
				
				grade = -1;
				op_ima_obj_attr_set(process_id,"grade",grade);
				//sink_id = -1;
				
				all_sleeped_time = 0;
				all_active_time = 0;
				all_tx_time = 0;
				all_rx_time = 0;
				
				data_id_list=op_prg_list_create();
				
				/*
				if(op_subq_stat(data_queue,OPC_QSTAT_PKSIZE)==queue_length){
					mac_flags->queue_full=OPC_TRUE;
				}else{
					mac_flags->queue_full=OPC_FALSE;
				}
				*/
				
				//data_nums=0;
				
				//durActive contains two parts: durReceive & durSedn, and durReceive==durSend
				if(one_two_cw == 2){
					durActive_half = 2*CW+durRTS+durCTS+durDATA+durACK+DIFS+3*SIFS;
				}else if(one_two_cw == 1){
					durActive_half = 2*CW+durRTS+durCTS+durDATA+durACK+DIFS+3*SIFS;
				}
				if(debug)
					printf("durActive_half=%f, T_cycle=%f\n",durActive_half,(sleep_factor+2)*durActive_half);
				
				durSleep = sleep_factor * durActive_half;
				if(node_type == sink)
				{
					op_intrpt_schedule_self(op_sim_time() + start_grade_timer,START_GRADE_CODE);
				}	
				
				op_intrpt_schedule_self(op_sim_time() + start_schedule_timer, START_SCHEDULE_CODE);
				
				//statistics regester
				QLength_handle = op_stat_reg("Queue Length", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				avg_dc_handle = op_stat_reg("Average Duty Cycle", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				
				//op_ima_obj_attr_get (process_id, "subqueue", &subq_objid);
				//num_subqs = op_topo_child_count (subq_objid, OPC_OBJTYPE_SUBQ);
				//printf("子队列个数:%d\n",num_subqs);
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "pri_Dchain_proc [init exit execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [init exit execs]", state0_exit_exec)
				{
				/*
				op_ima_obj_attr_get(node_id,"Grade Record File",temp_file_name);
				remove(temp_file_name);
				op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
				remove(temp_file_name);
				*/
				if(node_type == sink)
				{
					op_ima_obj_attr_get(node_id,"Log File",temp_file_name);
					in = fopen(temp_file_name,"at");
					
					fprintf(in,"Begin:\r\ndurActive_half: %f, durSleep: %f. cycle time: (%d+2)*%f=%f.(in \"gmac->init\")\r\n",\
						durActive_half,durSleep,sleep_factor,durActive_half,(sleep_factor+2)*durActive_half);
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
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_1", "pri_Dchain_proc [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "pri_Dchain_proc [idle enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [idle enter execs]", state1_enter_exec)
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
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
				}
				
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(debug)
						printf("In \"idle\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				}
				FSM_PROFILE_SECTION_OUT (state1_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"pri_Dchain_proc")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "pri_Dchain_proc [idle exit execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [idle exit execs]", state1_exit_exec)
				{
				/*
				if(node_type!=sink)
					grade=1;
				
				*/
				}
				FSM_PROFILE_SECTION_OUT (state1_exit_exec)


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (CONTEND_TO_SEND_RTS)
			FSM_TEST_COND (GO_TO_SLEEP)
			FSM_TEST_COND (WAKING_TO_WAIT_RTS)
			FSM_TEST_COND (START_GRADE)
			FSM_TEST_COND (SEND_GM)
			FSM_TEST_COND (FROM_LOWER && transceiver_state)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "CONTEND_TO_SEND_RTS", "", "idle", "second contention", "tr_28", "pri_Dchain_proc [idle -> second contention : CONTEND_TO_SEND_RTS / ]")
				FSM_CASE_TRANSIT (1, 4, state4_enter_exec, ;, "GO_TO_SLEEP", "", "idle", "sleeping", "tr_32", "pri_Dchain_proc [idle -> sleeping : GO_TO_SLEEP / ]")
				FSM_CASE_TRANSIT (2, 5, state5_enter_exec, ;, "WAKING_TO_WAIT_RTS", "", "idle", "waking", "tr_34", "pri_Dchain_proc [idle -> waking : WAKING_TO_WAIT_RTS / ]")
				FSM_CASE_TRANSIT (3, 17, state17_enter_exec, ;, "START_GRADE", "", "idle", "start grade", "tr_97", "pri_Dchain_proc [idle -> start grade : START_GRADE / ]")
				FSM_CASE_TRANSIT (4, 3, state3_enter_exec, ;, "SEND_GM", "", "idle", "send gm", "tr_100", "pri_Dchain_proc [idle -> send gm : SEND_GM / ]")
				FSM_CASE_TRANSIT (5, 2, state2_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "idle", "GM proc", "tr_102", "pri_Dchain_proc [idle -> GM proc : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (6, 1, state1_enter_exec, ;, "default", "", "idle", "idle", "tr_108", "pri_Dchain_proc [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (GM proc) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "GM proc", state2_enter_exec, "pri_Dchain_proc [GM proc enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [GM proc enter execs]", state2_enter_exec)
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
					op_sim_end("Waiting Grade Message Err: Are you kidding me?","I'm waiting Grade Message","Not you","请仔细检查");
				
				}
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (GM proc) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "GM proc", "pri_Dchain_proc [GM proc exit execs]")


			/** state (GM proc) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "GM proc", "idle", "tr_103", "pri_Dchain_proc [GM proc -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (send gm) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "send gm", state3_enter_exec, "pri_Dchain_proc [send gm enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [send gm enter execs]", state3_enter_exec)
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
			FSM_STATE_EXIT_FORCED (3, "send gm", "pri_Dchain_proc [send gm exit execs]")


			/** state (send gm) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "send gm", "idle", "tr_101", "pri_Dchain_proc [send gm -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (sleeping) enter executives **/
			FSM_STATE_ENTER_UNFORCED (4, "sleeping", state4_enter_exec, "pri_Dchain_proc [sleeping enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [sleeping enter execs]", state4_enter_exec)
				{
				if(op_intrpt_code() == START_SCHEDULE_CODE)// from "GO_TO_SLEEP"
				{	
					if(grade % (sleep_factor+2)>=2)
					{
						time = (sleep_factor+2-grade%(sleep_factor+2))*durActive_half;
						//time = durSleep - ((grade%(sleep_factor+2))-3)*durActive_half;			
					}
					else
					{
						op_sim_end("仿真出错","grade%(sleep_factor+2)不大于3或0","请反思","");
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
					if(debug)
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
						
					if (node_type == sensor){
						//when wake up, go to "second contention" state
						op_intrpt_schedule_self(op_sim_time() + time, WAKING_TO_SEND_RTS_CODE);
						if(debug)
							printf("Node:%d, Grade:%d, time:%f,in sleeping period.\n\
								To sleeping from the first period.\n\
								After %f seconds, go to second period.\n",\
								node_address,grade,op_sim_time(),time);
					
						//time statistic
						all_active_time = all_active_time+(op_sim_time()-record_time);
						record_time = op_sim_time();
					}else if(node_type == sink){
						//when wake up, go to "waking" state	
						op_intrpt_schedule_self(op_sim_time() + time, TO_WAKE_UP_CODE);
						
						if(debug)
							printf("Node:%d, Grade:%d, time:%f,sink node in sleeping period.\n\
								from the first period.\n\
								After %f seconds, go to wake up.\n\n",\
								node_address,grade,op_sim_time(),time);
					
						//time statistic
						all_active_time = all_active_time+(op_sim_time()-record_time);
						record_time = op_sim_time();
					}
				}
				else
				if(op_intrpt_code() == SLEEP_FROM_SECOND_CODE)//sleep from second period
				{
					
					time = second_contending_time + durActive_half + durSleep - op_sim_time();
					
					//when wake up, go to "waking" state
					op_intrpt_schedule_self(op_sim_time() + time, TO_WAKE_UP_CODE);
					if(debug)
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
					if(debug)
						printf("In \"sleeping\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				
				if((FROM_LOWER && !transceiver_state)||(FROM_LOWER && node_type==sink))
				{	
					op_pk_destroy(op_pk_get(RCV_STRM));
					if(debug)
						printf("Node:%d, time:%f. Node is sleeping, destroy the received pk.\n",\
							node_address,op_sim_time());
				}
				if(END)
				{
					all_sleeped_time = all_sleeped_time + (op_sim_time()-record_time);
					power_stat();
					record_grade();
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
				}
				//close transceiver
				transceiver_state = OPC_FALSE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,"pri_Dchain_proc")


			/** state (sleeping) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "sleeping", "pri_Dchain_proc [sleeping exit execs]")


			/** state (sleeping) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [sleeping trans conditions]", state4_trans_conds)
			FSM_INIT_COND (WAKING_UP)
			FSM_TEST_COND (WAKING_TO_SEND_RTS)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("sleeping")
			FSM_PROFILE_SECTION_OUT (state4_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 5, state5_enter_exec, ;, "WAKING_UP", "", "sleeping", "waking", "tr_37", "pri_Dchain_proc [sleeping -> waking : WAKING_UP / ]")
				FSM_CASE_TRANSIT (1, 6, state6_enter_exec, ;, "WAKING_TO_SEND_RTS", "", "sleeping", "second contention", "tr_40", "pri_Dchain_proc [sleeping -> second contention : WAKING_TO_SEND_RTS / ]")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "default", "", "sleeping", "sleeping", "tr_91", "pri_Dchain_proc [sleeping -> sleeping : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (waking) enter executives **/
			FSM_STATE_ENTER_FORCED (5, "waking", state5_enter_exec, "pri_Dchain_proc [waking enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [waking enter execs]", state5_enter_exec)
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
				//if(node_type == sensor){
					transceiver_state = OPC_TRUE;
					op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				//}
				
				waking_time = op_sim_time();
				
				if(op_subq_stat(data_queue,OPC_QSTAT_PKSIZE)!=queue_length){ //queue is not full
					time = CW+DIFS+durRTS;
					wait_RTS_event = op_intrpt_schedule_self(op_sim_time() + time, SLEEP_FROM_FIRST_CODE);
				}else{ //queue is full, cannot receive anything, go to sleep immediately.
					op_intrpt_schedule_self(op_sim_time() + 0.0, SLEEP_FROM_FIRST_CODE);
				}
				
				if(debug)
					printf("Node:%d, Grade:%d, time:%f, entered into first period (\"waking\" state).\n\
					    Waiting RTS.\n\
						If no RTS, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				
				//time statistic
				//if(node_type == sensor){
					all_sleeped_time = all_sleeped_time + (op_sim_time() - record_time);
					record_time = op_sim_time();
				//}
				}
				FSM_PROFILE_SECTION_OUT (state5_enter_exec)

			/** state (waking) exit executives **/
			FSM_STATE_EXIT_FORCED (5, "waking", "pri_Dchain_proc [waking exit execs]")


			/** state (waking) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "waking", "first", "tr_57", "pri_Dchain_proc [waking -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (second contention) enter executives **/
			FSM_STATE_ENTER_FORCED (6, "second contention", state6_enter_exec, "pri_Dchain_proc [second contention enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [second contention enter execs]", state6_enter_exec)
				{
				/*
				if(op_subq_stat(data_queue,OPC_QSTAT_PKSIZE)==queue_length){
					mac_flags->queue_full=OPC_TRUE;
				}else{
					mac_flags->queue_full=OPC_FALSE;
				}
				*/
				
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
				//	time = op_dist_uniform(CW*1000.0)/1000.0 + DIFS;
					time = op_dist_outcome(op_dist_load("uniform_int",0.0,CW*1000.0-1)); // from [0,W-1]
					if(debug) printf("in send rts, time = %f.\n",time);
					time = time/1000.0 + DIFS;
					send_RTS_event = op_intrpt_schedule_self(op_sim_time()+time,SEND_RTS_CODE);
					
					if(debug)
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
					if(debug)
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
			FSM_STATE_EXIT_FORCED (6, "second contention", "pri_Dchain_proc [second contention exit execs]")


			/** state (second contention) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "second contention", "second", "tr_59", "pri_Dchain_proc [second contention -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (wait CTS) enter executives **/
			FSM_STATE_ENTER_FORCED (7, "wait CTS", state7_enter_exec, "pri_Dchain_proc [wait CTS enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [wait CTS enter execs]", state7_enter_exec)
				{
				//send RTS
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				if(node_type!=sink && (next_hop_2=rt_pos_find_next())==0)
				{
					op_sim_end("Routing table is empty Err: Are you sure you are not kidding me?","","","");
				}
				
				pk_TV = op_pk_create_fmt("MAC_RTS_PK");
				op_pk_nfd_set(pk_TV, "Type", rts_pk);
				op_pk_nfd_set(pk_TV,"Src", node_address);
				op_pk_nfd_set(pk_TV,"Grade",grade);
				op_pk_nfd_set(pk_TV, "Dest", next_hop_2);
				
				op_pk_send(pk_TV, SEND_STRM);
				
				//time statistic
				all_tx_time = all_tx_time + durRTS;
				
				//set timer for waiting CTS from lower grade nodes
				if(one_two_cw == 2){
					time = durRTS+CW+SIFS+durCTS;
				}else if(one_two_cw == 1){
					time = durRTS+SIFS+durCTS;
				}
				
				wait_CTS_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_SECOND_CODE);
				if(debug)
					printf("Node:%d, Grade:%d, time:%f, in second period.\n\
						Have sent RTS.\n\
						Waiting to receive CTS.\n\
						If no CTS, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state7_enter_exec)

			/** state (wait CTS) exit executives **/
			FSM_STATE_EXIT_FORCED (7, "wait CTS", "pri_Dchain_proc [wait CTS exit execs]")


			/** state (wait CTS) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "wait CTS", "second", "tr_89", "pri_Dchain_proc [wait CTS -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (first) enter executives **/
			FSM_STATE_ENTER_UNFORCED (8, "first", state8_enter_exec, "pri_Dchain_proc [first enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [first enter execs]", state8_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(debug)
						printf("In \"first\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				
				if(END)
				{
					if(node_type==sensor)
					{
						all_active_time = all_active_time+(op_sim_time()-record_time);
						power_stat();
						record_grade();
					}
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
				}
				}
				FSM_PROFILE_SECTION_OUT (state8_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (17,"pri_Dchain_proc")


			/** state (first) exit executives **/
			FSM_STATE_EXIT_UNFORCED (8, "first", "pri_Dchain_proc [first exit execs]")


			/** state (first) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [first trans conditions]", state8_trans_conds)
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
				FSM_CASE_TRANSIT (0, 4, state4_enter_exec, ;, "SLEEP_FROM_FIRST", "", "first", "sleeping", "tr_56", "pri_Dchain_proc [first -> sleeping : SLEEP_FROM_FIRST / ]")
				FSM_CASE_TRANSIT (1, 12, state12_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "first", "RCV(1)", "tr_74", "pri_Dchain_proc [first -> RCV(1) : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (2, 13, state13_enter_exec, ;, "SEND_CTS", "", "first", "wait DATA", "tr_76", "pri_Dchain_proc [first -> wait DATA : SEND_CTS / ]")
				FSM_CASE_TRANSIT (3, 14, state14_enter_exec, ;, "SEND_ACK", "", "first", "send ACK", "tr_79", "pri_Dchain_proc [first -> send ACK : SEND_ACK / ]")
				FSM_CASE_TRANSIT (4, 15, state15_enter_exec, ;, "CHECK_CHANNEL", "", "first", "carrier sense(1)", "tr_93", "pri_Dchain_proc [first -> carrier sense(1) : CHECK_CHANNEL / ]")
				FSM_CASE_TRANSIT (5, 8, state8_enter_exec, ;, "default", "", "first", "first", "tr_90", "pri_Dchain_proc [first -> first : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (second) enter executives **/
			FSM_STATE_ENTER_UNFORCED (9, "second", state9_enter_exec, "pri_Dchain_proc [second enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [second enter execs]", state9_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(debug)
						printf("In \"second\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				if(END)
				{
					all_active_time = all_active_time + (op_sim_time()-record_time);
					power_stat();
					record_grade();
						
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
				}
				}
				FSM_PROFILE_SECTION_OUT (state9_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (19,"pri_Dchain_proc")


			/** state (second) exit executives **/
			FSM_STATE_EXIT_UNFORCED (9, "second", "pri_Dchain_proc [second exit execs]")


			/** state (second) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [second trans conditions]", state9_trans_conds)
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
				FSM_CASE_TRANSIT (0, 7, state7_enter_exec, ;, "SEND_RTS", "", "second", "wait CTS", "tr_60", "pri_Dchain_proc [second -> wait CTS : SEND_RTS / ]")
				FSM_CASE_TRANSIT (1, 4, state4_enter_exec, ;, "SLEEP_FROM_SECOND", "", "second", "sleeping", "tr_61", "pri_Dchain_proc [second -> sleeping : SLEEP_FROM_SECOND / ]")
				FSM_CASE_TRANSIT (2, 10, state10_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "second", "RCV(2)", "tr_66", "pri_Dchain_proc [second -> RCV(2) : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (3, 11, state11_enter_exec, ;, "SEND_DATA", "", "second", "send data", "tr_72", "pri_Dchain_proc [second -> send data : SEND_DATA / ]")
				FSM_CASE_TRANSIT (4, 16, state16_enter_exec, ;, "CHECK_CHANNEL", "", "second", "carrier sense(2)", "tr_95", "pri_Dchain_proc [second -> carrier sense(2) : CHECK_CHANNEL / ]")
				FSM_CASE_TRANSIT (5, 9, state9_enter_exec, ;, "default", "", "second", "second", "tr_92", "pri_Dchain_proc [second -> second : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (RCV(2)) enter executives **/
			FSM_STATE_ENTER_FORCED (10, "RCV(2)", state10_enter_exec, "pri_Dchain_proc [RCV(2) enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [RCV(2) enter execs]", state10_enter_exec)
				{
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_get(RCV_STRM);
				
				pk_size=op_pk_total_size_get(pk_TV);
				time=(pk_size+10)/data_rate;
				//time statistic
				//all_rx_time = all_rx_time + time;
				if(debug)
					printf("For sending data, Node:%d, Grade:%d, time:%f.\nReceived a packet, size: %d.\nReceive Duration is: %f.\n",	node_address,grade,op_sim_time(),pk_size,time);
				
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
						if(debug)
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
							if(debug)
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
							
							
							/*Since we don't consider retransmission, the reception of ACK or not becomes meaningless*/
							//Data has been sent correctly, remove it from temp_queue
							//op_pk_destroy(op_subq_pk_remove(data_queue, OPC_QPOS_HEAD));
							
							retx--;
							//recieved ACK correctly, go to sleep immediately
							op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
							if(debug)
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
			FSM_STATE_EXIT_FORCED (10, "RCV(2)", "pri_Dchain_proc [RCV(2) exit execs]")


			/** state (RCV(2)) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "RCV(2)", "second", "tr_71", "pri_Dchain_proc [RCV(2) -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (send data) enter executives **/
			FSM_STATE_ENTER_FORCED (11, "send data", state11_enter_exec, "pri_Dchain_proc [send data enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [send data enter execs]", state11_enter_exec)
				{
				//send DATA
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				//obtain DATA from data_queue
				//pk_TV = op_pk_copy(op_subq_pk_access(data_queue, OPC_QPOS_HEAD));
				
				//sprintf(message,"creation time: %f.\n", op_pk_creation_time_get (op_subq_pk_access(data_queue, OPC_QPOS_HEAD)));
				//op_sim_message(message,"1");
				
				/* since we don't consider consider retransmission, once a node win the contention,*/
				/* and send the data out, this data is removed from the queue. */
				pk_TV = op_subq_pk_remove(data_queue, OPC_QPOS_HEAD);
				//sprintf(message,"creation time: %f.\n", op_pk_creation_time_get (pk_TV));
				//op_sim_message(message,"2");
				
				//op_pk_creation_time_set (pk_TV,op_pk_creation_time_get (op_subq_pk_access(data_queue, OPC_QPOS_HEAD)));
					
				op_pk_nfd_set(pk_TV,"Previous Hop",node_address);
				op_pk_nfd_set(pk_TV,"Next Hop",next_hop_2);
				
				op_pk_send(pk_TV,SEND_STRM);
				retx++;
				
				//time statistic
				all_tx_time = all_tx_time + durDATA;
				
				//set timer for waiting ACK from lower grade node
				time = durDATA + SIFS + durACK;
				wait_ACK_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_SECOND_CODE);
				if(debug)
					printf("Node:%d, Grade:%d, time:%f, in second period.\n\
						Have sent DATA, waiting ACK.\n\
						If no ACK, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state11_enter_exec)

			/** state (send data) exit executives **/
			FSM_STATE_EXIT_FORCED (11, "send data", "pri_Dchain_proc [send data exit execs]")


			/** state (send data) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "send data", "second", "tr_73", "pri_Dchain_proc [send data -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (RCV(1)) enter executives **/
			FSM_STATE_ENTER_FORCED (12, "RCV(1)", state12_enter_exec, "pri_Dchain_proc [RCV(1) enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [RCV(1) enter execs]", state12_enter_exec)
				{
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_get(RCV_STRM);
				
				pk_size=op_pk_total_size_get(pk_TV);
				time=(pk_size+10)/data_rate;
				//time statistic
				//all_rx_time = all_rx_time + time;
				if(debug)
					printf("\nFor receiving data, Node:%d, Grade:%d, time:%f.\nReceived a packet, size: %d.\nReceive Duration is: %f.\n\n",	node_address,grade,op_sim_time(),pk_size,time);
				
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				
				//printf("out: please pay an attention here, 童飞!\n");
				//sprintf(message,"pk_type=%d.\n",pk_type);
				//op_sim_message(message,"");
				
				if(pk_type == rts_pk)
				{
					//time statistic
					all_rx_time = all_rx_time + durRTS;
				
					//wait for RTS
					op_pk_nfd_get(pk_TV,"Grade",&grade_TV);
					op_pk_nfd_get(pk_TV,"Dest",&dest_TV);
					//printf("in: please pay an attention here, 童飞!\n");
					if(grade_TV == grade+1)
					{
						if(op_ev_valid(wait_RTS_event))//the first time to receive RTS
						{
							op_ev_cancel(wait_RTS_event);
							if(one_two_cw == 2){
								op_pk_nfd_get(pk_TV,"Src",&next_hop_1);
							
								//prepare to send CTS
								//time = op_dist_uniform(CW*1000.0)/1000.0 + SIFS;
								time = op_dist_outcome(op_dist_load("uniform_int",0.0,CW*1000.0-1));
								if(debug) printf("in wait rts. time=%f.\n",time);
								time = time/1000.0 + SIFS;
								send_CTS_event = op_intrpt_schedule_self(op_sim_time()+time,SEND_CTS_CODE);
								if(debug)
									printf("Node:%d, Grade:%d, time:%f, in first period.\n\
										Successfully received RTS.\n\
										Waiting to send CTS.\n",\
										node_address,grade,op_sim_time());
								//sprintf(message,"Contend to send CTS, op_dist_uniform(CW)=%f\n",time-DIFS);
								//op_sim_message("",message);
							}
							else if(one_two_cw == 1){
								if(dest_TV==node_address)
								{
									op_pk_nfd_get(pk_TV,"Src",&next_hop_1);
							
									//prepare to send CTS
									time = SIFS;
									send_CTS_event = op_intrpt_schedule_self(op_sim_time()+time,SEND_CTS_CODE);
									if(debug)
										printf("Node:%d, Grade:%d, time:%f, in first period.\n\
												Successfully received RTS.\n\
								    			Waiting to send CTS.\n",\
												node_address,grade,op_sim_time());
									//sprintf(message,"Contend to send CTS, op_dist_uniform(CW)=%f\n",time-DIFS);
									//op_sim_message("",message);
								}else
								{
									if(debug)
										printf("Node:%d, Grade:%d, time:%f, in first period.\n\
												The received RTS is not destined to itself. Continue to wait. NAV has been set.\n",\
												node_address,grade,op_sim_time());
									op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_FIRST_CODE);
								}
							}
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
						if(debug)
							printf("Node:%d, Grade:%d, time:%f, in first period.\n\
								Failure to send CTS, because received CTS from other nodes.\n\
								Go to sleep immediately.\n",\
								node_address,grade,op_sim_time());
					}
					//op_sim_message("the received packet is cts_pk. ","be deleted");
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
							
							if(node_type == sink)
							{
								op_pk_nfd_get(pk_TV,"Data No",&data_id);
								
								if(!data_id_exist(data_id)){
									id_TV=(DataID *)op_prg_mem_alloc(sizeof(DataID));
									id_TV->st_data_id=data_id;
									op_prg_list_insert(data_id_list,id_TV,OPC_LISTPOS_TAIL);
									data_nums_1++;
								}
								
								op_pk_send(pk_TV,DISCARD_STRM);
								if(debug)
									printf("Data has arrived at sink!!!\n");
							}else
							{
								//insert the DATA packet into data_queue
								op_subq_pk_insert (data_queue,pk_TV,OPC_QPOS_TAIL);
							}
						
							//wait SIFS time to send ACK
							time = SIFS;
							op_intrpt_schedule_self(op_sim_time()+time,SEND_ACK_CODE);
							if(debug)
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
					if(debug)
						printf("Node:%d, Grade:%d, time:%f, in first period.\n\
							The received pk is not RTS, CTS, or DATA. The pk type is: %d.\n\
							Destroy this pk.\n",\
							node_address,grade,op_sim_time(),pk_type);
						
					op_pk_destroy(pk_TV);
				}	
				}
				FSM_PROFILE_SECTION_OUT (state12_enter_exec)

			/** state (RCV(1)) exit executives **/
			FSM_STATE_EXIT_FORCED (12, "RCV(1)", "pri_Dchain_proc [RCV(1) exit execs]")


			/** state (RCV(1)) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "RCV(1)", "first", "tr_75", "pri_Dchain_proc [RCV(1) -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (wait DATA) enter executives **/
			FSM_STATE_ENTER_FORCED (13, "wait DATA", state13_enter_exec, "pri_Dchain_proc [wait DATA enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [wait DATA enter execs]", state13_enter_exec)
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
				if(debug)
					printf("Node:%d, Grade:%d, time:%f, in first period.\n\
						Have sent CTS.\n\
						Wating to receive Data.\n\
						If no data, after %f secondes, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state13_enter_exec)

			/** state (wait DATA) exit executives **/
			FSM_STATE_EXIT_FORCED (13, "wait DATA", "pri_Dchain_proc [wait DATA exit execs]")


			/** state (wait DATA) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "wait DATA", "first", "tr_77", "pri_Dchain_proc [wait DATA -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (send ACK) enter executives **/
			FSM_STATE_ENTER_FORCED (14, "send ACK", state14_enter_exec, "pri_Dchain_proc [send ACK enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [send ACK enter execs]", state14_enter_exec)
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
				if(debug)
					printf("Node:%d, Grade:%d, time:%f, in first period.\n\
						Have replied ACK.\n\
						Go to sleep after finishing sending ACK.\n",\
						node_address,grade,op_sim_time());
				}
				FSM_PROFILE_SECTION_OUT (state14_enter_exec)

			/** state (send ACK) exit executives **/
			FSM_STATE_EXIT_FORCED (14, "send ACK", "pri_Dchain_proc [send ACK exit execs]")


			/** state (send ACK) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "send ACK", "first", "tr_80", "pri_Dchain_proc [send ACK -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (carrier sense(1)) enter executives **/
			FSM_STATE_ENTER_FORCED (15, "carrier sense(1)", state15_enter_exec, "pri_Dchain_proc [carrier sense(1) enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [carrier sense(1) enter execs]", state15_enter_exec)
				{
				if(debug)
				{
					printf("Node:%d, time:%f. In first period.\n进入 statwire 中断\n",node_address,op_sim_time());
					printf("up: the received snr=%E.intrpt_code=%d\n\n",op_stat_local_read(0),op_intrpt_code());
				}
				if(op_ev_valid(send_CTS_event) && (one_two_cw == 2))
				{
					op_ev_cancel(send_CTS_event);
					op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_FIRST_CODE);
					if(debug)
						printf("Channel is busy, cancel \"send_CTS_event\", go to sleep immediately.\n");
					
				}		
				
				}
				FSM_PROFILE_SECTION_OUT (state15_enter_exec)

			/** state (carrier sense(1)) exit executives **/
			FSM_STATE_EXIT_FORCED (15, "carrier sense(1)", "pri_Dchain_proc [carrier sense(1) exit execs]")


			/** state (carrier sense(1)) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "carrier sense(1)", "first", "tr_94", "pri_Dchain_proc [carrier sense(1) -> first : default / ]")
				/*---------------------------------------------------------*/



			/** state (carrier sense(2)) enter executives **/
			FSM_STATE_ENTER_FORCED (16, "carrier sense(2)", state16_enter_exec, "pri_Dchain_proc [carrier sense(2) enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [carrier sense(2) enter execs]", state16_enter_exec)
				{
				if(debug)
					printf("Node:%d, time:%f. In second period.\n",node_address,op_sim_time());
				if(debug)
				{
					printf("进入 statwire 中断\n");
					printf("down: the received snr=%E.intrpt_code=%d\n\n",op_stat_local_read(0),op_intrpt_code());
				}
				
				
				if(op_ev_valid(send_RTS_event))
				{
					op_ev_cancel(send_RTS_event);
					op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
					if(debug)
						printf("Channel is busy, cancel \"send_RTS_event\", go to sleep immediately.\n");
				}
				}
				FSM_PROFILE_SECTION_OUT (state16_enter_exec)

			/** state (carrier sense(2)) exit executives **/
			FSM_STATE_EXIT_FORCED (16, "carrier sense(2)", "pri_Dchain_proc [carrier sense(2) exit execs]")


			/** state (carrier sense(2)) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "carrier sense(2)", "second", "tr_96", "pri_Dchain_proc [carrier sense(2) -> second : default / ]")
				/*---------------------------------------------------------*/



			/** state (start grade) enter executives **/
			FSM_STATE_ENTER_FORCED (17, "start grade", state17_enter_exec, "pri_Dchain_proc [start grade enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_Dchain_proc [start grade enter execs]", state17_enter_exec)
				{
				//Sink will execute the following code
				grade = 0;
				op_ima_obj_attr_set(process_id,"grade",grade);
				//sink_id = node_address;
				
				pk_TV = op_pk_create_fmt("MAC_GRADE_PK");
				op_pk_nfd_set(pk_TV, "Type", grade_pk);
				//op_pk_nfd_set(pk_TV, "Sink ID", sink_id);
				op_pk_nfd_set(pk_TV,"Previous Hop",node_address);
				op_pk_nfd_set(pk_TV, "Grade", grade+1);
				
				op_pk_send(pk_TV, SEND_STRM);
				}
				FSM_PROFILE_SECTION_OUT (state17_enter_exec)

			/** state (start grade) exit executives **/
			FSM_STATE_EXIT_FORCED (17, "start grade", "pri_Dchain_proc [start grade exit execs]")


			/** state (start grade) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "start grade", "idle", "tr_98", "pri_Dchain_proc [start grade -> idle : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"pri_Dchain_proc")
		}
	}




void
_op_pri_Dchain_proc_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_pri_Dchain_proc_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_pri_Dchain_proc_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_pri_Dchain_proc_svar function. */
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
#undef node_type
#undef start_grade_timer
#undef all_sleeped_time
#undef all_active_time
#undef all_tx_time
#undef all_rx_time
#undef tx_id
#undef tx_channel_objid
#undef data_rate
#undef debug
#undef data_id_list
#undef one_two_cw
#undef routing_table_list
#undef queue_length
#undef queue_id
#undef subq_comp_attr_id
#undef sub_queue_id
#undef QLength_handle
#undef avg_dc_handle

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_pri_Dchain_proc_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_pri_Dchain_proc_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (pri_Dchain_proc)",
		sizeof (pri_Dchain_proc_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_pri_Dchain_proc_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	pri_Dchain_proc_state * ptr;
	FIN_MT (_op_pri_Dchain_proc_alloc (obtype))

	ptr = (pri_Dchain_proc_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "pri_Dchain_proc [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_pri_Dchain_proc_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	pri_Dchain_proc_state		*prs_ptr;

	FIN_MT (_op_pri_Dchain_proc_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (pri_Dchain_proc_state *)gen_ptr;

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
	if (strcmp ("node_type" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_type);
		FOUT
		}
	if (strcmp ("start_grade_timer" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->start_grade_timer);
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
	if (strcmp ("all_tx_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->all_tx_time);
		FOUT
		}
	if (strcmp ("all_rx_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->all_rx_time);
		FOUT
		}
	if (strcmp ("tx_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->tx_id);
		FOUT
		}
	if (strcmp ("tx_channel_objid" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->tx_channel_objid);
		FOUT
		}
	if (strcmp ("data_rate" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->data_rate);
		FOUT
		}
	if (strcmp ("debug" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->debug);
		FOUT
		}
	if (strcmp ("data_id_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->data_id_list);
		FOUT
		}
	if (strcmp ("one_two_cw" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->one_two_cw);
		FOUT
		}
	if (strcmp ("routing_table_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->routing_table_list);
		FOUT
		}
	if (strcmp ("queue_length" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->queue_length);
		FOUT
		}
	if (strcmp ("queue_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->queue_id);
		FOUT
		}
	if (strcmp ("subq_comp_attr_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->subq_comp_attr_id);
		FOUT
		}
	if (strcmp ("sub_queue_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->sub_queue_id);
		FOUT
		}
	if (strcmp ("QLength_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->QLength_handle);
		FOUT
		}
	if (strcmp ("avg_dc_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_dc_handle);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

