/* Process model C form file: pri_stat_proc_lwsn.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char pri_stat_proc_lwsn_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 535EE525 535EE525 1 wpd FeiTong 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                              ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

#include <math.h>
//Define node type
#define sink 	1
#define sensor 	2

#define END	        		    	(op_intrpt_type() == OPC_INTRPT_ENDSIM)

//Define remote-interrupt code and transition condition
#define REMOTE_STAT_CODE				500	/*remote statistics*/


#define REMOTE_STAT ((op_intrpt_type() == OPC_INTRPT_REMOTE) && (op_intrpt_code() == REMOTE_STAT_CODE))

//For energy consumption
double sensors_num=0.0;/* the number of sensors in the network. Its initial value is 0.0; */
				/* Every time it increases by one when it receives energy_dc_ici.  */
double average_duty_cycle=0.0;
double average_energy_consumption = 0.0; /*average energy consumption per sensor node*/
double network_running_time;
//For delivery ratio
int all_gent_pkt_num=0; //the totoal number of packets generated in the network

//For random topology: statistic at the sink node with "average packet delay vs. src_grade"
typedef struct Src_ETE_Delay
{
	int pkt_num;
	double delay_sum;
	int hops;
}Src_ETE_Delay;

//in the cahin topology, to calculate the averty duty cycle for each hop
typedef struct Hop_Duty_Cycle
{
	int hop;
	int node_num;
	double sum_duty_cycle;
}Hop_Duty_Cycle;

//src/relay delay
typedef struct Src_Relay_Delay
{
	int hop;
	double sum_src_delay;
	double sum_relay_delay;
	double src_pkt_num;
	double relay_pkt_num;
}Src_Relay_Delay;

List* grade_dist_list;
List* node_child_list;
List* line_delay_list;
List* duty_cycle_list;
List* src_relay_delay_list;

//test:
/*List* hop_pkt_rcved_num_list;
typedef struct
{
	double pk_num;//packet number from a specific grade (indicated by list index)
	double delay;//packet delay from this grade
}GradeDelay;
*/
//List* hop_src_pkt_num_list; //to record the num of pkts generating at each source

//functions
static void deal_with_ici(void);
static void func_sim_end(void);

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
	Objid	                  		node_id                                         ;
	Objid	                  		process_id                                      ;
	int	                    		node_type                                       ;
	int	                    		node_gent_pkt_num                               ;	/* the number of packets generated in each node */
	int	                    		dest_rcved_pkt_num                              ;	/* the number of pkts received by all sink(s) */
	Stathandle	             		delay_in_random_handle                          ;
	Stathandle	             		grade_dist_handle                               ;
	Stathandle	             		node_child_handle                               ;
	List*	                  		src_to_sink_delay_list                          ;
	Stathandle	             		line_delay_handle                               ;
	Stathandle	             		pkt_delivery_ratio_handle                       ;
	Stathandle	             		ec_per_rcvd_pkt_handle                          ;
	Stathandle	             		throughput_handle                               ;
	int	                    		dest_rcved_pkt_num_throughput                   ;
	double	                 		steady_time                                     ;
	Objid	                  		Dep_Admin_id                                    ;	/* the objid of "Deploygments_Admin" */
	int	                    		chain_number                                    ;	/* the number of chains */
	Stathandle	             		avg_hop_duty_cycle_handle                       ;
	Stathandle	             		avg_src_delay_handle                            ;
	Stathandle	             		avg_relay_delay_handle                          ;
	Stathandle	             		avg_hop_delay_handle                            ;
	int	                    		line_hops                                       ;
	int	                    		debug                                           ;
	} pri_stat_proc_lwsn_state;

