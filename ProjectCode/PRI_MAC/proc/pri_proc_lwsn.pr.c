/* Process model C form file: pri_proc_lwsn.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char pri_proc_lwsn_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 5506F677 5506F677 1 wpd FeiTong 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                              ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

#include <math.h>

//Define CUSHION_TIME, for a node to wait a packet. If it is zero, node cannot receive it
#define CUSHION_TIME 1e-10

//Define node type
#define sink 	1
#define sensor 	2

//Define ici type
/*#define		energy_dc_ici		1
#define		delivery_ici		2
#define		delay_ici			3
*/

#define stat_code	500

//Define intript source: ddc,lht,or hrc
#define from_ddc	1
#define from_lht	2
#define from_hrc	3

//Define packet type
#define grade_pk  1
#define rts_pk    2
#define cts_pk    3
#define ack_pk    4
#define data_pk   5

//Define queue type
#define grade_queue		0
#define data_queue		1
#define send_queue		2

//Define stream in-out NO.
#define SRC_STRM 		1
#define RCV_STRM 		0
#define SEND_STRM 		0
#define DISCARD_STRM 	1

#define INFINITY 1.0E100

//#define SLOT_NO_ASSIGN	-2 //the node is not assigned to a slot by the sink

//Define node state
/*
#define initial			0
#define waking			1
#define awake			2//go to sending state
#define sleeping		3
#define asleep			4
*/

//Define DEBUG
#define DEBUG							(debug == 1) /*1级调试 or 全局调试*/
#define DEBUG_2ND						(debug == 2) /*2级调试*/


//Define remote-interrupt code 
#define REMOTE_STAT_CODE				500	/*remote statistics*/

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
#define DECREASE_CW_CODE				70000

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
#define DECREASE_CW					((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == DECREASE_CW_CODE))
//#define DDC_ON						(mac_flags->DDC_on == OPC_TRUE)
//#define LHT_ON						(mac_flags->LHT_on == OPC_TRUE)
//#define HRC_ON						(mac_flags->HRC_on == OPC_TRUE)

typedef struct
{
	int rt_next_hop;
	int rt_grade;
}RoutingTable;

/*
typedef struct
{
	int address;//the address of a neighbor node of the sink.
	int start_slot;//the starting slot assigned to this node.
	int num_of_slots;//the number of consecutive slots assigned to this node.
}SinkNeighbor;	
*/

typedef struct PriT_mac_Flags
{
	//Boolean queue_full; //the queue is full or not, check every start of T state.
	Boolean queue_inf; //true if the queue length is infinity
	Boolean busy_before_grade;//the channel is sensed busy before sending the GRADE message
	Boolean busy_before_rts;//the channel is sensed busy before sending RTS.
	Boolean busy_before_cts;//the channel is sensed busy before replying CTS.
	//Boolean post_backoff;//true if the node won't go to sleep during T state even if the queue is empty
	//Boolean DDC_on;//the flag showing whether open the dynamic duty cycle.
	//Boolean ddc_relay;//show whether in dynamic duty cycle status if DDC_on is set
	//Boolean ddc_source;
	
	//Boolean LHT_on;//true when the LHT (Last-Hop Transmission) is supported
	//Boolean lht_next;//true if sleep time will really be used for data transmission.
	
	//Boolean HRC_on;//true if the hybrid reservation/contention-based transmitting schemes are supported
	//Boolean hrc_go;
	//Boolean slot_assigned;//true if the grade-one node is assigned to sleeping slot(s).
	
	Boolean sent_rts; //true if the sender sent an RTS; -2:表示倒数第二所加
	Boolean received_cts; //true if the sender receives the CTS after sendint RTS; -1: 表示最后所加
	Boolean received_ack; //true if the sender receives the ACK after sending DATA
	Boolean received_data;//true if the receiver receives DATA after replying CTS
	
	Boolean stat_flag; //statistics or not
	Boolean write_stat_flag;//write the statistics or not
}PriT_Mac_Flags;


typedef struct
{
	int st_data_id;
}DataID;


int retx=0; //the number of retransmission

double avg_energy_consumption = 0.0;
//double avg_duty_cycle=0.0;
int data_nums_1=0;

/*
* Packets loss due to overflow should check the following sources
* 1. function "upper_data_pk_proc" in FB, since a packet coming from 
*    the upper layer should be inserted into the data queue.
* 2. state "RCV(1)", when the relay node receives a packet from the 
*    lower layer, it needs to store it into the data queue.
*/
double pkts_loss_overflow = 0.0;

/*
* The packets loss due to collision is equal to the number of 
* retransmissions, i.e., the variable"retx" when the retransmission is not supported.
* "retx" contains the following operations
* 1. in state "send data" (in process "pri_proc_enhanced"). "retx" is increased whenever
*    it sends a packet out, assuming the packet is potentially collidided.
* 2. in state "RCV(2)", "retx" is decreased when ACK is received to show the packet is 
*    successfully received. 
*/
double pkts_loss_collision = 0.0;

//define avg_queue_stat_list to calculate the average queue status for each hop
List* avg_queue_stat_list = NULL;
typedef struct Avg_Q_Stat
{
	List* avg_q_prob_list;
	    
	double avg_p_t;
	double avg_p_s;
	double avg_p_b;
	double avg_p_r;
	
	double delay_to_sink;
	double duty_cycle;
	double throughput;
	int chain_num;
}Avg_Q_Stat;


