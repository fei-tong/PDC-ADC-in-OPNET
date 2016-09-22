/* Process model C form file: pw_mac.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char pw_mac_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 518A80FA 518A80FA 1 wpd FeiTong 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                              ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

//Define node type
#define sink 	1
#define sensor 	2

//Define packet type
#define grade_pk  1
//#define rts_pk    2
//#define cts_pk    3
//#define ack_pk    4
#define data_pk     5
#define beacon_pk   9

//Define queue type
#define grade_queue		0
#define data_queue		1
#define ack_queue		2

//Define stream in-out NO.
#define SRC_STRM 		1
#define RCV_STRM 		0
#define SEND_STRM 		0
#define DISCARD_STRM 	1

//Define Prediction State Flag embedded in DATA
#define Request			1
#define noRequest		2

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
#define WAKING_TO_RECEIVE_CODE			8000
#define WAKING_TO_SEND_CODE		 		9000
#define WAIT_DATA_EVENT_CODE			10000
#define SEND_DATA_CODE					20000
#define SEND_ACK_CODE					30000
#define RECOMPUTE_SEED_CODE				40000
#define SLEEP_FROM_RECEIVE_CODE 		50000
#define SLEEP_FROM_SEND_CODE			60000
#define SEND_BEACON_CODE				70000

#define END	        		    	(op_intrpt_type() == OPC_INTRPT_ENDSIM)
#define FROM_LOWER					((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == RCV_STRM))
#define FROM_UPPER					((op_intrpt_type() == OPC_INTRPT_STRM) && (op_intrpt_strm() == SRC_STRM))
#define START_GRADE					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == START_GRADE_CODE))
#define SEND_GM                 	((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_GM_CODE))

#define START_SCHEDULE				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == START_SCHEDULE_CODE))
//#define WAKING_TO_WAIT_RTS			(START_SCHEDULE && grade>=0 && grade%(sleep_factor+2)==0)
//#define CONTEND_TO_SEND_RTS			(START_SCHEDULE && grade>=0 && grade%(sleep_factor+2)==1)
//#define GO_TO_SLEEP					(START_SCHEDULE && grade>=0 && grade%(sleep_factor+2)>=2)

#define WAKING_TO_RECEIVE			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == WAKING_TO_RECEIVE_CODE))
#define WAKING_TO_SEND				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == WAKING_TO_SEND_CODE))
#define NO_DATA						((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == WAIT_DATA_EVENT_CODE))
#define RECOMPUTE_SEED				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == RECOMPUTE_SEED_CODE))
#define SEND_BEACON					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_BEACON_CODE))
#define SEND_DATA					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_DATA_CODE))
//#define SEND_CTS					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_CTS_CODE))
#define SEND_ACK					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_ACK_CODE))
#define SLEEP_FROM_RECEIVE			((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SLEEP_FROM_RECEIVE_CODE))
#define SLEEP_FROM_SEND				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SLEEP_FROM_SEND_CODE))
#define CHECK_CHANNEL				((op_intrpt_type() == OPC_INTRPT_STAT))

typedef struct
{
	int rt_next_hop;
	int rt_grade;
}RoutingTable;

typedef struct
{
	int ps_id;//node address of R
	int ps_rs;//randState[R], 2 bytes
	double ps_td;//timeDifference[R], 4 bytes
	double ps_next_wt;//last wakeup time: nextWakeupTime[R], 4 bytes
	double ps_lastupdate;//denotes the time S received the prediction state from R, which is used to compute Td, the prediction error caused by clock drift
}PredictionState;

typedef struct
{
	int st_data_id;
}DataID;


//function prototype
static Boolean data_id_exist(int id);
static void grade_pk_proc(Packet * pk);
static int RandNum(int,int);
static PredictionState * find_R_in_ps(int);
static void upper_data_pk_proc(void);
static void power_stat(void);
static void record_grade(void);
static double compute_wakeup_time(void);

int retx=0; //the number of retransmission
int data_nums_1 = 0;
double avg_energy_consumption = 0.0;
double avg_duty_cycle=0.0;

//int data_nums = 0;

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
	Boolean	                		Q_empty                                         ;
	Boolean	                		transceiver_state                               ;
	double	                 		A                                               ;
	double	                 		all_active_time                                 ;
	double	                 		all_rx_time                                     ;
	double	                 		all_sleeped_time                                ;
	double	                 		all_tx_time                                     ;
	double	                 		CW                                              ;	/* Contention Window */
	double	                 		durBEACON                                       ;
	double	                 		durDATA                                         ;
	double	                 		record_time                                     ;
	double	                 		RWaking_time                                    ;	/* record the time when the node wakes up for receiving by sending Beacon */
	double	                 		send_gm_timer                                   ;
	double	                 		start_grade_timer                               ;
	double	                 		start_schedule_timer                            ;
	double	                 		VRWaking_time                                   ;	/* Virtual Waking time of receiver */
	Evhandle	               		goto_SLEEP_event                                ;
	Evhandle	               		send_gm_event                                   ;
	Evhandle	               		virtual_to_receive_event                        ;	/* virtual wake to receive event */
	Evhandle	               		wait_DATA_event                                 ;
	Evhandle	               		wake_to_receive_event                           ;
	Evhandle	               		wake_to_send_event                              ;
	int	                    		ack_next_hop                                    ;
	int	                    		c                                               ;
	int	                    		coef_a                                          ;	/* the coefficient of parameter "a", e.g., 20 */
	int	                    		grade                                           ;
	int	                    		m                                               ;
	int	                    		data_next_hop                                   ;
	int	                    		node_address                                    ;
	int	                    		node_type                                       ;
	int	                    		randState                                       ;	/* Xn */
	List *	                 		prediction_state_list                           ;
	List *	                 		routing_table_list                              ;
	Objid	                  		node_id                                         ;
	Objid	                  		process_id                                      ;
	Objid	                  		rx_id                                           ;
	Evhandle	               		send_DATA_event                                 ;
	Evhandle	               		wait_ACK_event                                  ;
	double	                 		DIFS                                            ;
	double	                 		SIFS                                            ;
	double	                 		wakeinterval                                    ;
	Boolean	                		debug                                           ;
	Evhandle	               		send_BEACON_event                               ;
	Evhandle	               		send_ACK_event                                  ;
	Objid	                  		tx_id                                           ;
	Objid	                  		tx_channel_objid                                ;
	double	                 		data_rate                                       ;
	List *	                 		data_id_list                                    ;
	int	                    		range                                           ;
	} pw_mac_state;

#define Q_empty                 		op_sv_ptr->Q_empty
#define transceiver_state       		op_sv_ptr->transceiver_state
#define A                       		op_sv_ptr->A
#define all_active_time         		op_sv_ptr->all_active_time
#define all_rx_time             		op_sv_ptr->all_rx_time
#define all_sleeped_time        		op_sv_ptr->all_sleeped_time
#define all_tx_time             		op_sv_ptr->all_tx_time
#define CW                      		op_sv_ptr->CW
#define durBEACON               		op_sv_ptr->durBEACON
#define durDATA                 		op_sv_ptr->durDATA
#define record_time             		op_sv_ptr->record_time
#define RWaking_time            		op_sv_ptr->RWaking_time
#define send_gm_timer           		op_sv_ptr->send_gm_timer
#define start_grade_timer       		op_sv_ptr->start_grade_timer
#define start_schedule_timer    		op_sv_ptr->start_schedule_timer
#define VRWaking_time           		op_sv_ptr->VRWaking_time
#define goto_SLEEP_event        		op_sv_ptr->goto_SLEEP_event
#define send_gm_event           		op_sv_ptr->send_gm_event
#define virtual_to_receive_event		op_sv_ptr->virtual_to_receive_event
#define wait_DATA_event         		op_sv_ptr->wait_DATA_event
#define wake_to_receive_event   		op_sv_ptr->wake_to_receive_event
#define wake_to_send_event      		op_sv_ptr->wake_to_send_event
#define ack_next_hop            		op_sv_ptr->ack_next_hop
#define c                       		op_sv_ptr->c
#define coef_a                  		op_sv_ptr->coef_a
#define grade                   		op_sv_ptr->grade
#define m                       		op_sv_ptr->m
#define data_next_hop           		op_sv_ptr->data_next_hop
#define node_address            		op_sv_ptr->node_address
#define node_type               		op_sv_ptr->node_type
#define randState               		op_sv_ptr->randState
#define prediction_state_list   		op_sv_ptr->prediction_state_list
#define routing_table_list      		op_sv_ptr->routing_table_list
#define node_id                 		op_sv_ptr->node_id
#define process_id              		op_sv_ptr->process_id
#define rx_id                   		op_sv_ptr->rx_id
#define send_DATA_event         		op_sv_ptr->send_DATA_event
#define wait_ACK_event          		op_sv_ptr->wait_ACK_event
#define DIFS                    		op_sv_ptr->DIFS
#define SIFS                    		op_sv_ptr->SIFS
#define wakeinterval            		op_sv_ptr->wakeinterval
#define debug                   		op_sv_ptr->debug
#define send_BEACON_event       		op_sv_ptr->send_BEACON_event
#define send_ACK_event          		op_sv_ptr->send_ACK_event
#define tx_id                   		op_sv_ptr->tx_id
#define tx_channel_objid        		op_sv_ptr->tx_channel_objid
#define data_rate               		op_sv_ptr->data_rate
#define data_id_list            		op_sv_ptr->data_id_list
#define range                   		op_sv_ptr->range

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	pw_mac_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((pw_mac_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

void update_routing_table(int node_hop,int node_grade)
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
	int pre_hop,grade_in_pk;//,sink_id_in_pk;
//in
	FIN(grade_pk_proc(pk));
//body
	op_pk_nfd_get(pk,"Grade",&grade_in_pk);
	op_pk_nfd_get(pk,"Previous Hop",&pre_hop);
	//op_pk_nfd_get(pk,"Sink ID",&sink_id_in_pk);
	
	if(grade<0)
	{
		grade = grade_in_pk;
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
		//send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer), SEND_GM_CODE);
		if(node_address%2==0)
		{	
			send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer)+0.5, SEND_GM_CODE);
		}else{
			send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer)+0.1, SEND_GM_CODE);
		}
		
		FOUT;
	}
	
	if(grade_in_pk < grade)
	{
		grade = grade_in_pk;
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
		//send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer), SEND_GM_CODE);
		if(node_address%2==0)
		{	
			send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer)+0.5, SEND_GM_CODE);
		}else{
			send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer)+0.1, SEND_GM_CODE);
		}
		
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