#define node_id                 		op_sv_ptr->node_id
#define process_id              		op_sv_ptr->process_id
#define node_type               		op_sv_ptr->node_type
#define node_gent_pkt_num       		op_sv_ptr->node_gent_pkt_num
#define dest_rcved_pkt_num      		op_sv_ptr->dest_rcved_pkt_num
#define delay_in_random_handle  		op_sv_ptr->delay_in_random_handle
#define grade_dist_handle       		op_sv_ptr->grade_dist_handle
#define node_child_handle       		op_sv_ptr->node_child_handle
#define src_to_sink_delay_list  		op_sv_ptr->src_to_sink_delay_list
#define line_delay_handle       		op_sv_ptr->line_delay_handle
#define pkt_delivery_ratio_handle		op_sv_ptr->pkt_delivery_ratio_handle
#define ec_per_rcvd_pkt_handle  		op_sv_ptr->ec_per_rcvd_pkt_handle
#define throughput_handle       		op_sv_ptr->throughput_handle
#define dest_rcved_pkt_num_throughput		op_sv_ptr->dest_rcved_pkt_num_throughput
#define steady_time             		op_sv_ptr->steady_time
#define Dep_Admin_id            		op_sv_ptr->Dep_Admin_id
#define chain_number            		op_sv_ptr->chain_number
#define avg_hop_duty_cycle_handle		op_sv_ptr->avg_hop_duty_cycle_handle
#define avg_src_delay_handle    		op_sv_ptr->avg_src_delay_handle
#define avg_relay_delay_handle  		op_sv_ptr->avg_relay_delay_handle
#define avg_hop_delay_handle    		op_sv_ptr->avg_hop_delay_handle
#define line_hops               		op_sv_ptr->line_hops
#define debug                   		op_sv_ptr->debug

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	pri_stat_proc_lwsn_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((pri_stat_proc_lwsn_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

static void deal_with_ici(void)
/*
* after receiving remote interruption, deal with the bounded ici.
*/
{
//var
	char ici_format_name[128];
	//for remote statistics
	Ici  *iciptr;
	int  src_grade,current_node_grade,current_node_address,current_node_child_nums;
	int  *nodes_num_in_grade, *node_child_nums;
	int list_size,list_i;
	int hops;
	int node_address;
	Src_ETE_Delay* list_element;
	Hop_Duty_Cycle* dc_list_element; //duty_cycle_list element
	Src_Relay_Delay* sr_delay_element; //source/relay delay element
	double ete_delay;
	double duty_cycle, energy_consumption;
	
	double sum_src_delay, sum_relay_delay, src_pkt_num, relay_pkt_num;
	char message[512];
	
	//int * pkt_num;
	//test:
//	GradeDelay*	gd_TV;
	
//in
	FIN(deal_with_ici(void));
//body
	/* Get the ICI associated with the new arrival. */
	iciptr = op_intrpt_ici (); 
	op_ici_format(iciptr,ici_format_name);
	//printf("远程中断生效!!!\n");
	
	if(strcmp(ici_format_name,"energy_dc_ici") == 0){
	/* from function "power_stat" in module "pmac"*/
	
		/*the received ici is "energy_dc_ici"*/
	
		op_ici_attr_get_dbl(iciptr,"duty_cycle",&duty_cycle);
		op_ici_attr_get_dbl(iciptr,"energy_consumption",&energy_consumption);
		op_ici_attr_get_dbl(iciptr,"net_running_time",&network_running_time);
		op_ici_attr_get_int32(iciptr,"node_address",&node_address);
		sensors_num=sensors_num+1.0;
		average_duty_cycle = average_duty_cycle + duty_cycle;
		average_energy_consumption += energy_consumption;
		//printf("received the arrived ici!, duty_cycle=%f,sensors_num=%f\n\n",	duty_cycle,sensors_num);
		
		//average duty_cycle for each hop
		hops=ceil((double)node_address/(double)chain_number);
		list_size = op_prg_list_size (duty_cycle_list);
		if(hops<=list_size){
			dc_list_element = (Hop_Duty_Cycle *)op_prg_list_access (duty_cycle_list, hops-1);
			dc_list_element->node_num = dc_list_element->node_num + 1;
			dc_list_element->sum_duty_cycle = dc_list_element->sum_duty_cycle + duty_cycle;
		}else{
			dc_list_element = (Hop_Duty_Cycle *)op_prg_mem_alloc(sizeof(struct Hop_Duty_Cycle));
			dc_list_element->hop = hops;
			dc_list_element->node_num = 1;
			dc_list_element->sum_duty_cycle = duty_cycle;
			op_prg_list_insert(duty_cycle_list,dc_list_element,hops-1);
		}
		
	
	}else if(strcmp(ici_format_name,"src_relay_delay_ici") == 0){
	//source/relay delay
		op_ici_attr_get_int32(iciptr,"node_address",&node_address);
		op_ici_attr_get_dbl(iciptr,"sum_src_delay",&sum_src_delay);
		op_ici_attr_get_dbl(iciptr,"src_pkt_num",&src_pkt_num);
		op_ici_attr_get_dbl(iciptr,"sum_relay_delay",&sum_relay_delay);
		op_ici_attr_get_dbl(iciptr,"relay_pkt_num",&relay_pkt_num);
		
		hops=ceil((double)node_address/(double)chain_number);
		list_size = op_prg_list_size (src_relay_delay_list);
		if(hops<=list_size){
			sr_delay_element = (Src_Relay_Delay *)op_prg_list_access (src_relay_delay_list, hops-1);
			
			sr_delay_element->sum_src_delay += sum_src_delay;
			sr_delay_element->sum_relay_delay += sum_relay_delay;
			sr_delay_element->src_pkt_num += src_pkt_num;
			sr_delay_element->relay_pkt_num += relay_pkt_num;
		}else{
			sr_delay_element = (Src_Relay_Delay *)op_prg_mem_alloc(sizeof(struct Src_Relay_Delay));
			sr_delay_element->hop = hops;
			sr_delay_element->sum_src_delay = sum_src_delay;
			sr_delay_element->sum_relay_delay = sum_relay_delay;
			sr_delay_element->src_pkt_num = src_pkt_num;
			sr_delay_element->relay_pkt_num = relay_pkt_num;
			op_prg_list_insert(src_relay_delay_list,sr_delay_element,hops-1);			
		}
	
		
		
		
		
	}else if(strcmp(ici_format_name,"src_pkt_ici") == 0){
	/* from state "end" in module "gsrc" */
		op_ici_attr_get_int32(iciptr,"sent pkt num",&node_gent_pkt_num);
		op_ici_attr_get_int32(iciptr,"src grade",&src_grade);
		//需要一个list来记录每个grade的src产生的包的个数
		//code to be appeared
		//pkt_num = (int *)op_prg_list_access(hop_src_pkt_num_list,src_grade-1);
		//*pkt_num =  node_gent_pkt_num;
				
		all_gent_pkt_num = all_gent_pkt_num + node_gent_pkt_num;
	
	}else if(strcmp(ici_format_name,"dest_rcved_pkt_ici") == 0){
	/* from state "end" in module "collect data", only from the sink node */
		op_ici_attr_get_int32(iciptr,"dest rcved pkt",&dest_rcved_pkt_num);
		op_ici_attr_get_int32(iciptr,"dest rcved pkt throughput",&dest_rcved_pkt_num_throughput);
		
		//test:
	/*	op_ici_attr_get_ptr(iciptr, "hop dest rcved pkt",&hop_pkt_rcved_num_list);
		for(list_i=0;list_i<line_hops;list_i++)
		{
			//if(debug == 5) {printf("it's here.\n");}
			gd_TV=(GradeDelay*)op_prg_list_access(hop_pkt_rcved_num_list,list_i);
			if(debug == 4){printf("grade:%d,rcved pkt num:%f.\n",list_i, gd_TV->pk_num);}
		}
		
	*/	
		
		
		
	}else if(strcmp(ici_format_name,"delay_ici") == 0){
	/* from state "RCV(1)" in module "pmac" */

		op_ici_attr_get_int32(iciptr,"src_grade",&src_grade);
		op_ici_attr_get_dbl(iciptr,"ete_delay",&ete_delay);
		op_ici_attr_get_int32(iciptr,"current_node_grade",&current_node_grade);
		
		//1. for delay in line topology
		hops = src_grade - current_node_grade;
		list_size = op_prg_list_size (line_delay_list);
		if(list_size < hops){//the accessed element doesn't exist
			for(list_i = list_size; list_i < hops; list_i++){
				list_element = (Src_ETE_Delay *)op_prg_mem_alloc(sizeof(struct Src_ETE_Delay));
				list_element->pkt_num = 0;
				list_element->delay_sum = 0.0;
				list_element->hops = list_i+1;
				op_prg_list_insert(line_delay_list,list_element,list_i);
			}
		}
		//update the last element of the list
		list_element = (Src_ETE_Delay*)op_prg_list_access (line_delay_list, hops-1);
		list_element->pkt_num +=  1;
		list_element->delay_sum += ete_delay;
		if(list_element->hops != hops){
			sprintf(message,"hops=%d, src_grade=%d, current_node_grade=%d.",list_element->hops, src_grade,current_node_grade);
			op_sim_end("1. Error: In statistics->pri_stat_proc.","packet hops are wrong!",message,"");
		}
		
		//2. for delay in random topology
		if(node_type==sink){//calculate the average packet delay from each source to the sink node.
			
			list_size = op_prg_list_size (src_to_sink_delay_list);
			if(list_size < src_grade){//indicates that no element in the list for src_grade
				for(list_i = list_size; list_i < src_grade; list_i++){
				// create the elements until src_grade is contained in the list
					//printf("创建元素！\n");
					list_element = (Src_ETE_Delay *)op_prg_mem_alloc(sizeof(struct Src_ETE_Delay));
					list_element->pkt_num = 0;
					list_element->delay_sum = 0.0;
					list_element->hops = list_i+1 - current_node_grade;
					if(current_node_grade!=0)
						op_sim_end("error, current_node_grade!=0","","","");
					op_prg_list_insert(src_to_sink_delay_list,list_element,list_i);
				}
			}
			//update the last element of the list
			list_element = (Src_ETE_Delay*)op_prg_list_access (src_to_sink_delay_list, src_grade-1);
			list_element->pkt_num +=  1;
			list_element->delay_sum += ete_delay;
			if(list_element->hops != src_grade - current_node_grade){
				sprintf(message,"hops=%d, src_grade=%d, current_node_grade=%d.",list_element->hops, src_grade,current_node_grade);
				op_sim_end("2. Error: In statistics->pri_stat_proc.","packet hops are wrong!",message,"");
			}
		}
		
	}else if(strcmp(ici_format_name,"grade_ici") == 0){
	/* from function "record_grade" in module "pmac" */
	
		op_ici_attr_get_int32(iciptr,"node_grade",&current_node_grade);
		op_ici_attr_get_int32(iciptr,"node_address",&current_node_address);
		op_ici_attr_get_int32(iciptr,"child_nums",&current_node_child_nums);
		
		//1. stat: grade distribution in the network
		list_size = op_prg_list_size (grade_dist_list);
		if(list_size < current_node_grade){ //no element for this grade (node_grade)
			for(list_i = list_size; list_i < current_node_grade; list_i++){
				nodes_num_in_grade = (int*)op_prg_mem_alloc(sizeof(int));
				*nodes_num_in_grade = 0;
				op_prg_list_insert(grade_dist_list,nodes_num_in_grade,list_i);
			}
		}
		//update the last element of the list
		nodes_num_in_grade = (int*)op_prg_list_access (grade_dist_list,current_node_grade-1);
		*nodes_num_in_grade = *nodes_num_in_grade + 1;
		
		//2. stat: node child
		list_size = op_prg_list_size (node_child_list);
		if(list_size < current_node_address){
			for(list_i = list_size; list_i < current_node_address; list_i++){
				node_child_nums = (int*)op_prg_mem_alloc(sizeof(int));
				*node_child_nums = 0;
				op_prg_list_insert(node_child_list,node_child_nums,list_i);
			}
		}
		//update the last element of the list
		node_child_nums = (int*)op_prg_list_access (node_child_list,current_node_address-1);
		*node_child_nums = current_node_child_nums;
	}

	op_ici_destroy(iciptr);
//out
	FOUT;
}


static void func_sim_end(void)
/*
* At the end of the simulation, write statistics.
*/
{
//var
	int list_size, list_i,list_j, hop;
	Src_ETE_Delay* struct_element;
	Hop_Duty_Cycle* dc_list_element;
	Src_Relay_Delay* sr_delay_element; //source/relay delay element
	double network_energy_consumption;
	double sum_hop_delay;
	int* int_element;
	char message[512];
	
	//int * hop_src_pkt_num;
//	int * hop_dest_rcved_pkt_num;
//in
	FIN(func_sim_end(void));
//body
	if(node_type==sink){
		network_energy_consumption = average_energy_consumption;
		op_stat_write_t(ec_per_rcvd_pkt_handle,network_energy_consumption/((double)(dest_rcved_pkt_num)*network_running_time),0.0);
	
		/*
		* *. average duty cycle per node
		*/
		//printf("dest_rcved_pkt_num=%d, network_running_time=%f.\n",dest_rcved_pkt_num,network_running_time);
		if(sensors_num!=0.0){
			average_duty_cycle /= sensors_num;
			average_energy_consumption /= sensors_num;
			sprintf(message,"仿真结束!\nThe average duty cycle per node is: %f.\nThe average energy consumption per node per second is %f/%f = %f",\
				average_duty_cycle, average_energy_consumption,network_running_time,average_energy_consumption/network_running_time);
			op_sim_message(message,"");
		}
		
		list_size = op_prg_list_size (duty_cycle_list);
		for(list_i=0;list_i<list_size;list_i++){
			dc_list_element = (Hop_Duty_Cycle *)op_prg_list_access (duty_cycle_list, list_i);
			op_stat_write_t(avg_hop_duty_cycle_handle,dc_list_element->sum_duty_cycle/dc_list_element->node_num,dc_list_element->hop);
		}
		
		//source/relay delay
		list_size = op_prg_list_size (src_relay_delay_list);
		for(list_i=0;list_i<list_size;list_i++){
			sr_delay_element = (Src_Relay_Delay *)op_prg_list_access (src_relay_delay_list, list_i);
			op_stat_write_t(avg_src_delay_handle,sr_delay_element->sum_src_delay/sr_delay_element->src_pkt_num,sr_delay_element->hop);
			op_stat_write_t(avg_relay_delay_handle,sr_delay_element->sum_relay_delay/sr_delay_element->relay_pkt_num,sr_delay_element->hop);
			hop = sr_delay_element->hop;
			
			sum_hop_delay=sr_delay_element->sum_src_delay/sr_delay_element->src_pkt_num;
			for(list_j=list_i-1;list_j>=0;list_j--){
				sr_delay_element = (Src_Relay_Delay *)op_prg_list_access (src_relay_delay_list, list_j);
				sum_hop_delay += sr_delay_element->sum_relay_delay/sr_delay_element->relay_pkt_num;
			}
			op_stat_write_t(avg_hop_delay_handle,sum_hop_delay,hop);
		}
		
		/*
		* *. Packet delivery ratio
		*/
		sprintf(message,"the toatal number of packets received/The total number of packets generated: %d/%d = %f.\n\n",\
			dest_rcved_pkt_num,all_gent_pkt_num, (double)(dest_rcved_pkt_num)/(double)(all_gent_pkt_num));
		op_sim_message(message,"");
		op_stat_write_t(pkt_delivery_ratio_handle,(double)(dest_rcved_pkt_num)/(double)(all_gent_pkt_num),500.0);
		//op_stat_write_t(throughput_handle,(double)(dest_rcved_pkt_num)/network_running_time,520.0);
		op_stat_write_t(throughput_handle,(double)(dest_rcved_pkt_num_throughput)/(op_sim_time()-steady_time),520.0);
		
		
		
		
		
		
		/*
		* *. Packet delivery ratio for each hop
		*/
//		list_size = op_prg_list_size((List*)hop_pkt_rcved_num_list);
//		for(list_i=0;list_i<list_size;list_i++)
//		{
//			hop_dest_rcved_pkt_num=(int*)op_prg_list_access(hop_pkt_rcved_num_list,list_i);
//			if(debug == 4)
//				printf("read list:list_size = %d, rcved pkt num = %d.\n",list_size,*hop_dest_rcved_pkt_num );
			
			//hop_src_pkt_num=(int*)op_prg_list_access(hop_src_pkt_num_list,list_i);
			//hop_dest_rcved_pkt_num=(int*)op_prg_list_access(hop_pkt_rcved_num_list,list_i);
			//if(*hop_src_pkt_num != 0)
			//{
			//	op_stat_write_t(avg_hop_pkt_delivery_ratio_handle,(double)(*hop_dest_rcved_pkt_num)/(double)(*hop_src_pkt_num),list_i+1.0);
			//}
			
//		}
		
		
		
		
		
		
		
		/* 
		* * 1. for delay in line topology
		*/
		list_size = op_prg_list_size(line_delay_list);
		for(list_i=0;list_i<list_size;list_i++){
			struct_element = (Src_ETE_Delay*)op_prg_list_access (line_delay_list, list_i);
			op_stat_write_t(line_delay_handle,struct_element->delay_sum/struct_element->pkt_num,list_i+1);
			//sprintf(message,"delay_sum=%f,pkt_num=%d.\n",struct_element->delay_sum,struct_element->pkt_num);
			//op_sim_message(message,"");
		}
		
		/*
		* 2. for delay in random topology
		*/
		list_size = op_prg_list_size(src_to_sink_delay_list);
		for(list_i=0;list_i<list_size;list_i++){
			struct_element = (Src_ETE_Delay*)op_prg_list_access (src_to_sink_delay_list, list_i);
			op_stat_write_t(delay_in_random_handle,struct_element->delay_sum/struct_element->pkt_num,list_i+1);
			//sprintf(message,"delay_sum=%f,pkt_num=%d.\n",struct_element->delay_sum,struct_element->pkt_num);
			//op_sim_message(message,"");
		}
		
		//grade distribution in the network
		list_size = op_prg_list_size(grade_dist_list);
		for(list_i=0;list_i<list_size;list_i++){
			int_element = (int*)op_prg_list_access (grade_dist_list, list_i);
			op_stat_write_t(grade_dist_handle,*int_element,list_i+1);
		}
		
		//node children
		list_size = op_prg_list_size (node_child_list);
		for(list_i=0;list_i<list_size;list_i++){
			int_element = (int*)op_prg_list_access (node_child_list, list_i);
			op_stat_write_t(node_child_handle,*int_element,list_i+1);
		}
	}

	op_prg_list_free (src_to_sink_delay_list);
	op_prg_mem_free (src_to_sink_delay_list);
	
	if(node_type == sink){
		op_prg_list_free (line_delay_list); //since line_delay_list is a global variable, it can only be freed one time
		op_prg_mem_free (line_delay_list);
		
		op_prg_list_free (grade_dist_list); //since grade_dist_list is a global variable, it can only be freed one time
		op_prg_mem_free (grade_dist_list);
		
		op_prg_list_free (node_child_list); //since node_child_list is a global variable, it can only be freed one time
		op_prg_mem_free (node_child_list);
		
		op_prg_list_free (duty_cycle_list); //since duty_cycle_list is a global variable, it can only be freed one time
		op_prg_mem_free (duty_cycle_list);
		
		op_prg_list_free (src_relay_delay_list); //since src_relay_delay_list is a global variable, it can only be freed one time
		op_prg_mem_free (src_relay_delay_list);
/*		
		op_prg_list_free (hop_pkt_rcved_num_list); //since hop_pkt_rcved_num_list is a global variable, it can only be freed one time
		op_prg_mem_free (hop_pkt_rcved_num_list);
		
		op_prg_list_free (hop_src_pkt_num_list); //since hop_src_pkt_num_list is a global variable, it can only be freed one time
		op_prg_mem_free (hop_src_pkt_num_list);
*/		
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
	void pri_stat_proc_lwsn (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_pri_stat_proc_lwsn_init (int * init_block_ptr);
	void _op_pri_stat_proc_lwsn_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_pri_stat_proc_lwsn_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_pri_stat_proc_lwsn_alloc (VosT_Obtype, int);
	void _op_pri_stat_proc_lwsn_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
pri_stat_proc_lwsn (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (pri_stat_proc_lwsn ());

		{
		/* Temporary Variables */
		//int hop_TV;
		//int * pkt_num_TV;
		/* End of Temporary Variables */


		FSM_ENTER ("pri_stat_proc_lwsn")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (0, "idle", state0_enter_exec, "pri_stat_proc_lwsn [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (1,"pri_stat_proc_lwsn")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "idle", "pri_stat_proc_lwsn [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("pri_stat_proc_lwsn [idle trans conditions]", state0_trans_conds)
			FSM_INIT_COND (REMOTE_STAT)
			FSM_TEST_COND (END)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state0_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 1, state1_enter_exec, ;, "REMOTE_STAT", "", "idle", "stat", "tr_1", "pri_stat_proc_lwsn [idle -> stat : REMOTE_STAT / ]")
				FSM_CASE_TRANSIT (1, 2, state2_enter_exec, ;, "END", "", "idle", "end", "tr_9", "pri_stat_proc_lwsn [idle -> end : END / ]")
				FSM_CASE_TRANSIT (2, 0, state0_enter_exec, ;, "default", "", "idle", "idle", "tr_0", "pri_stat_proc_lwsn [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (stat) enter executives **/
			FSM_STATE_ENTER_FORCED (1, "stat", state1_enter_exec, "pri_stat_proc_lwsn [stat enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_stat_proc_lwsn [stat enter execs]", state1_enter_exec)
				{
				deal_with_ici();
				}
				FSM_PROFILE_SECTION_OUT (state1_enter_exec)

			/** state (stat) exit executives **/
			FSM_STATE_EXIT_FORCED (1, "stat", "pri_stat_proc_lwsn [stat exit execs]")


			/** state (stat) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "stat", "idle", "tr_2", "pri_stat_proc_lwsn [stat -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (end) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "end", state2_enter_exec, "pri_stat_proc_lwsn [end enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_stat_proc_lwsn [end enter execs]", state2_enter_exec)
				{
				func_sim_end();
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (end) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "end", "pri_stat_proc_lwsn [end exit execs]")


			/** state (end) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "end", "idle", "tr_7", "pri_stat_proc_lwsn [end -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (3, "init", "pri_stat_proc_lwsn [init enter execs]")
				FSM_PROFILE_SECTION_IN ("pri_stat_proc_lwsn [init enter execs]", state3_enter_exec)
				{
				// Obtain related ID
				process_id = op_id_self();
				node_id = op_topo_parent(process_id);
				op_ima_obj_attr_get(node_id, "Node Type",&node_type);
				//op_ima_obj_attr_get(node_id, "user id", &node_address);
				op_ima_obj_attr_get(node_id, "Steady Time",&steady_time);
				Dep_Admin_id = op_id_from_name(op_topo_parent(node_id),OPC_OBJTYPE_NDFIX,"Deployments_Admin");
				
				op_ima_obj_attr_get(op_id_from_name(Dep_Admin_id,OPC_OBJTYPE_PROC,"Deployment_Admin"),"Chain Number",&chain_number);
				//printf("chain_number is %d\n",chain_number);
				
				op_ima_obj_attr_get(node_id, "Debug",&debug);
				op_ima_obj_attr_get(node_id, "Line Hops",&line_hops);
				
				src_to_sink_delay_list = op_prg_list_create();
				if(node_type==sink){
					grade_dist_list = op_prg_list_create();
					node_child_list = op_prg_list_create();
					line_delay_list = op_prg_list_create();
					duty_cycle_list = op_prg_list_create();
					src_relay_delay_list = op_prg_list_create();
					
					//hop_pkt_rcved_num_list = op_prg_list_create();
				/*	hop_src_pkt_num_list = op_prg_list_create();
					for(hop_TV=0;hop_TV<line_hops;hop_TV++)
					{
						pkt_num_TV = (int *)op_prg_mem_alloc(sizeof(int));
						*pkt_num_TV = 0;
						op_prg_list_insert(hop_src_pkt_num_list,pkt_num_TV,OPC_LISTPOS_TAIL);
					}
				*/
				}
				
				
				//statistics regester
				delay_in_random_handle = op_stat_reg("Random.Delay in Random", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				grade_dist_handle = op_stat_reg("General.Grade Distribution", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				node_child_handle = op_stat_reg("General.Node Children", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				line_delay_handle = op_stat_reg("Line.Average E2E Delay", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				pkt_delivery_ratio_handle = op_stat_reg("General.Packet Delivery Ratio", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				ec_per_rcvd_pkt_handle = op_stat_reg("General.EC Per Packet", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				throughput_handle = op_stat_reg("General.Throughput", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				avg_hop_duty_cycle_handle = op_stat_reg("General.Avg Hop Duty Cycle", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				
				avg_src_delay_handle = op_stat_reg("General.Avg Src Delay", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				avg_relay_delay_handle = op_stat_reg("General.Avg Relay Delay", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				avg_hop_delay_handle = op_stat_reg("General.Avg Hop Delay", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				
				//avg_hop_pkt_delivery_ratio_handle = op_stat_reg("General.Avg Hop Pkt Delivery Ratio", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "init", "pri_stat_proc_lwsn [init exit execs]")


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "init", "idle", "tr_10", "pri_stat_proc_lwsn [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (3,"pri_stat_proc_lwsn")
		}
	}




void
_op_pri_stat_proc_lwsn_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_pri_stat_proc_lwsn_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_pri_stat_proc_lwsn_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_pri_stat_proc_lwsn_svar function. */
#undef node_id
#undef process_id
#undef node_type
#undef node_gent_pkt_num
#undef dest_rcved_pkt_num
#undef delay_in_random_handle
#undef grade_dist_handle
#undef node_child_handle
#undef src_to_sink_delay_list
#undef line_delay_handle
#undef pkt_delivery_ratio_handle
#undef ec_per_rcvd_pkt_handle
#undef throughput_handle
#undef dest_rcved_pkt_num_throughput
#undef steady_time
#undef Dep_Admin_id
#undef chain_number
#undef avg_hop_duty_cycle_handle
#undef avg_src_delay_handle
#undef avg_relay_delay_handle
#undef avg_hop_delay_handle
#undef line_hops
#undef debug

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_pri_stat_proc_lwsn_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_pri_stat_proc_lwsn_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (pri_stat_proc_lwsn)",
		sizeof (pri_stat_proc_lwsn_state));
	*init_block_ptr = 6;

	FRET (obtype)
	}

VosT_Address
_op_pri_stat_proc_lwsn_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	pri_stat_proc_lwsn_state * ptr;
	FIN_MT (_op_pri_stat_proc_lwsn_alloc (obtype))

	ptr = (pri_stat_proc_lwsn_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "pri_stat_proc_lwsn [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_pri_stat_proc_lwsn_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	pri_stat_proc_lwsn_state		*prs_ptr;

	FIN_MT (_op_pri_stat_proc_lwsn_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (pri_stat_proc_lwsn_state *)gen_ptr;

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
	if (strcmp ("node_type" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_type);
		FOUT
		}
	if (strcmp ("node_gent_pkt_num" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_gent_pkt_num);
		FOUT
		}
	if (strcmp ("dest_rcved_pkt_num" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->dest_rcved_pkt_num);
		FOUT
		}
	if (strcmp ("delay_in_random_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->delay_in_random_handle);
		FOUT
		}
	if (strcmp ("grade_dist_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->grade_dist_handle);
		FOUT
		}
	if (strcmp ("node_child_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->node_child_handle);
		FOUT
		}
	if (strcmp ("src_to_sink_delay_list" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->src_to_sink_delay_list);
		FOUT
		}
	if (strcmp ("line_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->line_delay_handle);
		FOUT
		}
	if (strcmp ("pkt_delivery_ratio_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pkt_delivery_ratio_handle);
		FOUT
		}
	if (strcmp ("ec_per_rcvd_pkt_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->ec_per_rcvd_pkt_handle);
		FOUT
		}
	if (strcmp ("throughput_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->throughput_handle);
		FOUT
		}
	if (strcmp ("dest_rcved_pkt_num_throughput" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->dest_rcved_pkt_num_throughput);
		FOUT
		}
	if (strcmp ("steady_time" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->steady_time);
		FOUT
		}
	if (strcmp ("Dep_Admin_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->Dep_Admin_id);
		FOUT
		}
	if (strcmp ("chain_number" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->chain_number);
		FOUT
		}
	if (strcmp ("avg_hop_duty_cycle_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_hop_duty_cycle_handle);
		FOUT
		}
	if (strcmp ("avg_src_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_src_delay_handle);
		FOUT
		}
	if (strcmp ("avg_relay_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_relay_delay_handle);
		FOUT
		}
	if (strcmp ("avg_hop_delay_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->avg_hop_delay_handle);
		FOUT
		}
	if (strcmp ("line_hops" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->line_hops);
		FOUT
		}
	if (strcmp ("debug" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->debug);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