//function prototype
static Boolean data_id_exist(int id);
static void grade_pk_proc(Packet * pk);
static void upper_data_pk_proc(void);
static void power_stat(void);
static void record_grade(void);
static void update_routing_table(int,int);
static int rt_pos_find_next(void);
static Boolean node_exist_sinkneighbor(int);
static void slot_assignment();
//static SinkNeighbor* query_sinkneighbor(int);
static void throughput_stat(void);
static void stat_based_on_prob(void);
static void simulation_end(void);

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
	double	                 		second_contending_time                          ;
	Evhandle	               		wait_CTS_event                                  ;
	Evhandle	               		wait_ACK_event                                  ;
	int	                    		next_hop_1                                      ;
	int	                    		next_hop_2                                      ;
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
	int	                    		debug                                           ;
	List *	                 		data_id_list                                    ;
	int	                    		one_two_cw                                      ;
	List *	                 		routing_table_list                              ;
	double	                 		queue_length                                    ;
	Objid	                  		queue_id                                        ;
	Objid	                  		subq_comp_attr_id                               ;
	Objid	                  		sub_queue_id                                    ;
	Stathandle	             		QLength_handle                                  ;
	Stathandle	             		avg_dc_handle                                   ;
	PriT_Mac_Flags*	        		mac_flags                                       ;
	double	                 		CW_counter                                      ;
	double	                 		slot_time                                       ;	/* the time duration of one slot */
	Evhandle	               		decrease_CW_event                               ;	/* the event handle when decrease CW_counter */
	List *	                 		queue_stat_list                                 ;	/* for each number of packets in the queue,                 */
	                        		                                                	/* record how many times this specific number of packets    */
	                        		                                                	/* appearing in the queue, for calculting the parobability. */
	Stathandle	             		QStatus_handle                                  ;
	double	                 		hops_avg_delay                                  ;
	int	                    		hops_pkt_num                                    ;
	Stathandle	             		Hops_avg_delay_handle                           ;
	double	                 		queue_length_add_one                            ;
	Stathandle	             		lost_pkt_col_handle                             ;	/* the number of pkts due to collision */
	Stathandle	             		lost_pkt_of_handle                              ;	/* the number of pkts due to bufferoverflow */
	Stathandle	             		avg_energy_handle                               ;
	double	                 		CW_grade                                        ;
	double	                 		slot_time_grade                                 ;
	Evhandle	               		ddc_event_handle                                ;
	Evhandle	               		lht_event_handle                                ;
	Stathandle	             		send_prob_handle                                ;
	double	                 		win_success                                     ;
	double	                 		total_send_trial                                ;
	double	                 		steady_time                                     ;
	Evhandle	               		regular_wakeup_evhandle                         ;	/* 正常（不是在sleeping中醒来）醒来事件句柄设置 */
	int	                    		remainder_retran_num                            ;	/* remainder retransmission number */
	Stathandle	             		receive_prob_handle                             ;
	double	                 		receive_success                                 ;
	double	                 		total_receive_trial                             ;
	double	                 		send_success                                    ;
	double	                 		avg_duty_cycle                                  ;
	Stathandle	             		avg_queueing_delay_handle                       ;
	double	                 		avg_queueing_delay                              ;
	double	                 		sum_src_delay                                   ;
	double	                 		src_pkt_num                                     ;
	double	                 		sum_relay_delay                                 ;
	double	                 		relay_pkt_num                                   ;
	double	                 		stat_stop_time                                  ;
	Stathandle	             		throughput2_handle                              ;
	double	                 		recvd_pkt_num_at_sink                           ;	/* to calculate system throughput */
	double	                 		duty_cycle_num_at_sink                          ;	/* to calculate system throughput */
	int	                    		chain_number                                    ;
	int	                    		line_hops                                       ;
	Stathandle	             		win_prob_handle                                 ;
	double	                 		sense_busy                                      ;
	Stathandle	             		busy_prob_handle                                ;
	Stathandle	             		prob_dc_handle                                  ;
	Stathandle	             		prob_delay_to_sink_handle                       ;
	Stathandle	             		prob_throughput_handle                          ;
	} pri_proc_lwsn_state;

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
#define second_contending_time  		op_sv_ptr->second_contending_time
#define wait_CTS_event          		op_sv_ptr->wait_CTS_event
#define wait_ACK_event          		op_sv_ptr->wait_ACK_event
#define next_hop_1              		op_sv_ptr->next_hop_1
#define next_hop_2              		op_sv_ptr->next_hop_2
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
#define mac_flags               		op_sv_ptr->mac_flags
#define CW_counter              		op_sv_ptr->CW_counter
#define slot_time               		op_sv_ptr->slot_time
#define decrease_CW_event       		op_sv_ptr->decrease_CW_event
#define queue_stat_list         		op_sv_ptr->queue_stat_list
#define QStatus_handle          		op_sv_ptr->QStatus_handle
#define hops_avg_delay          		op_sv_ptr->hops_avg_delay
#define hops_pkt_num            		op_sv_ptr->hops_pkt_num
#define Hops_avg_delay_handle   		op_sv_ptr->Hops_avg_delay_handle
#define queue_length_add_one    		op_sv_ptr->queue_length_add_one
#define lost_pkt_col_handle     		op_sv_ptr->lost_pkt_col_handle
#define lost_pkt_of_handle      		op_sv_ptr->lost_pkt_of_handle
#define avg_energy_handle       		op_sv_ptr->avg_energy_handle
#define CW_grade                		op_sv_ptr->CW_grade
#define slot_time_grade         		op_sv_ptr->slot_time_grade
#define ddc_event_handle        		op_sv_ptr->ddc_event_handle
#define lht_event_handle        		op_sv_ptr->lht_event_handle
#define send_prob_handle        		op_sv_ptr->send_prob_handle
#define win_success             		op_sv_ptr->win_success
#define total_send_trial        		op_sv_ptr->total_send_trial
#define steady_time             		op_sv_ptr->steady_time
#define regular_wakeup_evhandle 		op_sv_ptr->regular_wakeup_evhandle
#define remainder_retran_num    		op_sv_ptr->remainder_retran_num
#define receive_prob_handle     		op_sv_ptr->receive_prob_handle
#define receive_success         		op_sv_ptr->receive_success
#define total_receive_trial     		op_sv_ptr->total_receive_trial
#define send_success            		op_sv_ptr->send_success
#define avg_duty_cycle          		op_sv_ptr->avg_duty_cycle
#define avg_queueing_delay_handle		op_sv_ptr->avg_queueing_delay_handle
#define avg_queueing_delay      		op_sv_ptr->avg_queueing_delay
#define sum_src_delay           		op_sv_ptr->sum_src_delay
#define src_pkt_num             		op_sv_ptr->src_pkt_num
#define sum_relay_delay         		op_sv_ptr->sum_relay_delay
#define relay_pkt_num           		op_sv_ptr->relay_pkt_num
#define stat_stop_time          		op_sv_ptr->stat_stop_time
#define throughput2_handle      		op_sv_ptr->throughput2_handle
#define recvd_pkt_num_at_sink   		op_sv_ptr->recvd_pkt_num_at_sink
#define duty_cycle_num_at_sink  		op_sv_ptr->duty_cycle_num_at_sink
#define chain_number            		op_sv_ptr->chain_number
#define line_hops               		op_sv_ptr->line_hops
#define win_prob_handle         		op_sv_ptr->win_prob_handle
#define sense_busy              		op_sv_ptr->sense_busy
#define busy_prob_handle        		op_sv_ptr->busy_prob_handle
#define prob_dc_handle          		op_sv_ptr->prob_dc_handle
#define prob_delay_to_sink_handle		op_sv_ptr->prob_delay_to_sink_handle
#define prob_throughput_handle  		op_sv_ptr->prob_throughput_handle

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	pri_proc_lwsn_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((pri_proc_lwsn_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

static void update_routing_table(int node_hop,int node_grade)
{
//var
	RoutingTable *rt;
//in	
	FIN(update_routing_table(int node_hop,int node_grade));
//body	
	rt = (RoutingTable *)op_prg_mem_alloc(sizeof(RoutingTable));
	
	rt->rt_next_hop = node_hop;
	rt->rt_grade = node_grade;
	op_prg_list_insert(routing_table_list,rt,OPC_LISTPOS_TAIL);
//out	
	FOUT;
}



	
//Deal with grade pk
static void
grade_pk_proc(Packet * pk)
{
//var
	int pre_hop, grade_in_pk;//,sink_id_in_pk;
	//SinkNeighbor* sink_neighbor;
//in
	FIN(grade_pk_proc(pk));
//body
	op_pk_nfd_get(pk,"Grade",&grade_in_pk);
	op_pk_nfd_get(pk,"Previous Hop",&pre_hop);

	//op_pk_nfd_get(pk,"Sink ID",&sink_id_in_pk);
	
	if(grade<0)
	{
		grade = grade_in_pk;
		//op_ima_obj_attr_set(process_id,"grade",grade);
		op_ima_obj_attr_set(node_id,"grade",grade);
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
		//comment the above sentence and set timer for sending grade message
		CW_counter = op_dist_outcome(op_dist_load("uniform_int",0.0,CW_grade*1000-1)); // from [0,W/2-1]
		//if(DEBUG){printf("******************\n");printf("\nGM process: CW_counter is:%f.\n\n",CW_counter);}
		
		if(CW_counter == 0.0){
			op_intrpt_schedule_self(op_sim_time()+0.0, SEND_GM_CODE);
		}else{
			send_gm_event = op_intrpt_schedule_self(op_sim_time()+slot_time_grade,DECREASE_CW_CODE);
			CW_counter--;
		}
		
		FOUT;
	}
	
	if(grade_in_pk < grade)
	{
		grade = grade_in_pk;
		//op_ima_obj_attr_set(process_id,"grade",grade);
		op_ima_obj_attr_set(node_id,"grade",grade);
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
		//comment the above sentence and set timer for sending grade message
		CW_counter = op_dist_outcome(op_dist_load("uniform_int",0.0,CW_grade*1000-1)); // from [0,W-1]
		if(CW_counter == 0.0){
			op_intrpt_schedule_self(op_sim_time()+0.0, SEND_GM_CODE);
		}else{
			send_gm_event = op_intrpt_schedule_self(op_sim_time()+slot_time_grade,DECREASE_CW_CODE);
			CW_counter--;
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
		if(DEBUG)
			printf("The data subqueue is full, the upper layer packet is dropped.");
	}else{		
*/
	if(op_subq_stat(data_queue,OPC_QSTAT_PKSIZE)<queue_length_add_one || mac_flags->queue_inf == OPC_TRUE){//data queue is not full
		op_pk_nfd_set(upper_pk,"Type",data_pk);
		op_pk_nfd_set(upper_pk,"Src",node_address);
		op_pk_nfd_set(upper_pk,"Grade",grade);
		//op_pk_nfd_set(upper_pk,"Create Time Two", second_contending_time+(sleep_factor+2)*durActive_half);
		//op_pk_nfd_set(upper_pk,"Create Time Two", op_sim_time());
	
		op_subq_pk_insert (data_queue,upper_pk,OPC_QPOS_TAIL);
	
	//op_stat_write(QLength_handle, op_subq_stat(data_queue,OPC_QSTAT_PKSIZE));
		if(DEBUG)
			printf("Have inserted the received DATA from \"gsrc\" into DATA queue\n");
	}else{//data queue is full
		pkts_loss_overflow = pkts_loss_overflow + 1.0;
		if(DEBUG)
			printf("in function upper_data_pk_proc, data queue is full, the arrived pkt from the upper layer is dropped.\n");
	}
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
//	char message[512];
	
	Ici* iciptr;
	//for remote statistics
	double node_energy_consumption,node_duty_cycle; 
//in
	FIN(power_stat(void));
//body
	//average hop delay statistics
	op_stat_write_t(Hops_avg_delay_handle,hops_avg_delay/hops_pkt_num,stat_code+1.0);
	
	
	op_ima_obj_attr_get(node_id,"Power Record File",temp_file_name);
	if(strcmp(temp_file_name,"disabled")!=0)
	{	
		//op_sim_message("power stat","");
		if(node_type == sensor){
			node_energy_consumption = all_sleeped_time*0.05+(all_active_time-(all_tx_time+all_rx_time))*0.45+(all_tx_time+all_rx_time)*0.5;
			avg_energy_consumption += node_energy_consumption;
			node_duty_cycle = all_active_time/(all_sleeped_time+all_active_time);
			avg_duty_cycle += node_duty_cycle;
			
			op_stat_write_t(avg_dc_handle,avg_duty_cycle,node_address);
			
			/****************************************************************/
			/*create ICI for statistic purpose; then set remote interruption*/
			/*for average duty cycle per node								*/
			/****************************************************************/
			//printf("preparing to install ici.\n\n");
			iciptr = op_ici_create("energy_dc_ici");
			//op_ici_attr_set (iciptr, "type",energy_dc_ici);
			op_ici_attr_set_dbl(iciptr, "duty_cycle",node_duty_cycle);
			op_ici_attr_set_dbl(iciptr, "energy_consumption",node_energy_consumption);
			op_ici_attr_set_dbl(iciptr, "net_running_time", (all_sleeped_time+all_active_time));
			op_ici_attr_set_int32(iciptr, "node_address",node_address);
			op_ici_install (iciptr);
			//printf("install ici.\n");
			op_intrpt_force_remote (REMOTE_STAT_CODE,op_id_from_name(node_id,OPC_OBJTYPE_PROC,"statistics"));
			op_ici_install (OPC_NIL); 
		}
		nodes_num = op_topo_object_count (OPC_OBJTYPE_NDMOB);
		op_ima_obj_attr_get(process_id, "Sink Num",&sink_num);
		if(node_address == nodes_num-sink_num){
			
			//sprintf(message,"node_address=%d, grade=%d.\n",node_address,grade);
			//op_sim_message(message,"");
			
			//op_stat_write_t(avg_dc_handle,avg_duty_cycle/(nodes_num-sink_num),(double)(nodes_num-sink_num));
			//op_stat_write_t(avg_dc_handle,avg_duty_cycle/(nodes_num-sink_num),stat_code+2.0);
			op_stat_write_t(avg_energy_handle, avg_energy_consumption/((nodes_num-sink_num)*(all_sleeped_time+all_active_time)),stat_code+3.0);
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
/*
* Following things are recorded in this function:
* 1. node adress, and its grade
* 2. the number of retransmissions
* 3. the number of lost pkts due to collision: pkts_loss_collision
* 4. the number of lost pkts due to buffer overflow: pkts_loss_overflow
*
*/
//var
	FILE *file_in;
	char file_name[300];
	Ici* iciptr;
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
		
		pkts_loss_collision = retx;
		op_stat_write_t(lost_pkt_col_handle, pkts_loss_collision, stat_code+4.0);
		op_stat_write_t(lost_pkt_of_handle, pkts_loss_overflow, stat_code+5.0);
	}
	
	/****************************************************************/
	/*create ICI for statistic purpose; then set remote interruption*/
	/*for recording the grade distribution in the network			*/
	/****************************************************************/
	if(node_type==sensor){ //the sink node doesn't need to report its grade and childs
		iciptr = op_ici_create("grade_ici");
		op_ici_attr_set_int32(iciptr, "node_address",node_address);
		op_ici_attr_set_int32(iciptr, "node_grade",grade);
		op_ici_attr_set_int32(iciptr, "child_nums",op_prg_list_size(routing_table_list));
		op_ici_install (iciptr);
		op_intrpt_force_remote (REMOTE_STAT_CODE,op_id_from_name(node_id,OPC_OBJTYPE_PROC,"statistics"));
		op_ici_install (OPC_NIL);
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

static void queue_stat()
{
//var
//	int list_size;
	int list_i,*list_element;
//	int list_j;
	int hops;
	double sum=0.0;
	FILE *file_in;
	char file_name[300];
	Avg_Q_Stat* avg_q_stat_ele;
	double * q_k_prob;
//in
	FIN(queue_stat());
//body
	
	if(mac_flags->queue_inf==OPC_TRUE || node_type==sink)
	{
		FOUT;
	}
	
	//list_size = op_prg_list_size(queue_stat_list);
	for(list_i=0;list_i<=queue_length;list_i++){
		list_element = op_prg_list_access(queue_stat_list,list_i);
		sum=sum + (*list_element);
	}
	
	op_ima_obj_attr_get(node_id,"Grade Record File",file_name);
	file_in = fopen(file_name,"at");
	fprintf(file_in,"node %d:\n",\
		node_address);	
	
	hops=ceil((double)node_address/(double)chain_number);
		
	avg_q_stat_ele = (Avg_Q_Stat *)op_prg_list_access(avg_queue_stat_list,hops-1);
	avg_q_stat_ele->chain_num ++;
	for(list_i=0;list_i<=queue_length;list_i++){
		list_element = op_prg_list_access(queue_stat_list,list_i);
		op_stat_write_t(QStatus_handle,(*list_element)/sum,list_i);
	
		q_k_prob = (double*)op_prg_list_access(avg_q_stat_ele->avg_q_prob_list,list_i);
		*q_k_prob += (double)(*list_element)/(double)sum;
	
		
	
		fprintf(file_in,"%d	%f\n",list_i,(*list_element)/sum);
		if(DEBUG) printf("packet #: %d, appear times:%d.\n",list_i,*list_element);
	}
	fclose(file_in);
	
	
	
	if(total_send_trial!=0.0){
		//op_stat_write_t(win_prob_handle,win_success/total_send_trial,node_address);
		//op_stat_write_t(send_prob_handle,send_success/total_send_trial,node_address);
		
		avg_q_stat_ele->avg_p_t += win_success/total_send_trial;
		avg_q_stat_ele->avg_p_s += send_success/total_send_trial;
		avg_q_stat_ele->avg_p_b += sense_busy/total_send_trial;
	}
	if(total_receive_trial != 0.0){
		//op_stat_write_t(receive_prob_handle,receive_success/total_receive_trial,node_address);
		avg_q_stat_ele->avg_p_r += receive_success/total_receive_trial;
	}
	
	if(node_type == sensor){
		op_stat_write_t(avg_queueing_delay_handle, avg_queueing_delay/send_success,node_address);
	}
	
//out
	FOUT;
}

//delay stat
static void delay_stat(void)
{
//var
	Ici* iciptr;
//in
	FIN(delay_stat());
//body
	if(node_type == sensor){
		/****************************************************************/
		/*create ICI for statistic purpose; then set remote interruption*/
		/*for delay calculation							*/
		/****************************************************************/
		//printf("preparing to install ici.\n\n");
		iciptr = op_ici_create("src_relay_delay_ici");
		op_ici_attr_set_int32(iciptr, "node_address",node_address);
		op_ici_attr_set_dbl(iciptr, "sum_src_delay",sum_src_delay);
		op_ici_attr_set_dbl(iciptr, "src_pkt_num", src_pkt_num);
		op_ici_attr_set_dbl(iciptr, "sum_relay_delay",sum_relay_delay);
		op_ici_attr_set_dbl(iciptr, "relay_pkt_num",relay_pkt_num);
		op_ici_install (iciptr);
		//printf("install ici.\n");
		op_intrpt_force_remote (REMOTE_STAT_CODE,op_id_from_name(node_id,OPC_OBJTYPE_PROC,"statistics"));
		op_ici_install (OPC_NIL); 
	}
//out
	FOUT;
}

//throughput stat
static void throughput_stat(void)
{
//var
//in
	FIN(delay_stat());
//body
	if(node_type == sink){
		op_stat_write_t(throughput2_handle, recvd_pkt_num_at_sink/(duty_cycle_num_at_sink*(sleep_factor+2)*durActive_half),520.0);
	}
//out
	FOUT;
}


static void stat_based_on_prob(void)
{
//var
	int list_i,list_j, j, k,h;
	Avg_Q_Stat * avg_q_stat_ele, * avg_q_stat_ele2;
	double * q_empty_prob, * q_k_prob;
	double * q_full_prob;
	
	double sigma_prime, delta_s, delta_c, delta_b, part_sum, E_R, E_T;
	double T_cycle;
	double D_1, d1, D_2, D_3;
//in
	FIN(stat_based_on_prob());
//body
	if(node_type == sink)
	{
		sigma_prime = DIFS+durRTS+durCTS+durDATA+durACK+3*SIFS;
		delta_s = CW/2.0 + sigma_prime;
		delta_c = DIFS+CW/2.0+durRTS+SIFS+durCTS;
		delta_b = DIFS;
		
		T_cycle = (sleep_factor+2)*durActive_half;
		for(j=1;j<=CW*1000-1;j++)
		{
			part_sum = 0.0;
			for(k=1;k<=j;k++)
			{
				part_sum = part_sum + ((1/j)*(j-k+1)*0.001);
			}
			delta_b = delta_b + (1/(CW*1000-1))*part_sum;
		}
		
		for(list_i=0;list_i<line_hops;list_i++)//starting from the thrid hop (0:1st hop, 1:2nd hop)
		{			
			avg_q_stat_ele = (Avg_Q_Stat *)op_prg_list_access(avg_queue_stat_list,list_i);
			//printf("chain_num=%d, chain_number=%d.\n",avg_q_stat_ele->chain_num,chain_number);
			if(avg_q_stat_ele->chain_num == chain_number)
			{
				for(list_j=0;list_j<=queue_length;list_j++)
				{
					q_k_prob = (double*)op_prg_list_access(avg_q_stat_ele->avg_q_prob_list,list_j);
					*q_k_prob = (*q_k_prob)/chain_number;
					//printf("q_k_prob=%f\n",*q_k_prob);
					op_stat_write_t(QStatus_handle,*q_k_prob, list_i+1);
				}
				
				avg_q_stat_ele->avg_p_t=avg_q_stat_ele->avg_p_t/chain_number;
				avg_q_stat_ele->avg_p_s=avg_q_stat_ele->avg_p_s/chain_number;
				avg_q_stat_ele->avg_p_r=avg_q_stat_ele->avg_p_r/chain_number;
				avg_q_stat_ele->avg_p_b=avg_q_stat_ele->avg_p_b/chain_number;
				
				op_stat_write_t(win_prob_handle,avg_q_stat_ele->avg_p_t,list_i+1);
				op_stat_write_t(send_prob_handle,avg_q_stat_ele->avg_p_s,list_i+1);
				op_stat_write_t(receive_prob_handle,avg_q_stat_ele->avg_p_r,list_i+1);
				op_stat_write_t(busy_prob_handle,avg_q_stat_ele->avg_p_b,list_i+1);
				
			/**************************************************************************/
			// average duty cycle at each hop
				//the probability that queue is empty
				q_empty_prob = (double*)op_prg_list_access(avg_q_stat_ele->avg_q_prob_list,0);
				E_R = avg_q_stat_ele->avg_p_r *(CW/2.0 + sigma_prime) + \
					(1-avg_q_stat_ele->avg_p_r)*(DIFS+CW+durRTS);
				E_T = (1- (*q_empty_prob))*( avg_q_stat_ele->avg_p_s*delta_s + \
					(avg_q_stat_ele->avg_p_t - avg_q_stat_ele->avg_p_s)*delta_c + (1-avg_q_stat_ele->avg_p_t)*delta_b );
				avg_q_stat_ele->duty_cycle = (E_R+E_T)/T_cycle;
				op_stat_write_t(prob_dc_handle, avg_q_stat_ele->duty_cycle,list_i+1);
				
			/*************************************************************************/	
			//average delay to the sink from each hop
//				avg_q_stat_ele->delay_to_sink = T_cycle/2.0;
				//printf("list_i=%d\n",list_i);
				//D_H=0.0;
				for(h=list_i;h>=0;h--)//starting from the thrid hop (0:1st hop, 1:2nd hop)
				{
					//printf("h=%d\n",h);
					avg_q_stat_ele2=(Avg_Q_Stat *)op_prg_list_access(avg_queue_stat_list,h);
					if(h==list_i)
					{
						D_1=T_cycle/2.0;
						
					}else
					{
						D_1=CW/2.0 + SIFS + durACK;
						
					}
					
					D_2=0.0;
					d1 = T_cycle * (1.0/avg_q_stat_ele2->avg_p_t);
					printf("d1=%f\n",d1);
					q_full_prob = (double*)op_prg_list_access(avg_q_stat_ele2->avg_q_prob_list,(int)queue_length);
					for(k=0;k<queue_length;k++)
					{
						q_k_prob = (double*)op_prg_list_access(avg_q_stat_ele2->avg_q_prob_list,k);
						
						//temp = (*q_k_prob)/(*q_full_prob);
						D_2 = D_2 + k*d1*fabs(*q_k_prob)/fabs(1-(*q_full_prob));
					}
					D_3 = T_cycle * (1-avg_q_stat_ele2->avg_p_t)/avg_q_stat_ele2->avg_p_t + \
						durActive_half -CW/2.0 - SIFS - durACK;
					
					avg_q_stat_ele->delay_to_sink += D_1+D_2+D_3;
					
				}
				//printf("delay = %f, D_H=%f\n",avg_q_stat_ele->delay_to_sink,D_H);
				op_stat_write_t(prob_delay_to_sink_handle, avg_q_stat_ele->delay_to_sink,list_i+1);
				
			/*************************************************************************/	
			//average throughput to the sink from the first hop
				if(list_i == 0)
				{
					avg_q_stat_ele->throughput = chain_number*fabs(1-(*q_empty_prob))*avg_q_stat_ele->avg_p_s/T_cycle;
					op_stat_write_t(prob_throughput_handle,avg_q_stat_ele->throughput,list_i+1);
				}
				
			}else{
				op_sim_message("Error!!!!!!!!! in queue_stat() function","");
			}
		
		}
	
	}
//out
	FOUT;
}

//simulation end
static void simulation_end(void)
{
//var

//in
	FIN(simulation_end());

//body
	
	power_stat();
	record_grade();
	//queue_stat();
	delay_stat();
	throughput_stat();
	
	op_prg_list_free (data_id_list);
	op_prg_mem_free (data_id_list);
	
	op_prg_list_free (routing_table_list);
	op_prg_mem_free (routing_table_list);
	
	op_prg_list_free (queue_stat_list);
	op_prg_mem_free (queue_stat_list);

	
	stat_based_on_prob();

	if(avg_queue_stat_list!=NULL)//defined in header block
	{
		op_prg_list_free (avg_queue_stat_list);
		op_prg_mem_free (avg_queue_stat_list);
		avg_queue_stat_list = NULL;
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
	void pri_proc_lwsn (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_pri_proc_lwsn_init (int * init_block_ptr);
	void _op_pri_proc_lwsn_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_pri_proc_lwsn_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_pri_proc_lwsn_alloc (VosT_Obtype, int);
	void _op_pri_proc_lwsn_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
pri_proc_lwsn (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (pri_proc_lwsn ());

		{
		/* Temporary Variables */
		Packet* pk_TV=NULL;
		Packet* data_pk_TV=NULL;
		int pk_type;
		double time;
		int dest_TV;
		int grade_TV;
		int hop_num_TV;
		int pk_size;
		//int pk_ddc;
		int address_TV;
		
		//for "queue_stat_list"
		int list_i,list_j,*appear_num;
		double *q_prob;
		//File
		FILE *in;
		char temp_file_name[300];
		//char message[512];
		int data_id;
		
		DataID * id_TV;
		
		Avg_Q_Stat* avg_q_stat_ele;
		//For remote statistics
		int src_grade; //record the source grade when receiving a data packet
		Ici* iciptr_TV;
		//char ici_format_name_TV[128];
		//SinkNeighbor* sink_neighbor;
		
		/* End of Temporary Variables */


		FSM_ENTER ("pri_proc_lwsn")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "pri_proc_lwsn [init enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [init enter execs]", state0_enter_exec)
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
				
				
				if(DEBUG)
					printf("data_rate=%f\n",data_rate);
				
				pk_TV = op_pk_create_fmt("MAC_DATA_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durDATA=(pk_size)/data_rate;
				op_pk_destroy(pk_TV);
				if(DEBUG)
					printf("durDATA=%f\n",durDATA);
				
				pk_TV = op_pk_create_fmt("MAC_RTS_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durRTS=(pk_size)/data_rate;
				op_pk_destroy(pk_TV);
				if(DEBUG)
					printf("durRTS=%f\n",durRTS);
				
				pk_TV = op_pk_create_fmt("MAC_CTS_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durCTS=(pk_size)/data_rate;
				op_pk_destroy(pk_TV);
				if(DEBUG)
					printf("durCTS=%f\n",durCTS);
				
				pk_TV = op_pk_create_fmt("MAC_ACK_PK");
				pk_size=op_pk_total_size_get(pk_TV);
				durACK=(pk_size)/data_rate;
				op_pk_destroy(pk_TV);
				if(DEBUG)
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
				
				op_ima_obj_attr_get(node_id, "Steady Time",&steady_time);
				op_ima_obj_attr_get(node_id, "Stat Stop Time",&stat_stop_time);
				op_ima_obj_attr_get(node_id, "Node Type",&node_type);
				op_ima_obj_attr_get(node_id, "user id", &node_address);
				op_ima_obj_attr_get(node_id, "Line Hops",&line_hops);
				op_ima_obj_attr_get(node_id, "Maximum Retransmission", &remainder_retran_num);
				op_ima_obj_attr_get(op_id_from_name(op_id_from_name(op_topo_parent(node_id),OPC_OBJTYPE_NDFIX,"Deployments_Admin"),\
					OPC_OBJTYPE_PROC,"Deployment_Admin"),"Chain Number",&chain_number);
				
				if(node_type==sensor && node_address == 0)
				{
					op_ima_obj_attr_get(node_id, "name", &node_address);
					op_ima_obj_attr_set(node_id, "user id", node_address);
					op_sim_end("Error: node address has been set already\n","","","");
				}
				if(node_type == sink && node_address == 0)
				{
					op_ima_obj_attr_set(node_id, "user id", -1);
				}
				
				//op_ima_obj_attr_get(process_id, "Communication Range",&range);
				//op_ima_obj_attr_set(node_id, "Communication Range",range);
				
				op_ima_obj_attr_get(rx_id, "state", &transceiver_state);
				
				routing_table_list=op_prg_list_create();
				CW_grade = CW*1100;
				slot_time_grade = 0.001;
				grade = -1;
				//op_ima_obj_attr_set(process_id,"grade",grade);
				op_ima_obj_attr_set(node_id,"grade",grade);
				//sink_id = -1;
				
				all_sleeped_time = 0;
				all_active_time = 0;
				all_tx_time = 0;
				all_rx_time = 0;
				slot_time = 0.001; //the time duration of one slot is 0.001 seconds, i.e., 1ms
				
				data_id_list=op_prg_list_create();
				
				mac_flags=(PriT_Mac_Flags *) op_prg_mem_alloc (sizeof (PriT_Mac_Flags));
				/*
				if(op_subq_stat(data_queue,OPC_QSTAT_PKSIZE)==queue_length){
					mac_flags->queue_full=OPC_TRUE;
				}else{
					mac_flags->queue_full=OPC_FALSE;
				}*/
				//Initialization of the used flags: mac_flags
				//mac_flags->queue_full=OPC_FALSE;
				mac_flags->busy_before_grade=OPC_FALSE;
				mac_flags->busy_before_rts=OPC_FALSE;
				mac_flags->busy_before_cts=OPC_FALSE;
				mac_flags->queue_inf=OPC_FALSE;
				//mac_flags->post_backoff=OPC_FALSE;
				mac_flags->sent_rts = OPC_FALSE; //-2:表示倒数第二所加
				mac_flags->received_cts = OPC_FALSE;//-1:表示最后所加
				mac_flags->received_ack = OPC_FALSE;
				mac_flags->received_data = OPC_FALSE;
				
				mac_flags->stat_flag = OPC_FALSE;
				mac_flags->write_stat_flag = OPC_FALSE;
				
				
				//op_ima_obj_attr_get(process_id,"Post Backoff",&mac_flags->post_backoff);
				
				//obtain the queue length
				queue_id = op_topo_child(node_id,OPC_OBJTYPE_QUEUE,0);
				subq_comp_attr_id = op_topo_child(queue_id,OPC_OBJTYPE_COMP,0);
				sub_queue_id = op_topo_child(subq_comp_attr_id,OPC_OBJTYPE_SUBQ,1);
				//queue_length's type is double
				op_ima_obj_attr_get(sub_queue_id,"pk capacity",&queue_length);
				queue_length_add_one = -1;
				
				if(queue_length==INFINITY){
					mac_flags->queue_inf=OPC_TRUE;
					queue_length_add_one = queue_length;
					if(DEBUG) printf("it's infinity!!!\n");
				}else{
					if(DEBUG) printf("it's not infinity!!!\n");
					//queue_length_add_one=queue_length+1.0;
					queue_length_add_one=queue_length;
					op_ima_obj_attr_set(sub_queue_id,"pk capacity",queue_length_add_one);
					
					mac_flags->queue_inf=OPC_FALSE;
					queue_stat_list = op_prg_list_create();
					for(list_i=0;list_i<=queue_length;list_i++){
						appear_num=(int*)op_prg_mem_alloc(sizeof(int));
						*appear_num=0;
						op_prg_list_insert(queue_stat_list,appear_num,OPC_LISTPOS_TAIL);
					}
				}
				if (DEBUG) {
					printf("queue length:\n");
					printf("The maximum queue length is %f, queue_length_add_on is:%f.\n",queue_length,queue_length_add_one);
				}
				
				//data_nums=0;
				
				//durActive contains two parts: durReceive & durSedn, and durReceive==durSend
				if(one_two_cw == 2){
					durActive_half = 2*CW+durRTS+durCTS+durDATA+durACK+DIFS+3*SIFS;
				}else if(one_two_cw == 1){
					durActive_half = CW+durRTS+durCTS+durDATA+durACK+DIFS+3*SIFS; 
				}
				if(DEBUG)
					printf("durActive_half=%f, T_cycle=%f\n",durActive_half,(sleep_factor+2)*durActive_half);
				
				durSleep = sleep_factor * durActive_half;
				if(node_type == sink)
				{
					if(node_address==-2){
						start_grade_timer=5.0;
					}
					op_intrpt_schedule_self(op_sim_time() + start_grade_timer,START_GRADE_CODE);
				}	
				
				op_intrpt_schedule_self(op_sim_time() + start_schedule_timer, START_SCHEDULE_CODE);
				
				hops_avg_delay=0.0;
				hops_pkt_num=0;
				
				//statistics regester
				QLength_handle = op_stat_reg("Queue Length", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				QStatus_handle = op_stat_reg("Queue Status",OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				Hops_avg_delay_handle = op_stat_reg("Average Packet Delay",OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
				
				avg_dc_handle = op_stat_reg("General.Average Duty Cycle", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				lost_pkt_col_handle = op_stat_reg("General.Lost Pkt Due Collision", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				lost_pkt_of_handle = op_stat_reg("General.Lost Pkt Due Overflow", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				avg_energy_handle = op_stat_reg("General.Average Energy Consumption", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				avg_queueing_delay_handle = op_stat_reg("General.Average Queueing Delay", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				
				win_prob_handle = op_stat_reg("General.Win Probability", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				send_prob_handle = op_stat_reg("General.Send Probability", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				receive_prob_handle = op_stat_reg("General.Receive Probability", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				busy_prob_handle = op_stat_reg("General.Busy Probability", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				prob_dc_handle = op_stat_reg("General.Prob Duty Cycle", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				prob_delay_to_sink_handle = op_stat_reg("General.Prob Delay to Sink", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				prob_throughput_handle = op_stat_reg("General.Prob Throughput", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				
				recvd_pkt_num_at_sink = 0.0;
				duty_cycle_num_at_sink = 0.0;
				throughput2_handle = op_stat_reg("General.Throughput2", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				
				win_success = 0.0;
				sense_busy = 0.0;
				send_success = 0.0;
				total_send_trial = 0.0;
				receive_success = 0.0;
				total_receive_trial = 0.0;
				avg_duty_cycle = 0.0;
				//to calculate the average queueing delay
				avg_queueing_delay = 0.0;
				//sent_pkts_num = 0.0;
				
				sum_src_delay=0.0;
				src_pkt_num=0.0;
				sum_relay_delay=0.0;
				relay_pkt_num=0.0;
				
				if(avg_queue_stat_list == NULL){
					avg_queue_stat_list=op_prg_list_create();
					for(list_i=0;list_i<line_hops;list_i++){
						avg_q_stat_ele = (Avg_Q_Stat*)op_prg_mem_alloc(sizeof(struct Avg_Q_Stat));
						
						avg_q_stat_ele->avg_q_prob_list=op_prg_list_create();
						
						for(list_j=0;list_j<=queue_length;list_j++){
							q_prob=(double *)op_prg_mem_alloc(sizeof(double));
							*q_prob=0.0;
							op_prg_list_insert(avg_q_stat_ele->avg_q_prob_list,q_prob,OPC_LISTPOS_TAIL);
						}
						
						avg_q_stat_ele->avg_p_t=0.0;
						avg_q_stat_ele->avg_p_s=0.0;
						avg_q_stat_ele->avg_p_r=0.0;
						
						avg_q_stat_ele->delay_to_sink=0.0;
						avg_q_stat_ele->chain_num = 0;
						avg_q_stat_ele->duty_cycle=0.0;
						avg_q_stat_ele->throughput =0.0;
						
						op_prg_list_insert(avg_queue_stat_list,avg_q_stat_ele,OPC_LISTPOS_TAIL);
					}
				}
				//op_ima_obj_attr_get (process_id, "subqueue", &subq_objid);
				//num_subqs = op_topo_child_count (subq_objid, OPC_OBJTYPE_SUBQ);
				//printf("子队列个数:%d\n",num_subqs);
				
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "pri_proc_lwsn [init exit execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [init exit execs]", state0_exit_exec)
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
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_1", "pri_proc_lwsn [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "pri_proc_lwsn [idle enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [idle enter execs]", state1_enter_exec)
				{
				///*
				if(CHECK_CHANNEL){
					if(op_ev_valid(send_gm_event)){
						//op_ev_cancel(send_gm_event);
						//send_gm_event = op_intrpt_schedule_self(op_sim_time() + op_dist_exponential(send_gm_timer), SEND_GM_CODE);
						mac_flags->busy_before_grade=OPC_TRUE;
				
					}
				}
				
				if(DECREASE_CW){
					if(mac_flags->busy_before_grade == OPC_TRUE){
						//The channel is sensed busy, reset the flag and timer.
						mac_flags->busy_before_grade = OPC_FALSE;
						
						CW_counter = op_dist_outcome(op_dist_load("uniform_int",0.0,CW_grade*1000-1)); // from [0,W-1]
						if(CW_counter == 0.0){
							op_intrpt_schedule_self(op_sim_time()+0.0,SEND_GM_CODE);
						}else{
							send_gm_event = op_intrpt_schedule_self(op_sim_time()+slot_time_grade,DECREASE_CW_CODE);
							CW_counter--;
						}
					}else if(CW_counter == 0.0){
						op_intrpt_schedule_self(op_sim_time()+0.0,SEND_GM_CODE);		
					}else{
						send_gm_event = op_intrpt_schedule_self_mt(op_sim_time()+slot_time_grade,DECREASE_CW_CODE);
						CW_counter--;
					}
				}
				//*/
				
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
				
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
				
					op_prg_list_free (queue_stat_list);
					op_prg_mem_free (queue_stat_list);
				
				}
				
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(DEBUG)
						printf("In \"idle\" state: received DATA which is from \"gsrc\".\n");
					upper_data_pk_proc();
				}
				}
				FSM_PROFILE_SECTION_OUT (state1_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"pri_proc_lwsn")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "pri_proc_lwsn [idle exit execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [idle exit execs]", state1_exit_exec)
				{
				/*
				if(node_type!=sink)
					grade=1;
				
				*/
				}
				FSM_PROFILE_SECTION_OUT (state1_exit_exec)


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [idle trans conditions]", state1_trans_conds)
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
				FSM_CASE_TRANSIT (0, 6, state6_enter_exec, ;, "CONTEND_TO_SEND_RTS", "", "idle", "T:second contention", "tr_28", "pri_proc_lwsn [idle -> T:second contention : CONTEND_TO_SEND_RTS / ]")
				FSM_CASE_TRANSIT (1, 4, state4_enter_exec, ;, "GO_TO_SLEEP", "", "idle", "sleeping_S", "tr_32", "pri_proc_lwsn [idle -> sleeping_S : GO_TO_SLEEP / ]")
				FSM_CASE_TRANSIT (2, 5, state5_enter_exec, ;, "WAKING_TO_WAIT_RTS", "", "idle", "R:waking", "tr_34", "pri_proc_lwsn [idle -> R:waking : WAKING_TO_WAIT_RTS / ]")
				FSM_CASE_TRANSIT (3, 17, state17_enter_exec, ;, "START_GRADE", "", "idle", "start grade", "tr_97", "pri_proc_lwsn [idle -> start grade : START_GRADE / ]")
				FSM_CASE_TRANSIT (4, 3, state3_enter_exec, ;, "SEND_GM", "", "idle", "send gm", "tr_100", "pri_proc_lwsn [idle -> send gm : SEND_GM / ]")
				FSM_CASE_TRANSIT (5, 2, state2_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "idle", "GM proc", "tr_102", "pri_proc_lwsn [idle -> GM proc : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (6, 1, state1_enter_exec, ;, "default", "", "idle", "idle", "tr_108", "pri_proc_lwsn [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (GM proc) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "GM proc", state2_enter_exec, "pri_proc_lwsn [GM proc enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [GM proc enter execs]", state2_enter_exec)
				{
				pk_TV = op_pk_get(RCV_STRM);
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				if(pk_type == grade_pk)
				{
					grade_pk_proc(pk_TV);
				}
				else
				{
					printf("In \"GM proc\" state, the type of the received packet is: %d, grade=%d.\n",pk_type,grade);
					op_pk_destroy(pk_TV);
					op_sim_end("Waiting Grade Message Err: Are you kidding me?","I'm waiting Grade Message","Not you","请仔细检查");
				
				}
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (GM proc) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "GM proc", "pri_proc_lwsn [GM proc exit execs]")


			/** state (GM proc) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "GM proc", "idle", "tr_103", "pri_proc_lwsn [GM proc -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (send gm) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "send gm", state3_enter_exec, "pri_proc_lwsn [send gm enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [send gm enter execs]", state3_enter_exec)
				{
				if(op_subq_empty(grade_queue) == OPC_FALSE)
				{
					pk_TV = op_subq_pk_remove(grade_queue, OPC_QPOS_HEAD);
					op_pk_send(pk_TV,SEND_STRM);
					//printf("In \"send gm\" state. The grade message is removed!\n");
				}
				
				else
				{
					op_sim_end("Err: grade queue is empty, simulation is over!","","","");
				}
				
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (send gm) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "send gm", "pri_proc_lwsn [send gm exit execs]")


			/** state (send gm) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "send gm", "idle", "tr_101", "pri_proc_lwsn [send gm -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (sleeping_S) enter executives **/
			FSM_STATE_ENTER_UNFORCED (4, "sleeping_S", state4_enter_exec, "pri_proc_lwsn [sleeping_S enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [sleeping_S enter execs]", state4_enter_exec)
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
					//set timer,when wake up, go to "waking" state (R:-1)
					//iciptr_TV = op_ici_create("slot_ici");
					//op_ici_attr_set(iciptr_TV, "current slot",-1);
					//op_ici_attr_set(iciptr_TV, "intrpt source",from_ddc);
					//op_ici_install (iciptr_TV);
					op_intrpt_schedule_self(op_sim_time() + time, TO_WAKE_UP_CODE);
					//op_ici_install (OPC_NIL);
					if(DEBUG)
						printf("Node:%d, Grade:%d, time:%f,in sleeping period.\n\
							Initial to sleeping.\n\
							After %f seconds, go to wake up.\n",\
							node_address,grade,op_sim_time(),time);
					
					//time statistic
					record_time = op_sim_time();
				}
				else
				if(op_intrpt_code() == SLEEP_FROM_FIRST_CODE)//sleep from first period,i.e.,R state
				{
					//time = CW+durCTS+durDATA+durACK+3*SIFS;
					//time = waking_time + durActive_half - op_sim_time();
					
					if(DEBUG_2ND) printf("Node: %d. Time: %f. in sleeping from R.\n",node_address, op_sim_time());
						
					//if (node_type == sensor){
						
						if(!op_ev_valid(regular_wakeup_evhandle)){
							//when wake up, go to "second contention" state (T:0)
							//current_slot_TV=sv_current_slot+1;
							//printf("Node: %d, Time: %f. current_slot_TV: %d.\n",node_address, op_sim_time(),current_slot_TV);
							time = waking_time + durActive_half - op_sim_time();
							//iciptr_TV = op_ici_create("slot_ici");
							//op_ici_attr_set(iciptr_TV, "current slot",sv_current_slot+1);
							//op_ici_attr_set(iciptr_TV, "intrpt source",from_ddc_lht_hrc);
							//op_ici_install (iciptr_TV);
							op_intrpt_schedule_self(op_sim_time() + time, WAKING_TO_SEND_RTS_CODE);
							//op_ici_install (OPC_NIL);
							if(DEBUG)
								printf("Node:%d, Grade:%d, time:%f,in sleeping period.\n\
									To sleeping from the first period.\n\
									After %f seconds, go to second period.\n",\
									node_address,grade,op_sim_time(),time);
						}
					
						//time statistic
						if(mac_flags->stat_flag) {
							all_active_time = all_active_time+(op_sim_time()-record_time);
						}
						record_time = op_sim_time();
					
					/*}else if(node_type == sink){
						time = waking_time + durActive_half - op_sim_time() + (sleep_factor+1)*durActive_half;
						//when wake up, go to "waking" state	
						op_intrpt_schedule_self(op_sim_time() + time, TO_WAKE_UP_CODE);
						
						if(DEBUG)
							printf("Node:%d, Grade:%d, time:%f,sink node in sleeping period.\n\
								from the first period.\n\
								After %f seconds, go to wake up.\n\n",\
								node_address,grade,op_sim_time(),time);
					
						//time statistic
						if(mac_flags->stat_flag) {
							all_active_time = all_active_time+(op_sim_time()-record_time);
						}
						record_time = op_sim_time();
					}
					*/
				}
				else
				if(op_intrpt_code() == SLEEP_FROM_SECOND_CODE)//sleep from second period,i.e.,T state
				{
				
					
					if(mac_flags->sent_rts == OPC_TRUE && mac_flags->received_cts == OPC_FALSE){
						//didn't receive CTS, need to remove the data from the queue
						//In other words, collision happens
					
				
						mac_flags->sent_rts = OPC_FALSE;//重置是否发送了rts flag。
						//op_sim_end("WARNING:retransmission is necessary!","","","");
						if(remainder_retran_num == 0)
						{
							op_pk_destroy(op_subq_pk_remove(data_queue, OPC_QPOS_HEAD));
							op_ima_obj_attr_get(node_id, "Maximum Retransmission", &remainder_retran_num);
							
						}else if( remainder_retran_num > 0 ){
							remainder_retran_num--;
						}else{
							op_sim_end("Err: remainder_retran_num should not be negative!\n","","","");
						}
					}
					
				
					
					if(!op_ev_valid(regular_wakeup_evhandle))//还没有设置过正常醒来事件,说明刚刚的 T slot是真正的T （而不是sleeping里的T）
					{
						
						//printf("Node: %d. Time: %f. current slot: %d. in \"sleeping from T\"\n",node_address,op_sim_time(),sv_current_slot);
				
						time = second_contending_time + durActive_half + durSleep - op_sim_time();
					
						//when wake up, go to "waking" state (R:-1)
						/*
						iciptr_TV = op_ici_create("slot_ici");
						op_ici_attr_set(iciptr_TV, "current slot",-1);
						op_ici_attr_set(iciptr_TV, "intrpt source",from_ddc);
						op_ici_install (iciptr_TV);
						*/
						regular_wakeup_evhandle = op_intrpt_schedule_self(op_sim_time() + time, TO_WAKE_UP_CODE);
						//op_ici_install (OPC_NIL);
						
						if(DEBUG_2ND) printf("Node: %d, Time: %f. in S sleeping from T. set regular_wakeup event.\n",node_address, op_sim_time());
						
						if(DEBUG)
								printf("Node:%d, Grade:%d, time:%f,in sleeping period.\n\
									To sleeping from second period.\n\
									After %f seconds, go to \"wakeing\" state.\n",\
									node_address,grade,op_sim_time(),time);
				
					}
					
				
					
					//time statistic
					if(mac_flags->stat_flag) {
						all_active_time = all_active_time+(op_sim_time()-record_time);
					}
					record_time = op_sim_time();
				}
				
				
				
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(DEBUG)
						printf("Node: %d, In \"sleeping\" state: received DATA which is from \"gsrc\".\n",node_address);
					upper_data_pk_proc();
				}
				
				if((FROM_LOWER && !transceiver_state)||(FROM_LOWER && node_type==sink))
				{	
					op_pk_destroy(op_pk_get(RCV_STRM));
					if(DEBUG)
						printf("Node:%d, time:%f. Node is sleeping, destroy the received pk.\n",\
							node_address,op_sim_time());
				}
				if(END)
				{
				/*
					all_sleeped_time = all_sleeped_time + (op_sim_time()-record_time);
					power_stat();
					record_grade();
					queue_stat();
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
					op_prg_list_free (queue_stat_list);
					op_prg_mem_free (queue_stat_list);
				*/
					simulation_end();
				}
				//close transceiver
				transceiver_state = OPC_FALSE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (9,"pri_proc_lwsn")


			/** state (sleeping_S) exit executives **/
			FSM_STATE_EXIT_UNFORCED (4, "sleeping_S", "pri_proc_lwsn [sleeping_S exit execs]")


			/** state (sleeping_S) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [sleeping_S trans conditions]", state4_trans_conds)
			FSM_INIT_COND (WAKING_UP)
			FSM_TEST_COND (WAKING_TO_SEND_RTS)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("sleeping_S")
			FSM_PROFILE_SECTION_OUT (state4_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 5, state5_enter_exec, ;, "WAKING_UP", "", "sleeping_S", "R:waking", "tr_37", "pri_proc_lwsn [sleeping_S -> R:waking : WAKING_UP / ]")
				FSM_CASE_TRANSIT (1, 6, state6_enter_exec, ;, "WAKING_TO_SEND_RTS", "", "sleeping_S", "T:second contention", "tr_40", "pri_proc_lwsn [sleeping_S -> T:second contention : WAKING_TO_SEND_RTS / ]")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "default", "", "sleeping_S", "sleeping_S", "tr_91", "pri_proc_lwsn [sleeping_S -> sleeping_S : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (R:waking) enter executives **/
			FSM_STATE_ENTER_FORCED (5, "R:waking", state5_enter_exec, "pri_proc_lwsn [R:waking enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [R:waking enter execs]", state5_enter_exec)
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
				
				
				mac_flags->received_data = OPC_FALSE;
				
				
				//enable transceiver
				transceiver_state = OPC_TRUE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				
				waking_time = op_sim_time();
				if(mac_flags->stat_flag) {total_receive_trial++;}
				
				//if(mac_flags->queue_inf==OPC_TRUE || op_subq_stat(data_queue,OPC_QSTAT_PKSIZE)<queue_length_add_one){ //queue is not full
					time = CW+DIFS+durRTS  + CUSHION_TIME;//多等一个 等待 time
					wait_RTS_event = op_intrpt_schedule_self(op_sim_time() + time, SLEEP_FROM_FIRST_CODE);
				//}else{ //queue is full, cannot receive anything, go to sleep immediately.
				//	op_intrpt_schedule_self(op_sim_time() + 0.0, SLEEP_FROM_FIRST_CODE);
				//}
				
				if(DEBUG)
					printf("Node:%d, Grade:%d, time:%f, entered into first period (\"waking\" state).\n\
					    Waiting RTS.\n\
						If no RTS, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				
				//time statistic
				//if(node_type == sensor){
				if(mac_flags->stat_flag) {
					all_sleeped_time = all_sleeped_time + (op_sim_time() - record_time);
				}
				record_time = op_sim_time();
				
				//}
				}
				FSM_PROFILE_SECTION_OUT (state5_enter_exec)

			/** state (R:waking) exit executives **/
			FSM_STATE_EXIT_FORCED (5, "R:waking", "pri_proc_lwsn [R:waking exit execs]")


			/** state (R:waking) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "R:waking", "R:first", "tr_57", "pri_proc_lwsn [R:waking -> R:first : default / ]")
				/*---------------------------------------------------------*/



			/** state (T:second contention) enter executives **/
			FSM_STATE_ENTER_FORCED (6, "T:second contention", state6_enter_exec, "pri_proc_lwsn [T:second contention enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [T:second contention enter execs]", state6_enter_exec)
				{
				if(DEBUG_2ND) printf("Node: %d. Time: %f. In T state. \n",\
					node_address,op_sim_time());
				
				
				//=============statistics===============//
				if(op_sim_time()>=steady_time && op_sim_time()<=stat_stop_time){
					mac_flags->stat_flag = OPC_TRUE;
					mac_flags->write_stat_flag = OPC_TRUE;
					if(node_type == sink){
						duty_cycle_num_at_sink ++;
					}
				}else{
					mac_flags->stat_flag = OPC_FALSE;
				}
				
				if((!mac_flags->stat_flag) && (mac_flags->write_stat_flag)){
					mac_flags->write_stat_flag = OPC_FALSE;
					
					queue_stat();
				}
				//=============statistics===============//
				
				
				
				//The number of packets in the queue at the beginning of T state.
				if(mac_flags->stat_flag){
					op_stat_write(QLength_handle, op_subq_stat(data_queue,OPC_QSTAT_PKSIZE));
				}
				
				//calculate the number of times a specific number of packets 
				//appearing in the queue if the queue length is not infinity.
				if((!mac_flags->queue_inf) && mac_flags->stat_flag)
				{
					list_i=op_subq_stat(data_queue,OPC_QSTAT_PKSIZE);
					appear_num = op_prg_list_access(queue_stat_list,list_i);
					(*appear_num)++;
					if(DEBUG)
					{
						printf("queue_stat_list size: %d.\n",op_prg_list_size(queue_stat_list));
					}
				}
				
				//Log record
				if(CONTEND_TO_SEND_RTS)
				{
					record_time = op_sim_time();
				}
				second_contending_time = op_sim_time();
				
				/*
				1. check data_queue
				   if(Y) open transceiver, contending for sending RTS
				   if(N) go to sleep
				*/
				
				//enable transceiver
				transceiver_state = OPC_TRUE;
				op_ima_obj_attr_set(rx_id, "state", transceiver_state);
				
				//set corresponding flag
				mac_flags->sent_rts = OPC_FALSE; //还没有发送RTS
				mac_flags->received_cts = OPC_FALSE;//假设没有收到CTS，flag设为false：表示等待CTS。若成功收到CTS，该flag需设为true。
				mac_flags->received_ack = OPC_FALSE;//waiting for ack
				
				
				if(op_subq_empty(data_queue) == OPC_FALSE) //|| mac_flags->post_backoff==OPC_TRUE)
				{//data queue is not empty, or post backoff is set
				//	time = op_dist_uniform(CW*1000.0)/1000.0 + DIFS;
					
					//debug=OPC_TRUE;
				    if(mac_flags->stat_flag) {total_send_trial++;}
				
					CW_counter = op_dist_outcome(op_dist_load("uniform_int",0.0,CW*1000.0-1)); // from [0,W-1]
					if(DEBUG) printf("1. in send rts, CW_counter = %f.\n",CW_counter);
					
					if(CW_counter == 0.0){
						if(mac_flags->stat_flag) 
						{
							win_success++;
						}
						op_intrpt_schedule_self(op_sim_time()+DIFS+0.0,SEND_RTS_CODE);
						
					}else{
						decrease_CW_event = op_intrpt_schedule_self(op_sim_time()+DIFS+slot_time,DECREASE_CW_CODE);
						CW_counter--;
					}
					//time = time/1000.0 + DIFS;
					//if(DEBUG) printf("2. in send rts, CW_counter = %f.\n",CW_counter);
					
					if(DEBUG_2ND)
						printf("Node:%d, Grade:%d, time:%f, in T.\n\
							data_queue is not empty, Contending to send RTS.\n\
							Waiting to send RTS.\n",\
							node_address,grade,op_sim_time());
					//sprintf(message,"Contend to send RTS, op_dist_uniform(CW)=%f\n",time-DIFS);
					//op_sim_message("",message);
				}
				else//data queue is empty and post_backoff is not used, go to sleep
				{
					//debug=OPC_FALSE;
				
					op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
					if(DEBUG_2ND)
						printf("Node:%d, Grade:%d, time:%f, in T.\n\
							data_queue is empty.\n\
							Go to sleep immediately.\n",\
							node_address,grade,op_sim_time());
				}
				
				//time statistic
				if(mac_flags->stat_flag) {
					all_sleeped_time = all_sleeped_time + (op_sim_time()-record_time);
				}
				record_time = op_sim_time();
				}
				FSM_PROFILE_SECTION_OUT (state6_enter_exec)

			/** state (T:second contention) exit executives **/
			FSM_STATE_EXIT_FORCED (6, "T:second contention", "pri_proc_lwsn [T:second contention exit execs]")


			/** state (T:second contention) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "T:second contention", "T:second", "tr_59", "pri_proc_lwsn [T:second contention -> T:second : default / ]")
				/*---------------------------------------------------------*/



			/** state (wait CTS) enter executives **/
			FSM_STATE_ENTER_FORCED (7, "wait CTS", state7_enter_exec, "pri_proc_lwsn [wait CTS enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [wait CTS enter execs]", state7_enter_exec)
				{
				//send RTS
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				//if set next hop for routing manually, comment the following codes and add new line of code:
				/***************************************************************************
				if(node_type!=sink && (next_hop_2=rt_pos_find_next())==0)
				{
					op_sim_end("Routing table is empty Err: Are you sure you are not kidding me?","","","");
				}
				***************************************************************************/
				op_ima_obj_attr_get(node_id, "Next Hop",&next_hop_2);//and add new line of code
				
				pk_TV = op_pk_create_fmt("MAC_RTS_PK");
				op_pk_nfd_set(pk_TV, "Type", rts_pk);
				op_pk_nfd_set(pk_TV,"Src", node_address);
				op_pk_nfd_set(pk_TV,"Grade",grade);
				op_pk_nfd_set(pk_TV, "Dest", next_hop_2);
				
				
				
				op_pk_nfd_set(pk_TV, "Dynamic DC", 0);
				
				
				//calculate delay for the pakcet generated by the current node or receiving from previous-hop node
				if(mac_flags->stat_flag){
					data_pk_TV=op_subq_pk_access(data_queue, OPC_QPOS_HEAD);
					op_pk_nfd_get(data_pk_TV,"Src",&address_TV);
					if(address_TV == node_address){ //the packet is generated by the current node
						op_pk_nfd_get(data_pk_TV,"Create Time", &time);
						sum_src_delay = sum_src_delay + op_sim_time()-time + durRTS+2*SIFS+durCTS+durDATA;
						src_pkt_num ++;
					}else{//the packet is received from a previous-hop node
						op_pk_nfd_get(data_pk_TV,"Receiving Time", &time);
						sum_relay_delay = sum_relay_delay + op_sim_time()-time + durRTS+2*SIFS+durCTS+durDATA;
						relay_pkt_num ++;
					}
				}
				
				
				op_pk_send(pk_TV, SEND_STRM);
				mac_flags->sent_rts = OPC_TRUE;//成功发送rts
				retx++; //发送了一个RTS，假设发生碰撞，则需要重传，故retx增加1.若成功收到CTS,则retx需减一
				
				//time statistic
				if(mac_flags->stat_flag) {
					all_tx_time = all_tx_time + durRTS;
				}
				
				//set timer for waiting CTS from lower grade nodes
				if(one_two_cw == 2){
					time = durRTS+CW+SIFS+durCTS;
				}else if(one_two_cw == 1){
					time = durRTS+SIFS+durCTS + CUSHION_TIME;//多等一个等待时间，否则节点收不到包
				}
				
				wait_CTS_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_SECOND_CODE);
				//mac_flags->received_cts = OPC_FALSE;//假设没有收到CTS，flag设为false：表示等待CTS。若成功收到CTS，该flag需设为true。
				if(DEBUG)
					printf("Node:%d, Grade:%d, time:%f, in second period.\n\
						Have sent RTS.\n\
						Waiting to receive CTS.\n\
						If no CTS, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state7_enter_exec)

			/** state (wait CTS) exit executives **/
			FSM_STATE_EXIT_FORCED (7, "wait CTS", "pri_proc_lwsn [wait CTS exit execs]")


			/** state (wait CTS) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "wait CTS", "T:second", "tr_89", "pri_proc_lwsn [wait CTS -> T:second : default / ]")
				/*---------------------------------------------------------*/



			/** state (R:first) enter executives **/
			FSM_STATE_ENTER_UNFORCED (8, "R:first", state8_enter_exec, "pri_proc_lwsn [R:first enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [R:first enter execs]", state8_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(DEBUG)
						printf("Node:%d,In \"first\" state: received DATA which is from \"gsrc\".\n",node_address);
					upper_data_pk_proc();
				}
				
				if(END)
				{
				/*
					all_active_time = all_active_time+(op_sim_time()-record_time);
					power_stat();
					record_grade();
					queue_stat();
					
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
					op_prg_list_free (queue_stat_list);
					op_prg_mem_free (queue_stat_list);
				*/
					simulation_end();
				}
				
				if(DECREASE_CW && one_two_cw==2){
					if(mac_flags->busy_before_cts == OPC_TRUE){
						//The channel is sensed busy, reset the flag and go to sleep immediately.
						mac_flags->busy_before_cts = OPC_FALSE;
						op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_FIRST_CODE);
						if(DEBUG)
							printf("In \"first\" state: The channel is sensed busy, \
								reset the flag and go to sleep immediately.\n\n");
					}else if(CW_counter == 0){
						op_intrpt_schedule_self(op_sim_time()+0.0,SEND_CTS_CODE);
						if(DEBUG)
							printf("In \"second\" state: CW_counter = %f.\n",CW_counter);
					}else{
						decrease_CW_event = op_intrpt_schedule_self_mt(op_sim_time()+slot_time,DECREASE_CW_CODE);
						CW_counter--;
					}
				
				}
				}
				FSM_PROFILE_SECTION_OUT (state8_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (17,"pri_proc_lwsn")


			/** state (R:first) exit executives **/
			FSM_STATE_EXIT_UNFORCED (8, "R:first", "pri_proc_lwsn [R:first exit execs]")


			/** state (R:first) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [R:first trans conditions]", state8_trans_conds)
			FSM_INIT_COND (SLEEP_FROM_FIRST)
			FSM_TEST_COND (FROM_LOWER && transceiver_state)
			FSM_TEST_COND (SEND_CTS)
			FSM_TEST_COND (SEND_ACK)
			FSM_TEST_COND (CHECK_CHANNEL)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("R:first")
			FSM_PROFILE_SECTION_OUT (state8_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 4, state4_enter_exec, ;, "SLEEP_FROM_FIRST", "", "R:first", "sleeping_S", "tr_56", "pri_proc_lwsn [R:first -> sleeping_S : SLEEP_FROM_FIRST / ]")
				FSM_CASE_TRANSIT (1, 12, state12_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "R:first", "RCV(1)", "tr_74", "pri_proc_lwsn [R:first -> RCV(1) : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (2, 13, state13_enter_exec, ;, "SEND_CTS", "", "R:first", "wait DATA", "tr_76", "pri_proc_lwsn [R:first -> wait DATA : SEND_CTS / ]")
				FSM_CASE_TRANSIT (3, 14, state14_enter_exec, ;, "SEND_ACK", "", "R:first", "send ACK", "tr_79", "pri_proc_lwsn [R:first -> send ACK : SEND_ACK / ]")
				FSM_CASE_TRANSIT (4, 15, state15_enter_exec, ;, "CHECK_CHANNEL", "", "R:first", "carrier sense(1)", "tr_93", "pri_proc_lwsn [R:first -> carrier sense(1) : CHECK_CHANNEL / ]")
				FSM_CASE_TRANSIT (5, 8, state8_enter_exec, ;, "default", "", "R:first", "R:first", "tr_90", "pri_proc_lwsn [R:first -> R:first : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (T:second) enter executives **/
			FSM_STATE_ENTER_UNFORCED (9, "T:second", state9_enter_exec, "pri_proc_lwsn [T:second enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [T:second enter execs]", state9_enter_exec)
				{
				//Received DATA from upper layer
				if(FROM_UPPER)
				{
					if(DEBUG)
						printf("Node:%d, In \"second\" state: received DATA which is from \"gsrc\".\n",node_address);
					upper_data_pk_proc();
				}
				if(END)
				{
				/*
					all_active_time = all_active_time + (op_sim_time()-record_time);
					power_stat();
					record_grade();
					queue_stat();
						
					op_prg_list_free (data_id_list);
					op_prg_mem_free (data_id_list);
					op_prg_list_free (routing_table_list);
					op_prg_mem_free (routing_table_list);
					
					//if(mac_flags->queue_inf==OPC_FALSE)
					op_prg_list_free (queue_stat_list);
					op_prg_mem_free (queue_stat_list);
				*/
					simulation_end();
				}
				if(DECREASE_CW){
					if(mac_flags->busy_before_rts == OPC_TRUE){
						//The channel is sensed busy, reset the flag and go to sleep immediately.
						mac_flags->busy_before_rts = OPC_FALSE;
						if(mac_flags->stat_flag)  
						{
							sense_busy++;
						}
						op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
						if(DEBUG)
							printf("In \"second\" state: The channel is sensed busy, \
								reset the flag and go to sleep immediately.\n\n");
					}else if(CW_counter == 0){
						if(op_subq_empty(data_queue) == OPC_TRUE){
							op_sim_end("Error in \"T:second\"","","","");
							op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
						}else{
							if(mac_flags->stat_flag) {win_success++;}
							op_intrpt_schedule_self(op_sim_time()+0.0,SEND_RTS_CODE);
						}
						if(DEBUG)
							printf("In \"second\" state: CW_counter = %f.\n",CW_counter);
					}else{
						decrease_CW_event = op_intrpt_schedule_self(op_sim_time()+slot_time,DECREASE_CW_CODE);
						CW_counter--;
					}
				}
				}
				FSM_PROFILE_SECTION_OUT (state9_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (19,"pri_proc_lwsn")


			/** state (T:second) exit executives **/
			FSM_STATE_EXIT_UNFORCED (9, "T:second", "pri_proc_lwsn [T:second exit execs]")


			/** state (T:second) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [T:second trans conditions]", state9_trans_conds)
			FSM_INIT_COND (SEND_RTS)
			FSM_TEST_COND (SLEEP_FROM_SECOND)
			FSM_TEST_COND (FROM_LOWER && transceiver_state)
			FSM_TEST_COND (SEND_DATA)
			FSM_TEST_COND (CHECK_CHANNEL)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("T:second")
			FSM_PROFILE_SECTION_OUT (state9_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 7, state7_enter_exec, ;, "SEND_RTS", "", "T:second", "wait CTS", "tr_60", "pri_proc_lwsn [T:second -> wait CTS : SEND_RTS / ]")
				FSM_CASE_TRANSIT (1, 4, state4_enter_exec, ;, "SLEEP_FROM_SECOND", "", "T:second", "sleeping_S", "tr_61", "pri_proc_lwsn [T:second -> sleeping_S : SLEEP_FROM_SECOND / ]")
				FSM_CASE_TRANSIT (2, 10, state10_enter_exec, ;, "FROM_LOWER && transceiver_state", "", "T:second", "RCV(2)", "tr_66", "pri_proc_lwsn [T:second -> RCV(2) : FROM_LOWER && transceiver_state / ]")
				FSM_CASE_TRANSIT (3, 11, state11_enter_exec, ;, "SEND_DATA", "", "T:second", "send data", "tr_72", "pri_proc_lwsn [T:second -> send data : SEND_DATA / ]")
				FSM_CASE_TRANSIT (4, 16, state16_enter_exec, ;, "CHECK_CHANNEL", "", "T:second", "carrier sense(2)", "tr_95", "pri_proc_lwsn [T:second -> carrier sense(2) : CHECK_CHANNEL / ]")
				FSM_CASE_TRANSIT (5, 9, state9_enter_exec, ;, "default", "", "T:second", "T:second", "tr_92", "pri_proc_lwsn [T:second -> T:second : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (RCV(2)) enter executives **/
			FSM_STATE_ENTER_FORCED (10, "RCV(2)", state10_enter_exec, "pri_proc_lwsn [RCV(2) enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [RCV(2) enter execs]", state10_enter_exec)
				{
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_get(RCV_STRM);
				
				pk_size=op_pk_total_size_get(pk_TV);
				time=(pk_size)/data_rate;
				//time statistic
				//all_rx_time = all_rx_time + time;
				if(DEBUG)
					printf("For sending data, Node:%d, Grade:%d, time:%f.\nReceived a packet, size: %d.\nReceive Duration is: %f.\n",	node_address,grade,op_sim_time(),pk_size,time);
				
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				
				
				if(pk_type == rts_pk)
				{
					//time statistic
					if(mac_flags->stat_flag) {
						all_rx_time = all_rx_time + durRTS;
					}
					//RTS listening during contending to send RTS("uniform(CW)+durDIFS")
					if(op_ev_valid(decrease_CW_event))
					{
						op_ev_cancel(decrease_CW_event);
						op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
						if(DEBUG)
							printf("Node:%d, Grade:%d, time:%f, in second period.\n\
								Failure to send RTS, because received RTS from other nodes.\n\
								Go to sleep immediately.\n",\
								node_address,grade,op_sim_time());
					}
				}
				
				
				if(pk_type == cts_pk)
				{
					//time statistic
					if(mac_flags->stat_flag) {
						all_rx_time = all_rx_time + durCTS;
					}
					
					op_pk_nfd_get(pk_TV,"Grade",&grade_TV);
					op_pk_nfd_get(pk_TV,"Dest",&dest_TV);
					if(grade == grade_TV+1 && dest_TV == node_address)
					{
						if(op_ev_valid(wait_CTS_event))//the first time to receive CTS
						{
							op_ev_cancel(wait_CTS_event);
							op_pk_nfd_get(pk_TV,"Src",&next_hop_2);
							
							retx--;
							
							mac_flags->received_cts = OPC_TRUE; //received the CTS successfully
							
							op_ima_obj_attr_get(node_id, "Maximum Retransmission", &remainder_retran_num);
							
							//wait SIFS time to send DATA
							time = SIFS;
							op_intrpt_schedule_self(op_sim_time()+time,SEND_DATA_CODE);
							if(DEBUG)
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
					if(mac_flags->stat_flag) {
						all_rx_time = all_rx_time + durACK;
					}
					
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
							
							mac_flags->received_ack = OPC_TRUE; //received the ACK successfully
							
							//recieved ACK correctly, go to sleep immediately
							op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
							if(DEBUG)
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
			FSM_STATE_EXIT_FORCED (10, "RCV(2)", "pri_proc_lwsn [RCV(2) exit execs]")


			/** state (RCV(2)) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "RCV(2)", "T:second", "tr_71", "pri_proc_lwsn [RCV(2) -> T:second : default / ]")
				/*---------------------------------------------------------*/



			/** state (send data) enter executives **/
			FSM_STATE_ENTER_FORCED (11, "send data", state11_enter_exec, "pri_proc_lwsn [send data enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [send data enter execs]", state11_enter_exec)
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
				
				if(mac_flags->stat_flag) {
					send_success++;
					
					op_pk_nfd_get(pk_TV,"Create Time",&time);
					avg_queueing_delay += op_sim_time()+durDATA - time; 
				}
				
				op_pk_send(pk_TV,SEND_STRM);
				
				retx++;
				
				//time statistic
				if(mac_flags->stat_flag) {
				all_tx_time = all_tx_time + durDATA;
				}
				
				//set timer for waiting ACK from lower grade node
				time = durDATA + SIFS + durACK + CUSHION_TIME;// 多加一个SIFS，否则收不到包
				wait_ACK_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_SECOND_CODE);
				//mac_flags->received_ack = OPC_FALSE;//waiting for ack
				
				if(DEBUG)
					printf("Node:%d, Grade:%d, time:%f, in second period.\n\
						Have sent DATA, waiting ACK.\n\
						If no ACK, after %f seconds, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state11_enter_exec)

			/** state (send data) exit executives **/
			FSM_STATE_EXIT_FORCED (11, "send data", "pri_proc_lwsn [send data exit execs]")


			/** state (send data) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "send data", "T:second", "tr_73", "pri_proc_lwsn [send data -> T:second : default / ]")
				/*---------------------------------------------------------*/



			/** state (RCV(1)) enter executives **/
			FSM_STATE_ENTER_FORCED (12, "RCV(1)", state12_enter_exec, "pri_proc_lwsn [RCV(1) enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [RCV(1) enter execs]", state12_enter_exec)
				{
				if(pk_TV!=NULL)
				{
					op_pk_destroy(pk_TV);
				}
				
				pk_TV = op_pk_get(RCV_STRM);
				
				pk_size=op_pk_total_size_get(pk_TV);
				time=(pk_size)/data_rate;
				//time statistic
				//all_rx_time = all_rx_time + time;
				if(DEBUG)
					printf("\nFor receiving data, Node:%d, Grade:%d, time:%f.\nReceived a packet, size: %d.\nReceive Duration is: %f.\n\n",	node_address,grade,op_sim_time(),pk_size,time);
				
				op_pk_nfd_get(pk_TV,"Type",&pk_type);
				
				//printf("out: please pay an attention here, 童飞!\n");
				//sprintf(message,"pk_type=%d.\n",pk_type);
				//op_sim_message(message,"");
				
				if(pk_type == rts_pk)
				{
					//time statistic
					if(mac_flags->stat_flag) {
						all_rx_time = all_rx_time + durRTS;
					}
					//wait for RTS
					op_pk_nfd_get(pk_TV,"Grade",&grade_TV);
					op_pk_nfd_get(pk_TV,"Dest",&dest_TV);
					//printf("in: please pay an attention here, 童飞!\n");
					
					//DEBUG=OPC_FALSE;
					
					if(grade_TV == grade+1)
					{
						if(op_ev_valid(wait_RTS_event))//the first time to receive RTS
						{
							op_ev_cancel(wait_RTS_event); 
							
							//debug=OPC_TRUE;
							
							
							if(one_two_cw == 2){
								op_pk_nfd_get(pk_TV,"Src",&next_hop_1);
							
								//prepare to send CTS
								//time = op_dist_uniform(CW*1000.0)/1000.0 + SIFS;			
								CW_counter = op_dist_outcome(op_dist_load("uniform_int",0.0,CW*1000.0-1));
								if(DEBUG) printf("in wait rts. CW_counter=%f.\n",CW_counter);
								if(CW_counter == 0.0){
									op_intrpt_schedule_self(op_sim_time()+SIFS+0.0,SEND_CTS_CODE);
								}else{
									decrease_CW_event = op_intrpt_schedule_self(op_sim_time()+SIFS+slot_time,DECREASE_CW_CODE);
									CW_counter--;
								}
								
								//time = time/1000.0 + SIFS;
								//send_CTS_event = op_intrpt_schedule_self(op_sim_time()+time,SEND_CTS_CODE);
								if(DEBUG)
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
									op_intrpt_schedule_self(op_sim_time()+time,SEND_CTS_CODE);
									if(DEBUG)
										printf("Node:%d, Grade:%d, time:%f, in first period.\n\
												Successfully received RTS.\n\
								    			Waiting to send CTS.\n",\
												node_address,grade,op_sim_time());
									//sprintf(message,"Contend to send CTS, op_dist_uniform(CW)=%f\n",time-DIFS);
									//op_sim_message("",message);
								}else
								{
									if(DEBUG)
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
					if(mac_flags->stat_flag) {
						all_rx_time = all_rx_time + durCTS;
					}
					
					//CTS listening during contending to send CTS("uniform(CW)+DIFS")
					if(op_ev_valid(decrease_CW_event))
					{
						op_ev_cancel(decrease_CW_event);
						op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_FIRST_CODE);
						if(DEBUG)
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
					if(mac_flags->stat_flag) {
						all_rx_time = all_rx_time + durDATA;
					}
					
					op_pk_nfd_get(pk_TV,"Next Hop",&dest_TV);
					if(dest_TV == node_address)
					{
						if(op_ev_valid(wait_DATA_event))
						{
							//op_sim_message("data arrived at the sink!","");
							if(mac_flags->stat_flag) 
							{
								receive_success++;
							}
							
							mac_flags->received_data = OPC_TRUE;//received the data
						
							op_ev_cancel(wait_DATA_event);
							op_pk_nfd_get(pk_TV,"Previous Hop",&next_hop_1);
							
							op_pk_nfd_get(pk_TV,"Hop Num", &hop_num_TV);
							hop_num_TV++;
							op_pk_nfd_set(pk_TV,"Hop Num",hop_num_TV);
							
							//delay statistics
							hops_pkt_num++;
							hops_avg_delay=hops_avg_delay+op_sim_time()-op_pk_creation_time_get(pk_TV);
							
							/****************************************************************/
							/*create ICI for statistic purpose; then set remote interruption*/
							/*for average delay per packet vs. hops							*/
							/****************************************************************/
							if(mac_flags->stat_flag){
								op_pk_nfd_get(pk_TV,"Grade",&src_grade);
								iciptr_TV = op_ici_create("delay_ici");
								op_ici_attr_set_int32(iciptr_TV, "src_grade",src_grade);
								op_ici_attr_set_int32(iciptr_TV, "current_node_grade",grade);
								op_ici_attr_set_dbl(iciptr_TV, "ete_delay",op_sim_time()-op_pk_creation_time_get(pk_TV));
								//if (node_type==sink){
								//sprintf(message,"1. the ete delay is %f.",op_sim_time()-op_pk_creation_time_get(pk_TV));
								//op_sim_message(message,"");}
								op_ici_install (iciptr_TV);
								op_intrpt_force_remote (REMOTE_STAT_CODE,op_id_from_name(node_id,OPC_OBJTYPE_PROC,"statistics"));
								op_ici_install (OPC_NIL);
							}
							
							if(node_type == sink)
							{
								op_pk_nfd_get(pk_TV,"Data No",&data_id);
								//op_sim_message("received data!!!","");
								if(!data_id_exist(data_id)){
									id_TV=(DataID *)op_prg_mem_alloc(sizeof(DataID));
									id_TV->st_data_id=data_id;
									op_prg_list_insert(data_id_list,id_TV,OPC_LISTPOS_TAIL);
									data_nums_1++;
								}
								
								if(mac_flags->stat_flag){
									recvd_pkt_num_at_sink++;
								}
								op_pk_send(pk_TV,DISCARD_STRM);
								if(DEBUG)
									printf("Data has arrived at sink!!!\n");
							}else
							{
								if(op_subq_stat(data_queue,OPC_QSTAT_PKSIZE)<queue_length_add_one){//data queue is not full
									//insert the DATA packet into data_queue
									op_pk_nfd_set(pk_TV,"Receiving Time",op_sim_time());
									op_subq_pk_insert (data_queue,pk_TV,OPC_QPOS_TAIL);
								}else{//data queue is full
									pkts_loss_overflow = pkts_loss_overflow + 1.0;
									if(DEBUG)
										printf("in state \"RCV(1)\", data queue is full, the received data pkt from the lower layer is dropped.\n");
								}
							}
						
							//wait SIFS time to send ACK
							time = SIFS;
							op_intrpt_schedule_self(op_sim_time()+time,SEND_ACK_CODE);
							if(DEBUG)
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
					if(DEBUG)
						printf("Node:%d, Grade:%d, time:%f, in first period.\n\
							The received pk is not RTS, CTS, or DATA. The pk type is: %d.\n\
							Destroy this pk.\n",\
							node_address,grade,op_sim_time(),pk_type);
						
					op_pk_destroy(pk_TV);
				}	
				}
				FSM_PROFILE_SECTION_OUT (state12_enter_exec)

			/** state (RCV(1)) exit executives **/
			FSM_STATE_EXIT_FORCED (12, "RCV(1)", "pri_proc_lwsn [RCV(1) exit execs]")


			/** state (RCV(1)) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "RCV(1)", "R:first", "tr_75", "pri_proc_lwsn [RCV(1) -> R:first : default / ]")
				/*---------------------------------------------------------*/



			/** state (wait DATA) enter executives **/
			FSM_STATE_ENTER_FORCED (13, "wait DATA", state13_enter_exec, "pri_proc_lwsn [wait DATA enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [wait DATA enter execs]", state13_enter_exec)
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
				//op_pk_nfd_set(pk_TV,"Start Slot",SLOT_NO_ASSIGN);
				//op_pk_nfd_set(pk_TV,"Slot Num",SLOT_NO_ASSIGN);
				
				
				op_pk_send(pk_TV, SEND_STRM);
				
				//mac_flags->received_data = OPC_FALSE;
				
				//time statistic
				if(mac_flags->stat_flag) {
					all_tx_time = all_tx_time + durCTS;
				}
				
				//set timer for waiting DATA from upper grade node
				time = durCTS + SIFS +durDATA  + CUSHION_TIME;//多加一个时间，否则节点收不到包
				wait_DATA_event = op_intrpt_schedule_self(op_sim_time() + time,SLEEP_FROM_FIRST_CODE);
				if(DEBUG)
					printf("Node:%d, Grade:%d, time:%f, in first period.\n\
						Have sent CTS.\n\
						Wating to receive Data.\n\
						If no data, after %f secondes, go to sleep.\n",\
						node_address,grade,op_sim_time(),time);
				}
				FSM_PROFILE_SECTION_OUT (state13_enter_exec)

			/** state (wait DATA) exit executives **/
			FSM_STATE_EXIT_FORCED (13, "wait DATA", "pri_proc_lwsn [wait DATA exit execs]")


			/** state (wait DATA) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "wait DATA", "R:first", "tr_77", "pri_proc_lwsn [wait DATA -> R:first : default / ]")
				/*---------------------------------------------------------*/



			/** state (send ACK) enter executives **/
			FSM_STATE_ENTER_FORCED (14, "send ACK", state14_enter_exec, "pri_proc_lwsn [send ACK enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [send ACK enter execs]", state14_enter_exec)
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
				if(mac_flags->stat_flag) {
					all_tx_time = all_tx_time + durACK;
				}
				
				op_intrpt_schedule_self(op_sim_time() + durACK,SLEEP_FROM_FIRST_CODE);
				if(DEBUG)
					printf("Node:%d, Grade:%d, time:%f, in first period.\n\
						Have replied ACK.\n\
						Go to sleep after finishing sending ACK.\n",\
						node_address,grade,op_sim_time());
				}
				FSM_PROFILE_SECTION_OUT (state14_enter_exec)

			/** state (send ACK) exit executives **/
			FSM_STATE_EXIT_FORCED (14, "send ACK", "pri_proc_lwsn [send ACK exit execs]")


			/** state (send ACK) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "send ACK", "R:first", "tr_80", "pri_proc_lwsn [send ACK -> R:first : default / ]")
				/*---------------------------------------------------------*/



			/** state (carrier sense(1)) enter executives **/
			FSM_STATE_ENTER_FORCED (15, "carrier sense(1)", state15_enter_exec, "pri_proc_lwsn [carrier sense(1) enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [carrier sense(1) enter execs]", state15_enter_exec)
				{
				if(DEBUG)
				{
					printf("Node:%d, time:%f. In first period.\n进入 statwire 中断\n",node_address,op_sim_time());
					printf("up: the received snr=%E.intrpt_code=%d\n\n",op_stat_local_read(0),op_intrpt_code());
				}
				if(op_ev_valid(decrease_CW_event) && (one_two_cw == 2))
				{
					//the channel is sensed busy before replying CTS.
					mac_flags->busy_before_cts=OPC_TRUE;
					//op_ev_cancel(send_CTS_event);
					//op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_FIRST_CODE);
					if(DEBUG)
						printf("Channel is busy, go to sleep after the current slot.\n");
					
				}		
				
				}
				FSM_PROFILE_SECTION_OUT (state15_enter_exec)

			/** state (carrier sense(1)) exit executives **/
			FSM_STATE_EXIT_FORCED (15, "carrier sense(1)", "pri_proc_lwsn [carrier sense(1) exit execs]")


			/** state (carrier sense(1)) transition processing **/
			FSM_TRANSIT_FORCE (8, state8_enter_exec, ;, "default", "", "carrier sense(1)", "R:first", "tr_94", "pri_proc_lwsn [carrier sense(1) -> R:first : default / ]")
				/*---------------------------------------------------------*/



			/** state (carrier sense(2)) enter executives **/
			FSM_STATE_ENTER_FORCED (16, "carrier sense(2)", state16_enter_exec, "pri_proc_lwsn [carrier sense(2) enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [carrier sense(2) enter execs]", state16_enter_exec)
				{
				if(DEBUG)
					printf("Node:%d, time:%f. In second period.\n",node_address,op_sim_time());
				if(DEBUG)
				{
					printf("进入 statwire 中断\n");
					printf("down: the received snr=%E.intrpt_code=%d\n\n",op_stat_local_read(0),op_intrpt_code());
				}
				
				
				if(op_ev_valid(decrease_CW_event))
				{
					//op_ev_cancel(decrease_CW_event);
					mac_flags->busy_before_rts=OPC_TRUE;
					
					//op_intrpt_schedule_self(op_sim_time(),SLEEP_FROM_SECOND_CODE);
					if(DEBUG)
						printf("Channel is busy, go to sleep after the current slot.\n");
				}
				}
				FSM_PROFILE_SECTION_OUT (state16_enter_exec)

			/** state (carrier sense(2)) exit executives **/
			FSM_STATE_EXIT_FORCED (16, "carrier sense(2)", "pri_proc_lwsn [carrier sense(2) exit execs]")


			/** state (carrier sense(2)) transition processing **/
			FSM_TRANSIT_FORCE (9, state9_enter_exec, ;, "default", "", "carrier sense(2)", "T:second", "tr_96", "pri_proc_lwsn [carrier sense(2) -> T:second : default / ]")
				/*---------------------------------------------------------*/



			/** state (start grade) enter executives **/
			FSM_STATE_ENTER_FORCED (17, "start grade", state17_enter_exec, "pri_proc_lwsn [start grade enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_proc_lwsn [start grade enter execs]", state17_enter_exec)
				{
				//Sink will execute the following code
				grade = 0;
				//op_ima_obj_attr_set(process_id,"grade",grade);
				op_ima_obj_attr_set(node_id,"grade",grade);
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
			FSM_STATE_EXIT_FORCED (17, "start grade", "pri_proc_lwsn [start grade exit execs]")


			/** state (start grade) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "start grade", "idle", "tr_98", "pri_proc_lwsn [start grade -> idle : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"pri_proc_lwsn")
		}
	}




void
_op_pri_proc_lwsn_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_pri_proc_lwsn_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_pri_proc_lwsn_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_pri_proc_lwsn_svar function. */
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
#undef second_contending_time
#undef wait_CTS_event
#undef wait_ACK_event
#undef next_hop_1
#undef next_hop_2
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
#undef mac_flags
#undef CW_counter
#undef slot_time
#undef decrease_CW_event
#undef queue_stat_list
#undef QStatus_handle
#undef hops_avg_delay
#undef hops_pkt_num
#undef Hops_avg_delay_handle
#undef queue_length_add_one
#undef lost_pkt_col_handle
#undef lost_pkt_of_handle
#undef avg_energy_handle
#undef CW_grade
#undef slot_time_grade
#undef ddc_event_handle
#undef lht_event_handle
#undef send_prob_handle
#undef win_success
#undef total_send_trial
#undef steady_time
#undef regular_wakeup_evhandle
#undef remainder_retran_num
#undef receive_prob_handle
#undef receive_success
#undef total_receive_trial
#undef send_success
#undef avg_duty_cycle
#undef avg_queueing_delay_handle
#undef avg_queueing_delay
#undef sum_src_delay
#undef src_pkt_num
#undef sum_relay_delay
#undef relay_pkt_num
#undef stat_stop_time
#undef throughput2_handle
#undef recvd_pkt_num_at_sink
#undef duty_cycle_num_at_sink
#undef chain_number
#undef line_hops
#undef win_prob_handle
#undef sense_busy
#undef busy_prob_handle
#undef prob_dc_handle
#undef prob_delay_to_sink_handle
#undef prob_throughput_handle

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_pri_proc_lwsn_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_pri_proc_lwsn_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (pri_proc_lwsn)",
		sizeof (pri_proc_lwsn_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_pri_proc_lwsn_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	pri_proc_lwsn_state * ptr;
	FIN_MT (_op_pri_proc_lwsn_alloc (obtype))

	ptr = (pri_proc_lwsn_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "pri_proc_lwsn [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_pri_proc_lwsn_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	pri_proc_lwsn_state		*prs_ptr;

	FIN_MT (_op_pri_proc_lwsn_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (pri_proc_lwsn_state *)gen_ptr;

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
	if (strcmp ("mac_flags" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->mac_flags);
		FOUT
		}
	if (strcmp ("CW_counter" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->CW_counter);
		FOUT
		}
	if (strcmp ("slot_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->slot_time);
		FOUT
		}
	if (strcmp ("decrease_CW_event" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->decrease_CW_event);
		FOUT
		}
	if (strcmp ("queue_stat_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->queue_stat_list);
		FOUT
		}
	if (strcmp ("QStatus_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->QStatus_handle);
		FOUT
		}
	if (strcmp ("hops_avg_delay" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->hops_avg_delay);
		FOUT
		}
	if (strcmp ("hops_pkt_num" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->hops_pkt_num);
		FOUT
		}
	if (strcmp ("Hops_avg_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->Hops_avg_delay_handle);
		FOUT
		}
	if (strcmp ("queue_length_add_one" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->queue_length_add_one);
		FOUT
		}
	if (strcmp ("lost_pkt_col_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->lost_pkt_col_handle);
		FOUT
		}
	if (strcmp ("lost_pkt_of_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->lost_pkt_of_handle);
		FOUT
		}
	if (strcmp ("avg_energy_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_energy_handle);
		FOUT
		}
	if (strcmp ("CW_grade" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->CW_grade);
		FOUT
		}
	if (strcmp ("slot_time_grade" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->slot_time_grade);
		FOUT
		}
	if (strcmp ("ddc_event_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->ddc_event_handle);
		FOUT
		}
	if (strcmp ("lht_event_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->lht_event_handle);
		FOUT
		}
	if (strcmp ("send_prob_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_prob_handle);
		FOUT
		}
	if (strcmp ("win_success" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->win_success);
		FOUT
		}
	if (strcmp ("total_send_trial" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->total_send_trial);
		FOUT
		}
	if (strcmp ("steady_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->steady_time);
		FOUT
		}
	if (strcmp ("regular_wakeup_evhandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->regular_wakeup_evhandle);
		FOUT
		}
	if (strcmp ("remainder_retran_num" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->remainder_retran_num);
		FOUT
		}
	if (strcmp ("receive_prob_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->receive_prob_handle);
		FOUT
		}
	if (strcmp ("receive_success" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->receive_success);
		FOUT
		}
	if (strcmp ("total_receive_trial" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->total_receive_trial);
		FOUT
		}
	if (strcmp ("send_success" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->send_success);
		FOUT
		}
	if (strcmp ("avg_duty_cycle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_duty_cycle);
		FOUT
		}
	if (strcmp ("avg_queueing_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_queueing_delay_handle);
		FOUT
		}
	if (strcmp ("avg_queueing_delay" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_queueing_delay);
		FOUT
		}
	if (strcmp ("sum_src_delay" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->sum_src_delay);
		FOUT
		}
	if (strcmp ("src_pkt_num" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->src_pkt_num);
		FOUT
		}
	if (strcmp ("sum_relay_delay" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->sum_relay_delay);
		FOUT
		}
	if (strcmp ("relay_pkt_num" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->relay_pkt_num);
		FOUT
		}
	if (strcmp ("stat_stop_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->stat_stop_time);
		FOUT
		}
	if (strcmp ("throughput2_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->throughput2_handle);
		FOUT
		}
	if (strcmp ("recvd_pkt_num_at_sink" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->recvd_pkt_num_at_sink);
		FOUT
		}
	if (strcmp ("duty_cycle_num_at_sink" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->duty_cycle_num_at_sink);
		FOUT
		}
	if (strcmp ("chain_number" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->chain_number);
		FOUT
		}
	if (strcmp ("line_hops" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->line_hops);
		FOUT
		}
	if (strcmp ("win_prob_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->win_prob_handle);
		FOUT
		}
	if (strcmp ("sense_busy" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->sense_busy);
		FOUT
		}
	if (strcmp ("busy_prob_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->busy_prob_handle);
		FOUT
		}
	if (strcmp ("prob_dc_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->prob_dc_handle);
		FOUT
		}
	if (strcmp ("prob_delay_to_sink_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->prob_delay_to_sink_handle);
		FOUT
		}
	if (strcmp ("prob_throughput_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->prob_throughput_handle);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