static double compute_wakeup_time(void)//Compute Sender Wakeup Time
{
//var
	PredictionState * func_ps=NULL;
	double curTime;
	int i,ps_size;
//in
	FIN(compute_wakeup_time(void));
//body
	ps_size=op_prg_list_size(prediction_state_list);
	if(ps_size>0)
	{
		i = op_dist_outcome(op_dist_load("uniform_int",0,ps_size-1));
		func_ps = op_prg_list_access(prediction_state_list,i);
		
		//compute the current time of R
		curTime=op_sim_time();//-func_ps->ps_td;//curTime[R]=localTime
		
		//if next wakeup time of S for sending to R has been computed before
		if(func_ps->ps_next_wt>curTime)
		{
			FRET(func_ps->ps_next_wt-curTime);
		}
		
		while(func_ps->ps_next_wt<=curTime){
			func_ps->ps_rs=RandNum(func_ps->ps_rs,func_ps->ps_id);
			func_ps->ps_next_wt+=func_ps->ps_rs/1000.0;
		}
		
		if(func_ps->ps_next_wt>(curTime)){
			FRET(func_ps->ps_next_wt-(curTime));
		}else{
			FRET(0.0);
		}
	}
//out
	FRET(0.0);//wakeup now to wait R
}

//find R(R_id) in prediction state table to get its prediction state
static PredictionState * find_R_in_ps(int R_id)
{
//var
	PredictionState * func_ps=NULL;
	int i,ps_size;
//in
	FIN(find_R_in_ps(int));
//body
	ps_size=op_prg_list_size(prediction_state_list);
	for(i=0;i<ps_size;i++)
	{
		func_ps=(PredictionState *)op_prg_list_access(prediction_state_list,i);
		if(func_ps->ps_id==R_id)
			break;
	}
//out
	FRET(func_ps);
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
	op_pk_nfd_set(upper_pk,"Grade",grade);//for test
	
	op_subq_pk_insert (data_queue,upper_pk,OPC_QPOS_TAIL);
	
	if(debug)
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
	int nodes_num,sink_num;
//in
	FIN(power_stat(void));
//vody
	op_ima_obj_attr_get(node_id,"Power Record File",temp_file_name);
	if(strcmp(temp_file_name,"disable")!=0)
	{	
		avg_energy_consumption += all_sleeped_time*0.05+(all_active_time-(all_tx_time+all_rx_time))*0.45+(all_tx_time+all_rx_time)*0.5;
		avg_duty_cycle += all_active_time/(all_sleeped_time+all_active_time);
		nodes_num = op_topo_object_count (OPC_OBJTYPE_NDMOB);
		op_ima_obj_attr_get(process_id, "Sink Num",&sink_num);
		if(node_address == nodes_num-sink_num){
			in = fopen(temp_file_name,"at");
	
			//fprintf(in,"%d %f %f %f\r\n",node_address,all_sleeped_time*0.05+(all_active_time-(all_tx_time+all_rx_time))*0.45+(all_tx_time+all_rx_time)*0.5,all_active_time/(all_sleeped_time+all_active_time),(all_sleeped_time+all_active_time));
			
			fprintf(in,"%d %f %f %d\r\n",node_address,avg_energy_consumption/(nodes_num*(all_sleeped_time+all_active_time)),(avg_duty_cycle/nodes_num)*100,nodes_num);
		
			fclose(in);
		}
	}	
//out
	FOUT;
}



static int RandNum(int rs,int id)//rs: randState[R],id: node ID
{
//var
	int func_rs;
//in
	FIN(RandNum(int,int));
//body
	func_rs=((id*coef_a+1)*rs+c) % m;
	while(func_rs<500 || func_rs>1500)//(500~1500)
	{
		func_rs=((id*coef_a+1)*func_rs+c) % m;
	}
//out
	FRET(func_rs);
}

static void record_grade(void)
{
//var
	//int i,rt_size;
	//RoutingTable* rt;
	FILE *file_in;
	char file_name[300];
//in
	FIN(record_grade(void));
//body
	op_ima_obj_attr_get(node_id,"Grade Record File",file_name);
	
	file_in = fopen(file_name,"at");
	fprintf(file_in,"%d	%d\r\n",\
		node_address,grade);
	/*
	rt_size = op_prg_list_size(routing_table_list);
	for(i=0;i<rt_size;i++)
	{
		rt = (RoutingTable *)op_prg_list_access(routing_table_list,i);
		fprintf(file_in,"    %d, 级别是:%d. \r\n",\
			rt->rt_next_hop, rt->rt_grade);
	}
	fprintf(file_in,"\r\n");
	*/
	fclose(file_in);
	
	if(node_type==sink && node_address==-1)
	{
		op_ima_obj_attr_get(process_id,"Retx File",file_name);
		file_in = fopen(file_name,"at");
		fprintf(file_in,"%d	%d %f\r\n",\
		retx,data_nums_1,(double)retx/(double)data_nums_1);
		fclose(file_in);
	}
//out
	FOUT;
}

int rt_pos_find_next()
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
	FRET(-1);
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
	void pw_mac (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_pw_mac_init (int * init_block_ptr);
	void _op_pw_mac_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_pw_mac_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_pw_mac_alloc (VosT_Obtype, int);
	void _op_pw_mac_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
pw_mac (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (pw_mac ());

		{
		/* Temporary Variables */
		Packet* pk_TV=NULL;
		Packet* pk_ack_TV=NULL;
		int pk_type;
		double time;
		int dest_TV;
		int psFlag_TV;
		int grade_TV;
		int hop_num_TV;
		double CW_TV;
		int cur_seed_TV;
		double creat_time_TV;
		double RWaking_time_TV;
		int pk_size;
		//File
		FILE *in;
		char temp_file_name[300];
		//char message[100];
		int data_id;
		
		DataID * id_TV;
		/* End of Temporary Variables */


		FSM_ENTER ("pw_mac")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "pw_mac [init enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [init enter execs]", state0_enter_exec)
				{
				// Obtain related ID
				process_id = op_id_self();
				node_id = op_topo_parent(process_id);
				rx_id = op_id_from_name(node_id,OPC_OBJTYPE_RARX,"rx");//the name of receiver radio in node layer called "rx"
				tx_id = op_id_from_name(node_id,OPC_OBJTYPE_RATX ,"tx");//the name of receiver radio in node layer called "tx"
				
				//obtain tx channel of compound type
				op_ima_obj_attr_get_objid (tx_id, "channel",&tx_channel_objid);
				
				tx_channel_objid = op_topo_child (tx_channel_objid, OPC_OBJTYPE_RATXCH, 0);
				op_ima_obj_attr_get(tx_channel_objid,"data rate",&data_rate);
				
				op_ima_obj_attr_get(process_id, "Debug",&debug);
				
				if(debug)
					printf("data_rate=%f\n",data_rate);
				
				pk_TV = op_pk_create_fmt("MAC_BEACON_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durBEACON=(pk_size+10)/data_rate;
				op_pk_destroy(pk_TV);
				if(debug)
					printf("durBEACON=%f\n",durBEACON);
				
				pk_TV = op_pk_create_fmt("MAC_DATA_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durDATA=(pk_size+10)/data_rate;
				op_pk_destroy(pk_TV);
				if(debug)
					printf("durDATA=%f\n",durDATA);
				
				//obtain parameter
				op_ima_obj_attr_get(process_id, "Start Grade Timer", &start_grade_timer);		//1
				op_ima_obj_attr_get(process_id, "Send GM Timer", &send_gm_timer);				//2
				op_ima_obj_attr_get(process_id, "Start Schedule Timer", &start_schedule_timer);	//3
				//op_ima_obj_attr_get(process_id, "durDATA",&durDATA);
				//op_ima_obj_attr_get(process_id, "durBEACON",&durBEACON);
				op_ima_obj_attr_get(process_id, "COEF_A",&coef_a);
				op_ima_obj_attr_get(process_id, "C",&c);
				op_ima_obj_attr_get(process_id, "M",&m);
				op_ima_obj_attr_get(process_id, "A",&A);
				
				
				//op_ima_obj_attr_get(process_id, "Sleep Factor", &sleep_factor);					//4
				//op_ima_obj_attr_get(process_id, "CW",&CW);										//5
				//op_ima_obj_attr_get(process_id, "durRTS",&durRTS);								//6
				//op_ima_obj_attr_get(process_id, "durCTS",&durCTS);								//7
				//op_ima_obj_attr_get(process_id, "durDATA",&durDATA);							//8
				//op_ima_obj_attr_get(process_id, "durACK",&durACK);								//9
				op_ima_obj_attr_get(process_id, "DIFS",&DIFS);									//10
				op_ima_obj_attr_get(process_id, "SIFS",&SIFS);									//11
				
				op_ima_obj_attr_get(node_id, "user id", &node_address);
				op_ima_obj_attr_get(node_id, "Node Type",&node_type);
				
				//op_ima_obj_attr_get(process_id, "Communication Range",&range);
				//op_ima_obj_attr_set(node_id, "Communication Range",range);
				
				op_ima_obj_attr_get(rx_id, "state", &transceiver_state);
				
				
				prediction_state_list=op_prg_list_create();
				routing_table_list=op_prg_list_create();
				data_id_list=op_prg_list_create();
				
				grade = -1;
				//sink_id = -1;
				
				all_sleeped_time = 0;
				all_active_time = 0;
				all_tx_time = 0;
				all_rx_time = 0;
				
				//data_nums=0;
				//durActive contains two parts: durReceive & durSedn, and durReceive==durSend
				//durActive_half = 2*CW+durRTS+durCTS+durDATA+durACK+2*DIFS+2*SIFS;
				//durSleep = sleep_factor * durActive_half;
				if(node_type == sink)
				{
					if(node_address==-1)
					{
						op_intrpt_schedule_self(op_sim_time() + start_grade_timer,START_GRADE_CODE);
					}else{
						op_intrpt_schedule_self(op_sim_time() + start_grade_timer+1.0, START_GRADE_CODE);
					}
				}	
				
				op_intrpt_schedule_self(op_sim_time() + start_schedule_timer, START_SCHEDULE_CODE);
				
				//op_ima_obj_attr_get (process_id, "subqueue", &subq_objid);
				//num_subqs = op_topo_child_count (subq_objid, OPC_OBJTYPE_SUBQ);
				//printf("子队列个数:%d\n",num_subqs);
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "pw_mac [init exit execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [init exit execs]", state0_exit_exec)
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
					
					//fprintf(in,"Begin:\r\ndurActive_half: %f, durSleep: %f. cycle time: (%d+2)*%f=%f.(in \"gmac->init\")\r\n",\
					//	durActive_half,durSleep,sleep_factor,durActive_half,(sleep_factor+2)*durActive_half);
					//fprintf(in,"duty cycle:%f.\r\n",\
					//		durActive_half*2/(durActive_half*2+durSleep));
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
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle_0", "tr_115", "pw_mac [init -> idle_0 : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle_0) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle_0", state1_enter_exec, "pw_mac [idle_0 enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [idle_0 enter execs]", state1_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(debug)
					printf("Node:%d, time:%f. In \"idle\" state: received DATA which is from \"gsrc\".\n",\
							node_address,op_sim_time());
					upper_data_pk_proc();
				}
				
				if(END)
				{
					record_grade();
					//time statistic
					//op_prg_list_free (NAV_list);
					//op_prg_mem_free (NAV_list);
					
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
					
					op_prg_list_free (prediction_state_list);
					op_prg_mem_free (prediction_state_list);
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
				}
				}
				FSM_PROFILE_SECTION_OUT (state1_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"pw_mac")


			/** state (idle_0) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle_0", "pw_mac [idle_0 exit execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [idle_0 exit execs]", state1_exit_exec)
				{
				if(START_SCHEDULE) //start PW_MAC
				{
					randState=op_dist_outcome(op_dist_load("uniform_int",500,1500));
					wakeinterval= randState/1000.0;
					record_time = op_sim_time();
					
					if(op_subq_empty(data_queue) == OPC_FALSE)//data queue is not empty
					{
						Q_empty=OPC_FALSE;
						VRWaking_time=op_sim_time();//virtual Waking time of receiver
						virtual_to_receive_event=op_intrpt_schedule_self(op_sim_time() + wakeinterval,RECOMPUTE_SEED_CODE);
						
						if(debug)
							printf("Node:%d, Grade:%d, time:%f,in idle_0. Data queue is not empty.\nGoing to SWaking From START_SCHEDULE && ! Q_empty.\nSet virtual_to_receive_event (wakeinterval=%f).\n", node_address,grade,op_sim_time(),wakeinterval);
					}
					else//data queue is empty, go to sleep
					{
						Q_empty=OPC_TRUE;
						RWaking_time=op_sim_time();
					    wake_to_receive_event=op_intrpt_schedule_self(op_sim_time() + wakeinterval, WAKING_TO_RECEIVE_CODE);
						
						if(debug)
						printf("Node:%d, Grade:%d, time:%f,in idle_0. Data queue is empty.\nGoing to sleeping From START_SCHEDULE && Q_empty.\nAfter %f seconds, wake to receive.\n",\
							node_address,grade,op_sim_time(),wakeinterval);
					}
					
				}
				}
				FSM_PROFILE_SECTION_OUT (state1_exit_exec)


			/** state (idle_0) transition processing **/
			FSM_PROFILE_SECTION_IN ("pw_mac [idle_0 trans conditions]", state1_trans_conds)
			FSM_INIT_COND (START_GRADE)
			FSM_TEST_COND (FROM_LOWER && transceiver_state)
			FSM_TEST_COND (SEND_GM)
			FSM_TEST_COND (START_SCHEDULE && Q_empty)
			FSM_TEST_COND (START_SCHEDULE && (!Q_empty))
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle_0")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "START_GRADE", "", "idle_0", "start grade", "tr_97", "pw_mac [idle_0 -> start grade : START_GRADE / ]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "idle_0", "GM proc", "tr_102", "pw_mac [idle_0 -> GM proc : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "SEND_GM", "", "idle_0", "send gm", "tr_100", "pw_mac [idle_0 -> send gm : SEND_GM / ]")
				FSM_CASE_TRANSIT (3, 5, state5_enter_exec, ;, "START_SCHEDULE && Q_empty", "", "idle_0", "sleeping", "tr_117", "pw_mac [idle_0 -> sleeping : START_SCHEDULE && Q_empty / ]")
				FSM_CASE_TRANSIT (4, 6, state6_enter_exec, ;, "START_SCHEDULE && (!Q_empty)", "", "idle_0", "SWaking", "tr_120", "pw_mac [idle_0 -> SWaking : START_SCHEDULE && (!Q_empty) / ]")
				FSM_CASE_TRANSIT (5, 1, state1_enter_exec, ;, "default", "", "idle_0", "idle_0", "tr_37", "pw_mac [idle_0 -> idle_0 : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (start grade) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "start grade", state2_enter_exec, "pw_mac [start grade enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [start grade enter execs]", state2_enter_exec)
				{
				//Sink will execute the following code
				grade = 0;
				//sink_id = node_address;
				
				pk_TV = op_pk_create_fmt("MAC_GRADE_PK");
				op_pk_nfd_set(pk_TV, "Type", grade_pk);
				//op_pk_nfd_set(pk_TV, "Sink ID", sink_id);
				op_pk_nfd_set(pk_TV, "Grade", grade+1);
				op_pk_nfd_set(pk_TV,"Previous Hop",node_address);
				
				op_pk_send(pk_TV, SEND_STRM);
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (start grade) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "start grade", "pw_mac [start grade exit execs]")


			/** state (start grade) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "start grade", "idle_0", "tr_98", "pw_mac [start grade -> idle_0 : default / ]")
				/*---------------------------------------------------------*/



			/** state (GM proc) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "GM proc", state3_enter_exec, "pw_mac [GM proc enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [GM proc enter execs]", state3_enter_exec)
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
					op_sim_end("Waiting Grade Message Err: Are you kidding me?","I'm waiting Grade Message","Not you","");
				
				}
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (GM proc) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "GM proc", "pw_mac [GM proc exit execs]")


			/** state (GM proc) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "GM proc", "idle_0", "tr_52", "pw_mac [GM proc -> idle_0 : default / ]")
				/*---------------------------------------------------------*/



			/** state (send gm) enter executives **/
			FSM_STATE_ENTER_FORCED (4, "send gm", state4_enter_exec, "pw_mac [send gm enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [send gm enter execs]", state4_enter_exec)
				{
				if(op_subq_empty(grade_queue) == OPC_FALSE)
				{
					pk_TV = op_subq_pk_remove(grade_queue, OPC_QPOS_HEAD);
					op_pk_send(pk_TV,SEND_STRM);
				}
				else
				{
					op_sim_end("Err: ack queue is empty, simulation is over!","","","");
				}
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** state (send gm) exit executives **/
			FSM_STATE_EXIT_FORCED (4, "send gm", "pw_mac [send gm exit execs]")


			/** state (send gm) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "send gm", "idle_0", "tr_101", "pw_mac [send gm -> idle_0 : default / ]")
				/*---------------------------------------------------------*/



			/** state (sleeping) enter executives **/
			FSM_STATE_ENTER_UNFORCED (5, "sleeping", state5_enter_exec, "pw_mac [sleeping enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [sleeping enter execs]", state5_enter_exec)
				{
				//close transceiver
				transceiver_state = OPC_FALSE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				
				if(START_SCHEDULE && Q_empty)//data queue is empty
				{
					CW=0;//initialize Contention Window
					//randState=op_dist_outcome(op_dist_load("uniform_int",500,1500));
					//wakeinterval= randState/1000.0;
					//wake_to_receive_event=op_intrpt_schedule_self(op_sim_time() + wakeinterval, WAKING_TO_RECEIVE_CODE);
					//printf("Node:%d, Grade:%d, time:%f,in sleeping period.\n\
					//		Initial to sleeping From START_SCHEDULE && Q_empty.\n\
					//		After %f seconds, wake to receive.\n",\
					//		node_address,grade,op_sim_time(),wakeinterval);
				}else
					
				if(SLEEP_FROM_RECEIVE){//check data queue
					wakeinterval= randState/1000.0;
					time=RWaking_time+wakeinterval-op_sim_time();
					
					if(op_ev_valid(wake_to_receive_event)){//error
						    op_sim_end("仿真出错","In sleeping state (SLEEP_FROM_RECEIVE):","请反思","wake_to_receive_event 应为无效!");
							op_ev_cancel(wake_to_receive_event);		
					}
					wake_to_receive_event=op_intrpt_schedule_self(op_sim_time() + time, WAKING_TO_RECEIVE_CODE);
					
					if(debug)
						printf("Node:%d, Grade:%d, time:%f,in sleeping period.\nTo sleeping from RECEIVE.\nCheck data queue.\n",node_address,grade,op_sim_time());
					//check data queue
					if(op_subq_empty(data_queue) == OPC_FALSE){//data queue is not empty
					
						op_ev_cancel(wake_to_receive_event);//won't be as a receiver
						//still need to calculate seed
						wakeinterval= randState/1000.0;
						time=RWaking_time+wakeinterval-op_sim_time();
						VRWaking_time=RWaking_time;
						virtual_to_receive_event=op_intrpt_schedule_self(op_sim_time() + time,RECOMPUTE_SEED_CODE);
							
						time=compute_wakeup_time();
						if(time-A<0){
							time=0;
						}else{
							time=time-A;
						}
						wake_to_send_event=op_intrpt_schedule_self(op_sim_time() + time, WAKING_TO_SEND_CODE);
						
						if(debug)
							printf("Data queue is not empty. Set virtual_to_receive_event.\nAfter %f seconds, wake to send (time:%f).\n",time,op_sim_time()+time);
					}else{//data queue is empty
						if(debug)
							printf("Data queue is empty. Set wake_to_receive_event. \nAfter %f seconds, wake to receive (time:%f).\n",time,op_sim_time()+time);
					}
					
					//time statistic
					all_active_time = all_active_time+(op_sim_time()-record_time);
					record_time = op_sim_time();
					
				}else
				if(SLEEP_FROM_SEND){
					//check data queue
					if(op_subq_empty(data_queue) == OPC_FALSE){//data queue is not empty
						time=compute_wakeup_time();
						if(time-A<0){
							time=0;
						}else{
							time=time-A;
						}
						wake_to_send_event=op_intrpt_schedule_self(op_sim_time() + time, WAKING_TO_SEND_CODE);
						
						if(debug)
							printf("Node:%d, Grade:%d, time:%f,in sleeping period.\nTo sleeping from SEND. Data Queue is still not empty.\nAfter %f seconds, wake to send (time: %f).\n",node_address,grade,op_sim_time(),time,op_sim_time()+time);
						
					}else{//data queue is empty, wakeup to as a receiver
						if(op_ev_valid(virtual_to_receive_event)){
							op_ev_cancel(virtual_to_receive_event);
							wakeinterval= randState/1000.0;
							time=VRWaking_time+wakeinterval-op_sim_time();
							RWaking_time=VRWaking_time;
							
							wake_to_receive_event=op_intrpt_schedule_self(op_sim_time() + time, WAKING_TO_RECEIVE_CODE);
							
							if(debug)
							printf("Node:%d, Grade:%d, time:%f,in sleeping period.\nTo sleeping from SEND. Data Queue is empty.\nAfter %f seconds, wake to receive.\n",\
								node_address,grade,op_sim_time(),time);
						}else{
							op_sim_end("仿真出错","In sleeping state (SLEEP_FROM_SEND): virtual_to_receive_event 异常失效!","请反思","");
						}
					}
					
					//time statistic
					all_active_time = all_active_time+(op_sim_time()-record_time);
					record_time = op_sim_time();
				}
				
				
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(debug)
					printf("1--Node: %d,Grade:%d. time=%f. In \"sleeping\" state (FROM_UPPER): received DATA which is from \"gsrc\".\n",node_address,grade,op_sim_time());
					upper_data_pk_proc();
					
					//will wake up to receive
					if(op_ev_valid(wake_to_receive_event))//cancel WAKING_TO_RECEIVE event
					{
						op_ev_cancel(wake_to_receive_event);
						//still need to calculate seed
						wakeinterval= randState/1000.0;
						time = RWaking_time+wakeinterval-op_sim_time();
						VRWaking_time=RWaking_time;
						virtual_to_receive_event=op_intrpt_schedule_self(op_sim_time() + time,RECOMPUTE_SEED_CODE);
						if(debug)
							printf("2--randState=%d, cancel wake_to_receive_event.\nSet virtual_to_receive_event.\n After time=%f seconds, set next virtual_to_receive_event (time: %f)\n",randState,time,op_sim_time()+time);
						
						if(!op_ev_valid(wake_to_send_event)){
							
							time = compute_wakeup_time();
							if(time-A<0){
								time=0;
							}else{
								time=time-A;
							}
							
							wake_to_send_event=op_intrpt_schedule_self(op_sim_time() + time, WAKING_TO_SEND_CODE);
							if(debug)
								printf("3--Set wake_to_send_event.\n After time=%f seconds, go to RWakint state to send data(time: %f)\n",time,op_sim_time()+time);
				
							
						}else{
							op_sim_end("仿真出错","In sleeping state (FROM_UPPER):","请反思","wake_to_receive_event 和 wake_to_send_event不可能同时有效!");
						}
					}else if(!op_ev_valid(wake_to_send_event)){
						
						op_sim_end("仿真出错","In sleeping state (FROM_UPPER):","请反思","wake_to_receive_event已经无效，那么wake_to_send_event应该为有效!");
					}
					
				}
				
				if(FROM_LOWER && !transceiver_state)
				{
					op_pk_destroy(op_pk_get(RCV_STRM));
					
					if(debug)
					printf("Node:%d, time:%f. Node is sleeping, destroy the received pk.\n",\
							node_address,op_sim_time());
				}
				if(END)
				{
				
					//time statistic
					all_sleeped_time = all_sleeped_time + (op_sim_time()-record_time);
					power_stat();
					record_grade();
					
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
					
					op_prg_list_free (prediction_state_list);
					op_prg_mem_free (prediction_state_list);
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
				}
				}
				FSM_PROFILE_SECTION_OUT (state5_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (11,"pw_mac")


			/** state (sleeping) exit executives **/
			FSM_STATE_EXIT_UNFORCED (5, "sleeping", "pw_mac [sleeping exit execs]")


			/** state (sleeping) transition processing **/
			FSM_PROFILE_SECTION_IN ("pw_mac [sleeping trans conditions]", state5_trans_conds)
			FSM_INIT_COND (WAKING_TO_SEND)
			FSM_TEST_COND (WAKING_TO_RECEIVE)
			FSM_TEST_COND (RECOMPUTE_SEED)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("sleeping")
			FSM_PROFILE_SECTION_OUT (state5_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "WAKING_TO_SEND", "", "sleeping", "SWaking", "tr_124", "pw_mac [sleeping -> SWaking : WAKING_TO_SEND / ]")
				FSM_CASE_TRANSIT (1, 8, state8_enter_exec, ;, "WAKING_TO_RECEIVE", "", "sleeping", "RWaking", "tr_127", "pw_mac [sleeping -> RWaking : WAKING_TO_RECEIVE / ]")
				FSM_CASE_TRANSIT (2, 13, state13_enter_exec, ;, "RECOMPUTE_SEED", "", "sleeping", "RecmpSeed1", "tr_136", "pw_mac [sleeping -> RecmpSeed1 : RECOMPUTE_SEED / ]")
				FSM_CASE_TRANSIT (3, 5, state5_enter_exec, ;, "default", "", "sleeping", "sleeping", "tr_138", "pw_mac [sleeping -> sleeping : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (SWaking) enter executives **/
			FSM_STATE_ENTER_FORCED (6, "SWaking", state6_enter_exec, "pw_mac [SWaking enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [SWaking enter execs]", state6_enter_exec)
				{
				//enable transceiver
				if(debug){
					if(START_SCHEDULE)
						printf("Node:%d, Grade:%d, time:%f,in SWaking state from idle_0.\nFor sending data since data queue is not empty.\nAfter open transceiver, go to idle1 for waiting for Beacon.\n",	node_address,grade,op_sim_time(),wakeinterval);
					else if(WAKING_TO_SEND)
						printf("Node:%d, Grade:%d, time:%f,in SWaking state from sleeping.\nFor sending data since data queue is not empty.\nAfter open transceiver, go to idle1 for waiting for Beacon.\n",	node_address,grade,op_sim_time(),wakeinterval);
				}
				transceiver_state = OPC_TRUE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				
				//time statistic
				all_sleeped_time = all_sleeped_time + (op_sim_time() - record_time);
				record_time = op_sim_time();
				}
				FSM_PROFILE_SECTION_OUT (state6_enter_exec)

			/** state (SWaking) exit executives **/
			FSM_STATE_EXIT_FORCED (6, "SWaking", "pw_mac [SWaking exit execs]")


			/** state (SWaking) transition processing **/
			FSM_TRANSIT_FORCE (7, state7_enter_exec, ;, "default", "", "SWaking", "idle1", "tr_121", "pw_mac [SWaking -> idle1 : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle1) enter executives **/
			FSM_STATE_ENTER_UNFORCED (7, "idle1", state7_enter_exec, "pw_mac [idle1 enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [idle1 enter execs]", state7_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(debug)
						printf("Node:%d, Grade:%d, time:%f,In \"idle1\" state: received DATA which is from \"gsrc\".\n",node_address,grade,op_sim_time());
				
					upper_data_pk_proc();
				}
				
				if(END)
				{
				
					//time statistic
					all_active_time = all_active_time+(op_sim_time()-record_time);
					power_stat();
					record_grade();
					
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
					
					op_prg_list_free (prediction_state_list);
					op_prg_mem_free (prediction_state_list);
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
				}
				}
				FSM_PROFILE_SECTION_OUT (state7_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (15,"pw_mac")


			/** state (idle1) exit executives **/
			FSM_STATE_EXIT_UNFORCED (7, "idle1", "pw_mac [idle1 exit execs]")


			/** state (idle1) transition processing **/
			FSM_PROFILE_SECTION_IN ("pw_mac [idle1 trans conditions]", state7_trans_conds)
			FSM_INIT_COND (SLEEP_FROM_SEND)
			FSM_TEST_COND (RECOMPUTE_SEED)
			FSM_TEST_COND (FROM_LOWER && transceiver_state)
			FSM_TEST_COND (SEND_DATA)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle1")
			FSM_PROFILE_SECTION_OUT (state7_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 5, state5_enter_exec, ;, "SLEEP_FROM_SEND", "", "idle1", "sleeping", "tr_125", "pw_mac [idle1 -> sleeping : SLEEP_FROM_SEND / ]")
				FSM_CASE_TRANSIT (1, 14, state14_enter_exec, ;, "RECOMPUTE_SEED", "", "idle1", "RecmpSeed2", "tr_139", "pw_mac [idle1 -> RecmpSeed2 : RECOMPUTE_SEED / ]")
				FSM_CASE_TRANSIT (2, 15, state15_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "idle1", "RcvBeacon", "tr_141", "pw_mac [idle1 -> RcvBeacon : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (3, 16, state16_enter_exec, ;, "SEND_DATA", "", "idle1", "send DATA", "tr_143", "pw_mac [idle1 -> send DATA : SEND_DATA / ]")
				FSM_CASE_TRANSIT (4, 7, state7_enter_exec, ;, "default", "", "idle1", "idle1", "tr_145", "pw_mac [idle1 -> idle1 : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (RWaking) enter executives **/
			FSM_STATE_ENTER_FORCED (8, "RWaking", state8_enter_exec, "pw_mac [RWaking enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [RWaking enter execs]", state8_enter_exec)
				{
				//enable transceiver
				transceiver_state = OPC_TRUE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				RWaking_time=op_sim_time();
				
				//the node is awake in the current duty cycle. 
				//compute the current interval
				randState=RandNum(randState,node_address);
				wakeinterval=randState/1000.0;
				goto_SLEEP_event=op_intrpt_schedule_self(op_sim_time() + wakeinterval,SLEEP_FROM_RECEIVE_CODE);
				
				
				
				time=SIFS;
				send_BEACON_event = op_intrpt_schedule_self(op_sim_time()+time,SEND_BEACON_CODE);
				
				if(debug)
					printf("Node:%d, Grade:%d, time:%f,Waking to receive. Waiting SIFS to send Beacon First.\nThe current wakeup interval is %f seconds.\n", node_address,grade,op_sim_time(),wakeinterval);
				
				//time statistic
				all_sleeped_time = all_sleeped_time + (op_sim_time()-record_time);
				record_time = op_sim_time();
				}
				FSM_PROFILE_SECTION_OUT (state8_enter_exec)

			/** state (RWaking) exit executives **/
			FSM_STATE_EXIT_FORCED (8, "RWaking", "pw_mac [RWaking exit execs]")


			/** state (RWaking) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "RWaking", "idle2", "tr_122", "pw_mac [RWaking -> idle2 : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle2) enter executives **/
			FSM_STATE_ENTER_UNFORCED (9, "idle2", state9_enter_exec, "pw_mac [idle2 enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [idle2 enter execs]", state9_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(debug)
						printf("Node:%d, Grade:%d, time:%f, In \"idle2\" state: received DATA which is from \"gsrc\".\n",node_address, grade, op_sim_time());
					upper_data_pk_proc();
				}
				
				if(END)
				{
				
					//time statistic
					all_active_time = all_active_time + (op_sim_time()-record_time);
					power_stat();
					record_grade();
					
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
					
					op_prg_list_free (prediction_state_list);
					op_prg_mem_free (prediction_state_list);
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
				}
				}
				FSM_PROFILE_SECTION_OUT (state9_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (19,"pw_mac")


			/** state (idle2) exit executives **/
			FSM_STATE_EXIT_UNFORCED (9, "idle2", "pw_mac [idle2 exit execs]")


			/** state (idle2) transition processing **/
			FSM_PROFILE_SECTION_IN ("pw_mac [idle2 trans conditions]", state9_trans_conds)
			FSM_INIT_COND (SLEEP_FROM_RECEIVE)
			FSM_TEST_COND (CHECK_CHANNEL)
			FSM_TEST_COND (SEND_BEACON)
			FSM_TEST_COND (SEND_ACK)
			FSM_TEST_COND (NO_DATA)
			FSM_TEST_COND (FROM_LOWER && transceiver_state)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle2")
			FSM_PROFILE_SECTION_OUT (state9_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 5, state5_enter_exec, ;, "SLEEP_FROM_RECEIVE", "", "idle2", "sleeping", "tr_126", "pw_mac [idle2 -> sleeping : SLEEP_FROM_RECEIVE / ]")
				FSM_CASE_TRANSIT (1, 12, state12_enter_exec, ;, "CHECK_CHANNEL", "", "idle2", "CarrierSense1", "tr_133", "pw_mac [idle2 -> CarrierSense1 : CHECK_CHANNEL / ]")
				FSM_CASE_TRANSIT (2, 17, state17_enter_exec, ;, "SEND_BEACON", "", "idle2", "sendBeacon", "tr_147", "pw_mac [idle2 -> sendBeacon : SEND_BEACON / ]")
				FSM_CASE_TRANSIT (3, 18, state18_enter_exec, ;, "SEND_ACK", "", "idle2", "send ACK", "tr_149", "pw_mac [idle2 -> send ACK : SEND_ACK / ]")
				FSM_CASE_TRANSIT (4, 10, state10_enter_exec, ;, "NO_DATA", "", "idle2", "NO_DATA", "tr_151", "pw_mac [idle2 -> NO_DATA : NO_DATA / ]")
				FSM_CASE_TRANSIT (5, 11, state11_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "idle2", "RcvData", "tr_153", "pw_mac [idle2 -> RcvData : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (6, 9, state9_enter_exec, ;, "default", "", "idle2", "idle2", "tr_135", "pw_mac [idle2 -> idle2 : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (NO_DATA) enter executives **/
			FSM_STATE_ENTER_FORCED (10, "NO_DATA", state10_enter_exec, "pw_mac [NO_DATA enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [NO_DATA enter execs]", state10_enter_exec)
				{
				//op_ima_obj_attr_get(process_id, "CW",&CW);
				
				//First cancel previous set timer
				if(op_ev_valid(goto_SLEEP_event)){
					op_ev_cancel(goto_SLEEP_event);
					if(debug)
						printf("1--Node:%d, Grade:%d, time:%f,in NO_DATA state from idle2.\nCancel goto_SLEEP_event.\n",node_address,grade,op_sim_time());
				}
				if(debug)
					printf("2--Node:%d, Grade:%d, time:%f,in NO_DATA state from idle2.\nGo to sleep immediately.\n",node_address,grade,op_sim_time());
				//Then go to sleep right now
				op_intrpt_schedule_self(op_sim_time() + 0.0,SLEEP_FROM_RECEIVE_CODE);
				}
				FSM_PROFILE_SECTION_OUT (state10_enter_exec)

			/** state (NO_DATA) exit executives **/
			FSM_STATE_EXIT_FORCED (10, "NO_DATA", "pw_mac [NO_DATA exit execs]")


			/** state (NO_DATA) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "NO_DATA", "idle2", "tr_152", "pw_mac [NO_DATA -> idle2 : default / ]")
				/*---------------------------------------------------------*/



			/** state (RcvData) enter executives **/
			FSM_STATE_ENTER_FORCED (11, "RcvData", state11_enter_exec, "pw_mac [RcvData enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [RcvData enter execs]", state11_enter_exec)
				{
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_get(RCV_STRM);
				
				pk_size=op_pk_total_size_get(pk_TV);
				time=(pk_size+10)/data_rate;
				//time statistic
				all_rx_time = all_rx_time + time;
				
				if(debug)
					printf("At The Receiver, Node:%d, Grade:%d, time:%f.\nReceived a packet, size: %d.\nReceive Duration is: %f.\n",	node_address,grade,op_sim_time(),pk_size,time);
				
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				
				//will cancel wait_data_event here if receiving desired DATA
				//and reply with ACK: also a BEACON immediately
				//Then go to sleep
				//write now
				
				if(pk_type == data_pk){//receive a data packet
					op_pk_nfd_get(pk_TV,"Next Hop",&dest_TV);
					if(dest_TV == node_address){//the data is the desired one for the receiver
						if(op_ev_valid(wait_DATA_event)){
							op_ev_cancel(wait_DATA_event);
							op_pk_nfd_get(pk_TV,"Previous Hop",&ack_next_hop);
							
							op_pk_nfd_get(pk_TV,"Hop Num", &hop_num_TV);
							hop_num_TV++;
							op_pk_nfd_set(pk_TV,"Hop Num",hop_num_TV);
							op_pk_nfd_get(pk_TV,"PS Flag", &psFlag_TV);
							
							//for test
							op_pk_nfd_get(pk_TV,"Grade", &grade_TV);
							if(grade_TV!=grade+1){
								printf("grade_TV=%d, grade=%d.\n",grade_TV,grade);
								op_sim_end("仿真出错","the received data is from a lower grade","请反思","");
							}
							op_pk_nfd_set(pk_TV,"Grade",grade);
							
							if(node_type == sink){
								op_pk_nfd_get(pk_TV,"Data No",&data_id);
								if(!data_id_exist(data_id)){
									id_TV=(DataID *)op_prg_mem_alloc(sizeof(DataID));
									id_TV->st_data_id=data_id;
									op_prg_list_insert(data_id_list,id_TV,OPC_LISTPOS_TAIL);
									data_nums_1++;
								}
								op_pk_send(pk_TV,DISCARD_STRM);
								//printf("Data has arrived at sink!!!\n");
							}else{
								//insert the DATA packet into data_queue
								op_subq_pk_insert (data_queue,pk_TV,OPC_QPOS_TAIL);
							}
							
							//reply a BEACON as ACK immediately
							//send BEACON
							//printf("test1!!!\n");
							if(pk_ack_TV!=NULL){
								op_pk_destroy(pk_ack_TV);
							}
							//printf("test2!!!\n");
							if(debug)
								printf("The Receiver, Node:%d, Grade:%d, time:%f.\nSuccessfully received DATA.\nReply ACK Beacon after SIFS time.\n",	node_address,grade,op_sim_time());
							
							pk_ack_TV = op_pk_create_fmt("MAC_BEACON_PK");
							op_pk_nfd_set(pk_ack_TV, "Type", beacon_pk);
							op_pk_nfd_set(pk_ack_TV,"Src", node_address);
							op_pk_nfd_set(pk_ack_TV,"Dest",ack_next_hop);
							op_pk_nfd_set(pk_ack_TV,"Grade",grade);
							
							//if the sender has no request for the state of the receiver
							op_pk_nfd_set(pk_ack_TV,"Current Seed",-1.0);
							op_pk_nfd_set(pk_ack_TV,"Create Time",-1.0);
							op_pk_nfd_set(pk_ack_TV,"Waking Time",-1.0);
										
							
							if(psFlag_TV==Request){//the sender has request for the state of the receiver
								op_pk_nfd_set(pk_ack_TV,"Current Seed",randState);
								op_pk_nfd_set(pk_ack_TV,"Create Time",op_sim_time());
								op_pk_nfd_set(pk_ack_TV,"Waking Time",RWaking_time);
								if(debug)
									printf("Embedded prediction state.\n");
							}
							
							
							CW=0;//set CW to zero
							op_subq_pk_insert (ack_queue,pk_ack_TV,OPC_QPOS_TAIL);
							time=SIFS;
							send_ACK_event=op_intrpt_schedule_self(op_sim_time()+time,SEND_ACK_CODE);
							//op_pk_send(pk_ack_TV,SEND_STRM);
							
							//First cancel previous set timer
							if(op_ev_valid(goto_SLEEP_event)){
								op_ev_cancel(goto_SLEEP_event);
							}
							//go to sleep after durBEACON time
							op_intrpt_schedule_self(op_sim_time() + SIFS+durBEACON,SLEEP_FROM_RECEIVE_CODE);
							
						}else{//wait_DATA_event has been invalid
							op_pk_destroy(pk_TV);
							
							if(debug)
							printf("Error:\n\
									The Receiver, Node:%d, Grade:%d, time:%f.\n	wait_DATA_event has been invalid, the receiver is in Sleep.\nBut Receiving a DATA: That is impossible.\n",\
									node_address,grade,op_sim_time());
							//op_sim_end("Error!","wait_DATA_event has been invalid, the receiver is in Sleep.","But Receiving a DATA: That is impossible.","");
						}
					}else{//the data is not the desired one for the receiver, not a collision
					
						if(debug)
						printf("The Receiver, Node:%d, Grade:%d, time:%f.\n	Warning: The received DATA is not the Desired one.\nSet CW and go to sleep after the left time.\n",\
								node_address,grade,op_sim_time());
						
						op_pk_destroy(pk_TV);
						CW=0;
						//op_ima_obj_attr_get(process_id, "CW",&CW);
						
					}
				}else{//not a DATA packet, not a collision
					if(debug)
					printf("Node:%d, Grade:%d, time:%f.\nThe received pk is not DATA. The pk type is: %d.\nDestroy this pk.\n",node_address,grade,op_sim_time(),pk_type);
						
					op_pk_destroy(pk_TV);
					CW=0;
					//op_ima_obj_attr_get(process_id, "CW",&CW);
				}
				}
				FSM_PROFILE_SECTION_OUT (state11_enter_exec)

			/** state (RcvData) exit executives **/
			FSM_STATE_EXIT_FORCED (11, "RcvData", "pw_mac [RcvData exit execs]")


			/** state (RcvData) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "RcvData", "idle2", "tr_154", "pw_mac [RcvData -> idle2 : default / ]")
				/*---------------------------------------------------------*/



			/** state (CarrierSense1) enter executives **/
			FSM_STATE_ENTER_FORCED (12, "CarrierSense1", state12_enter_exec, "pw_mac [CarrierSense1 enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [CarrierSense1 enter execs]", state12_enter_exec)
				{
				if(debug)
				printf("Node:%d, time:%f. For Carrier Sense1 from idle2. \n Entered statwire Interruption.\n",node_address,op_sim_time());
				
				if(op_ev_valid(wait_DATA_event)){//The receiver is waiting data
					//a collision may happen, set CW
					op_ima_obj_attr_get(process_id, "CW",&CW);
				}
				
				/*
				if(op_ev_valid(send_BEACON_event)){
					op_ev_cancel(send_BEACON_event);
					//Then go to sleep right now
					op_intrpt_schedule_self(op_sim_time() + 0.0,SLEEP_FROM_RECEIVE_CODE);
					if(debug)
						printf("Channel is busy, cancel \"send_BEACON_event\", go to sleep immediately.\n");
				}
				*/
				}
				FSM_PROFILE_SECTION_OUT (state12_enter_exec)

			/** state (CarrierSense1) exit executives **/
			FSM_STATE_EXIT_FORCED (12, "CarrierSense1", "pw_mac [CarrierSense1 exit execs]")


			/** state (CarrierSense1) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "CarrierSense1", "idle2", "tr_134", "pw_mac [CarrierSense1 -> idle2 : default / ]")
				/*---------------------------------------------------------*/



			/** state (RecmpSeed1) enter executives **/
			FSM_STATE_ENTER_FORCED (13, "RecmpSeed1", state13_enter_exec, "pw_mac [RecmpSeed1 enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [RecmpSeed1 enter execs]", state13_enter_exec)
				{
				VRWaking_time=op_sim_time();//Virtual Waking time of receiver
				randState=RandNum(randState,node_address);//compute current interval
				
				wakeinterval=randState/1000.0;
				virtual_to_receive_event=op_intrpt_schedule_self(op_sim_time() + wakeinterval,RECOMPUTE_SEED_CODE);
				
				if(debug)
					printf("Node:%d, Grade:%d, time:%f,in RecmpSeed1 from sleeping.\n randState is %d. Next virtual waking to receive time= %f.\n",	node_address,grade,op_sim_time(),randState, op_sim_time()+wakeinterval);
				}
				FSM_PROFILE_SECTION_OUT (state13_enter_exec)

			/** state (RecmpSeed1) exit executives **/
			FSM_STATE_EXIT_FORCED (13, "RecmpSeed1", "pw_mac [RecmpSeed1 exit execs]")


			/** state (RecmpSeed1) transition processing **/
			FSM_TRANSIT_FORCE (5, state5_enter_exec, ;, "default", "", "RecmpSeed1", "sleeping", "tr_137", "pw_mac [RecmpSeed1 -> sleeping : default / ]")
				/*---------------------------------------------------------*/



			/** state (RecmpSeed2) enter executives **/
			FSM_STATE_ENTER_FORCED (14, "RecmpSeed2", state14_enter_exec, "pw_mac [RecmpSeed2 enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [RecmpSeed2 enter execs]", state14_enter_exec)
				{
				VRWaking_time=op_sim_time();//Virtual Waking time of receiver
				randState=RandNum(randState,node_address);//compute current interval
				
				wakeinterval=randState/1000.0;
				virtual_to_receive_event=op_intrpt_schedule_self(op_sim_time() + wakeinterval,RECOMPUTE_SEED_CODE);
				
				if(debug)
				printf("Node:%d, Grade:%d, time:%f,in RecmpSeed2 from idle1.\n randState is %d. Next virtual waking to receive time= %f.\n",node_address,grade,op_sim_time(),randState, op_sim_time()+wakeinterval);
				}
				FSM_PROFILE_SECTION_OUT (state14_enter_exec)

			/** state (RecmpSeed2) exit executives **/
			FSM_STATE_EXIT_FORCED (14, "RecmpSeed2", "pw_mac [RecmpSeed2 exit execs]")


			/** state (RecmpSeed2) transition processing **/
			FSM_TRANSIT_FORCE (7, state7_enter_exec, ;, "default", "", "RecmpSeed2", "idle1", "tr_140", "pw_mac [RecmpSeed2 -> idle1 : default / ]")
				/*---------------------------------------------------------*/



			/** state (RcvBeacon) enter executives **/
			FSM_STATE_ENTER_FORCED (15, "RcvBeacon", state15_enter_exec, "pw_mac [RcvBeacon enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [RcvBeacon enter execs]", state15_enter_exec)
				{
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_get(RCV_STRM);
				
				pk_size=op_pk_total_size_get(pk_TV);
				time=(pk_size+10)/data_rate;
				//time statistic
				all_rx_time = all_rx_time + time;
				
				if(debug)
					printf("At The Sender, Node:%d, Grade:%d, time:%f.\nReceived a packet, size: %d.\nReceive Duration is: %f.\n",	node_address,grade,op_sim_time(),pk_size,time);
				
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				
				//the beacon or the beacon for ack
				if(pk_type == beacon_pk){//receive a beacon packet
					op_pk_nfd_get(pk_TV,"Grade",&grade_TV);
					
					op_pk_nfd_get(pk_TV,"Dest",&dest_TV);
					
					//1. broadcasted from a node in grade-1
					if(dest_TV == -1 && (grade_TV+1) == grade && !op_ev_valid(send_DATA_event) && !op_ev_valid(wait_ACK_event)){
						//prepare to send DATA
					
						op_pk_nfd_get(pk_TV,"Src",&data_next_hop);
						
						op_pk_nfd_get(pk_TV,"CW",&CW_TV);
						time = op_dist_uniform(CW_TV*1000.0)/1000.0+DIFS;
						
						send_DATA_event=op_intrpt_schedule_self(op_sim_time() + time,SEND_DATA_CODE);
						
						if(debug)
							printf("At the sender, Node:%d, Grade:%d, time:%f.\nThe received beacon is a broadcast. Need to send data after %f seconds.\nWaiting to send data!\n",\
								node_address,grade,op_sim_time(),time);	
						
					}else
					//2. a beacon for ack from a node in grade-1
					if(dest_TV == node_address && grade_TV+1 == grade && op_ev_valid(wait_ACK_event)){
						op_ev_cancel(wait_ACK_event);
							
						op_pk_nfd_get(pk_TV,"Current Seed",&cur_seed_TV);
						op_pk_nfd_get(pk_TV,"Create Time",&creat_time_TV);
						op_pk_nfd_get(pk_TV,"Waking Time",&RWaking_time_TV);
						
						if(debug)
							printf("At the Sender, Node:%d, Grade:%d, time:%f.\nThe received beacon is for ACK.\n",\
								node_address,grade,op_sim_time());
				
						if(cur_seed_TV>=0 && creat_time_TV>=0 && RWaking_time_TV>=0){//update predicition table
							PredictionState * ps_TV=(PredictionState *)op_prg_mem_alloc(sizeof(PredictionState));
								
							ps_TV->ps_id=data_next_hop;
							ps_TV->ps_rs=cur_seed_TV;
							ps_TV->ps_td=op_sim_time()-creat_time_TV;
							wakeinterval=cur_seed_TV/1000.0; //change unit: second
							ps_TV->ps_next_wt=RWaking_time_TV+wakeinterval;//-op_sim_time();
							ps_TV->ps_lastupdate=op_sim_time();
							
							op_prg_list_insert(prediction_state_list,ps_TV,OPC_LISTPOS_TAIL);
							
							if(debug)
								printf("The prediction state of the receiver is embedded in this beacon.\nThe next waking time for this sender is %f.\n",ps_TV->ps_next_wt);
						}
						
						//Data has been sent correctly, remove it from temp_queue
						op_pk_destroy(op_subq_pk_remove(data_queue, OPC_QPOS_HEAD));
						retx--;
						//recieved ACK correctly, go to sleep immediately
						op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SEND_CODE);
						if(debug)
							printf("Go to sleep immediately.\n");
						
					//3. not the desired beacon packet	
					}else{
						op_pk_destroy(pk_TV);
						
						if(debug)
							printf("AT The Sender, Node:%d, Grade:%d, time:%f.\nThe received beacon is not the desired one.\nJust ignore it.\n",\
									node_address,grade,op_sim_time());
					}
				}else{//other packet (e.g., DATA)
					op_pk_destroy(pk_TV);
					
					//op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SEND_CODE);
					
					if(debug)
						printf("At the sender, Node:%d, Grade:%d, time:%f.\nThe received not a beacon.\nKeeps awake!\n",node_address,grade,op_sim_time());
				}
				
				/*
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				*/
				}
				FSM_PROFILE_SECTION_OUT (state15_enter_exec)

			/** state (RcvBeacon) exit executives **/
			FSM_STATE_EXIT_FORCED (15, "RcvBeacon", "pw_mac [RcvBeacon exit execs]")


			/** state (RcvBeacon) transition processing **/
			FSM_TRANSIT_FORCE (7, state7_enter_exec, ;, "default", "", "RcvBeacon", "idle1", "tr_142", "pw_mac [RcvBeacon -> idle1 : default / ]")
				/*---------------------------------------------------------*/



			/** state (send DATA) enter executives **/
			FSM_STATE_ENTER_FORCED (16, "send DATA", state16_enter_exec, "pw_mac [send DATA enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [send DATA enter execs]", state16_enter_exec)
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
				op_pk_nfd_set(pk_TV,"Next Hop",data_next_hop);
				
				if(find_R_in_ps(data_next_hop)==NULL){//the sender have no record of the receiver's state
					op_pk_nfd_set(pk_TV,"PS Flag",Request);
					if(debug)
					printf("1--Node:%d, Grade:%d, time:%f.\nNeed to request Prediction State. Set PS Flag.\n",node_address,grade,op_sim_time());
				}else{
					op_pk_nfd_set(pk_TV,"PS Flag",noRequest);
					if(debug)
					printf("1--Node:%d, Grade:%d, time:%f.\nDon't need to request Prediction State.\n",node_address,grade,op_sim_time());
				}
				
				op_pk_send(pk_TV,SEND_STRM);
				retx++;
				
				//time statistic
				all_tx_time = all_tx_time + durDATA;
				
				//set timer for waiting ACK from lower grade node
				time = durDATA + SIFS + durBEACON;
				wait_ACK_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_SEND_CODE);
				
				if(debug)
				printf("2--Node:%d, Grade:%d, time:%f.\nHave sent DATA, waiting a beacon for ACK.\nIf no ACK, after %f seconds, go to sleep (time:%f).\n",\
						node_address,grade,op_sim_time(),time,op_sim_time()+time);
				}
				FSM_PROFILE_SECTION_OUT (state16_enter_exec)

			/** state (send DATA) exit executives **/
			FSM_STATE_EXIT_FORCED (16, "send DATA", "pw_mac [send DATA exit execs]")


			/** state (send DATA) transition processing **/
			FSM_TRANSIT_FORCE (7, state7_enter_exec, ;, "default", "", "send DATA", "idle1", "tr_144", "pw_mac [send DATA -> idle1 : default / ]")
				/*---------------------------------------------------------*/



			/** state (sendBeacon) enter executives **/
			FSM_STATE_ENTER_FORCED (17, "sendBeacon", state17_enter_exec, "pw_mac [sendBeacon enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [sendBeacon enter execs]", state17_enter_exec)
				{
				//send BEACON
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_create_fmt("MAC_BEACON_PK");
				op_pk_nfd_set(pk_TV, "Type", beacon_pk);
				op_pk_nfd_set(pk_TV,"Src", node_address);
				op_pk_nfd_set(pk_TV,"Dest",-1);//broadcast
				//op_pk_nfd_set(pk_TV,"Current Seed",randState);
				//op_pk_nfd_set(pk_TV,"Create Time",op_sim_time());
				//op_pk_nfd_set(pk_TV,"RWaking Time",RWaking_time);
				op_pk_nfd_set(pk_TV,"Grade",grade);
				op_pk_nfd_set(pk_TV,"CW",CW);
				
				op_pk_send(pk_TV, SEND_STRM);
				
				//time statistic
				all_tx_time = all_tx_time + durBEACON;
				
				time=durBEACON+CW+DIFS+durDATA;
				//set timer for waiting DATA from the sender
				wait_DATA_event=op_intrpt_schedule_self(op_sim_time() + time,WAIT_DATA_EVENT_CODE);
				
				if(debug)
				printf("Node:%d, Grade:%d, time:%f,in sendBeacon state. Have Sent Beacon.\nSet wait_DATA_event.\nAfter %f seconds, if there is no data, go to NO_DATA (time:%f).\n",\
							node_address,grade,op_sim_time(),time,op_sim_time()+time);
				}
				FSM_PROFILE_SECTION_OUT (state17_enter_exec)

			/** state (sendBeacon) exit executives **/
			FSM_STATE_EXIT_FORCED (17, "sendBeacon", "pw_mac [sendBeacon exit execs]")


			/** state (sendBeacon) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "sendBeacon", "idle2", "tr_148", "pw_mac [sendBeacon -> idle2 : default / ]")
				/*---------------------------------------------------------*/



			/** state (send ACK) enter executives **/
			FSM_STATE_ENTER_FORCED (18, "send ACK", state18_enter_exec, "pw_mac [send ACK enter execs]")
				FSM_PROFILE_SECTION_IN ("pw_mac [send ACK enter execs]", state18_enter_exec)
				{
				if(op_subq_empty(ack_queue) == OPC_FALSE)
				{
					if(debug)
						printf("Node:%d, Grade:%d, time:%f,in sendACK state. Have Sent ACK.\n",\
							node_address,grade,op_sim_time());
					pk_TV = op_subq_pk_remove(ack_queue, OPC_QPOS_HEAD);
					op_pk_send(pk_TV,SEND_STRM);
					//time statistic
					all_tx_time = all_tx_time + durDATA;
				}
				else
				{
					op_sim_end("Err: grade queue is empty, simulation is over!","","","");
				}
				}
				FSM_PROFILE_SECTION_OUT (state18_enter_exec)

			/** state (send ACK) exit executives **/
			FSM_STATE_EXIT_FORCED (18, "send ACK", "pw_mac [send ACK exit execs]")


			/** state (send ACK) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "send ACK", "idle2", "tr_150", "pw_mac [send ACK -> idle2 : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"pw_mac")
		}
	}




void
_op_pw_mac_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_pw_mac_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_pw_mac_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_pw_mac_svar function. */
#undef Q_empty
#undef transceiver_state
#undef A
#undef all_active_time
#undef all_rx_time
#undef all_sleeped_time
#undef all_tx_time
#undef CW
#undef durBEACON
#undef durDATA
#undef record_time
#undef RWaking_time
#undef send_gm_timer
#undef start_grade_timer
#undef start_schedule_timer
#undef VRWaking_time
#undef goto_SLEEP_event
#undef send_gm_event
#undef virtual_to_receive_event
#undef wait_DATA_event
#undef wake_to_receive_event
#undef wake_to_send_event
#undef ack_next_hop
#undef c
#undef coef_a
#undef grade
#undef m
#undef data_next_hop
#undef node_address
#undef node_type
#undef randState
#undef prediction_state_list
#undef routing_table_list
#undef node_id
#undef process_id
#undef rx_id
#undef send_DATA_event
#undef wait_ACK_event
#undef DIFS
#undef SIFS
#undef wakeinterval
#undef debug
#undef send_BEACON_event
#undef send_ACK_event
#undef tx_id
#undef tx_channel_objid
#undef data_rate
#undef data_id_list
#undef range

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_pw_mac_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_pw_mac_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (pw_mac)",
		sizeof (pw_mac_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_pw_mac_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	pw_mac_state * ptr;
	FIN_MT (_op_pw_mac_alloc (obtype))

	ptr = (pw_mac_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "pw_mac [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_pw_mac_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	pw_mac_state		*prs_ptr;

	FIN_MT (_op_pw_mac_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (pw_mac_state *)gen_ptr;

	if (strcmp ("Q_empty" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->Q_empty);
		FOUT
		}
	if (strcmp ("transceiver_state" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->transceiver_state);
		FOUT
		}
	if (strcmp ("A" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->A);
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
	if (strcmp ("all_sleeped_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->all_sleeped_time);
		FOUT
		}
	if (strcmp ("all_tx_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->all_tx_time);
		FOUT
		}
	if (strcmp ("CW" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->CW);
		FOUT
		}
	if (strcmp ("durBEACON" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durBEACON);
		FOUT
		}
	if (strcmp ("durDATA" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->durDATA);
		FOUT
		}
	if (strcmp ("record_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->record_time);
		FOUT
		}
	if (strcmp ("RWaking_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->RWaking_time);
		FOUT
		}
	if (strcmp ("send_gm_timer" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_gm_timer);
		FOUT
		}
	if (strcmp ("start_grade_timer" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->start_grade_timer);
		FOUT
		}
	if (strcmp ("start_schedule_timer" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->start_schedule_timer);
		FOUT
		}
	if (strcmp ("VRWaking_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->VRWaking_time);
		FOUT
		}
	if (strcmp ("goto_SLEEP_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->goto_SLEEP_event);
		FOUT
		}
	if (strcmp ("send_gm_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_gm_event);
		FOUT
		}
	if (strcmp ("virtual_to_receive_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->virtual_to_receive_event);
		FOUT
		}
	if (strcmp ("wait_DATA_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wait_DATA_event);
		FOUT
		}
	if (strcmp ("wake_to_receive_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wake_to_receive_event);
		FOUT
		}
	if (strcmp ("wake_to_send_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wake_to_send_event);
		FOUT
		}
	if (strcmp ("ack_next_hop" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->ack_next_hop);
		FOUT
		}
	if (strcmp ("c" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->c);
		FOUT
		}
	if (strcmp ("coef_a" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->coef_a);
		FOUT
		}
	if (strcmp ("grade" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->grade);
		FOUT
		}
	if (strcmp ("m" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->m);
		FOUT
		}
	if (strcmp ("data_next_hop" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->data_next_hop);
		FOUT
		}
	if (strcmp ("node_address" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_address);
		FOUT
		}
	if (strcmp ("node_type" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_type);
		FOUT
		}
	if (strcmp ("randState" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->randState);
		FOUT
		}
	if (strcmp ("prediction_state_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->prediction_state_list);
		FOUT
		}
	if (strcmp ("routing_table_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->routing_table_list);
		FOUT
		}
	if (strcmp ("node_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_id);
		FOUT
		}
	if (strcmp ("process_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->process_id);
		FOUT
		}
	if (strcmp ("rx_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->rx_id);
		FOUT
		}
	if (strcmp ("send_DATA_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_DATA_event);
		FOUT
		}
	if (strcmp ("wait_ACK_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wait_ACK_event);
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
	if (strcmp ("wakeinterval" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->wakeinterval);
		FOUT
		}
	if (strcmp ("debug" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->debug);
		FOUT
		}
	if (strcmp ("send_BEACON_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_BEACON_event);
		FOUT
		}
	if (strcmp ("send_ACK_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_ACK_event);
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
	if (strcmp ("data_id_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->data_id_list);
		FOUT
		}
	if (strcmp ("range" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->range);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

