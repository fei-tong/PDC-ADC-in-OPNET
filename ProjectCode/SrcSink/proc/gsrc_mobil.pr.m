MIL_3_Tfile_Hdr_ 145A 140A modeler 9 4C8894A8 51257F3D 2C fei-PC fei 0 0 none none 0 0 none A92BC089 1E90 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                    Ф═gЅ      @  D  H      ╗  l  p  t  x  ё  ѕ  ї  »      	Send DATA                                                                       ЦZ             
Start Time                                                                                   ЦZ             Interval                                                                              
   10   @$                9   @"                8   @                 7   @                6   @                5   @                4   @                3   @                2   @                 1   ?­                 ЦZ             Traffic Load                                infinity(-1)                                     infinity(-1)                 0(Remote or No traffic)                 1(test)                20                40      (          60      <          80      P          100      d           ЦZ             Debug                                                                       ЦZ                 	   begsim intrpt         
          
   doc file            	nd_module      endsim intrpt         
          
   failure intrpts            disabled      intrpt interval         н▓IГ%ћ├}          priority                        recovery intrpts            disabled      subqueue                     count          
          
      list   	      
          
      super priority                              Objid	\process_id;       Objid	\node_id;       double	\start_time;       double	\interval;       Boolean	\send_DATA;       int	\node_address;       int	\traffic_load;       Boolean	\infinity;       int	\node_type;       int	\self_pk_num;       Boolean	\debug;          //File   	FILE *in;   char temp_file_name[300];      #define SEND_STRM 0   //Define node type   #define sink 	1   #define sensor 	2       B#define END	        		    	(op_intrpt_type() == OPC_INTRPT_ENDSIM)       .//Self-interrupt code and transition condition   #define SEND_DATA_S_CODE		100   h#define SEND_DATA_S				((op_intrpt_type() == OPC_INTRPT_SELF) && (op_intrpt_code() == SEND_DATA_S_CODE))   0//Remote-interrupt code and transition condition   #define SEND_DATA_R_CODE		200   j#define SEND_DATA_R				((op_intrpt_type() == OPC_INTRPT_REMOTE) && (op_intrpt_code() == SEND_DATA_R_CODE))       int pk_num=0;   //function prototype   'static void create_and_send_DATA(void);   data_num=0;      //create DATA pk   static void    create_and_send_DATA(void)   {   //var   	Packet * pk_DATA;   //in   	FIN(create_and_send_DATA());   //body   	data_num++;   +	pk_DATA = op_pk_create_fmt("MAC_DATA_PK");   $	op_pk_nfd_set(pk_DATA,"Hop Num",0);   4	op_pk_nfd_set(pk_DATA,"Create Time",op_sim_time());   +	op_pk_nfd_set(pk_DATA,"Data No",data_num);   	op_pk_send(pk_DATA,SEND_STRM);   //out   	FOUT;   }                                          ќ            
   init   
       
   "   // Obtain related ID   process_id = op_id_self();   &node_id = op_topo_parent(process_id);       8op_ima_obj_attr_get(process_id, "Send DATA",&send_DATA);   :op_ima_obj_attr_get(process_id, "Start Time",&start_time);   6op_ima_obj_attr_get(process_id, "Interval",&interval);   >op_ima_obj_attr_get(process_id, "Traffic Load",&traffic_load);   0op_ima_obj_attr_get(process_id, "Debug",&debug);       7op_ima_obj_attr_get(node_id, "user id", &node_address);   5op_ima_obj_attr_get(node_id, "Node Type",&node_type);           if(traffic_load<0)   {   	infinity = OPC_TRUE;   }else   {   	infinity = OPC_FALSE;   }           if(send_DATA&&traffic_load<0)   {   F	op_intrpt_schedule_self(op_sim_time() + start_time,SEND_DATA_S_CODE);   }   if(send_DATA&&traffic_load>0)   {   F	op_intrpt_schedule_self(op_sim_time() + start_time,SEND_DATA_S_CODE);   	--traffic_load;   }   //pk_num=0;   self_pk_num=0;   
                     
          
          pr_state        J            
   idle   
                                   
           
          pr_state        J   Z          
   send DATA S   
       
      create_and_send_DATA();   	pk_num++;   self_pk_num++;   if(!infinity && traffic_load>0)   {   D	op_intrpt_schedule_self(op_sim_time() + interval,SEND_DATA_S_CODE);   	--traffic_load;   }	   if(infinity)   {   D	op_intrpt_schedule_self(op_sim_time() + interval,SEND_DATA_S_CODE);   }       	if(debug)   rprintf("Node: %d. In \"gsrc\",time:%f,\nHave created DATA and sent it to \"gmac\".\n",node_address,op_sim_time());   
                     
          
          pr_state        J  ┬          
   send DATA R   
       
      create_and_send_DATA();   	pk_num++;   self_pk_num++;       	if(debug)   3printf("In \"gsrc\",Remote Interruption,time:%f,\n\   1		Have created DATA and sent it to \"gmac\".\n",\   		op_sim_time());   
                     
          
          pr_state        ■            
   end   
       J      if(send_DATA)   {   /	if(op_ima_obj_attr_exists(node_id,"Log File"))   	{	   9		op_ima_obj_attr_get(node_id,"Log File",temp_file_name);   "		in = fopen(temp_file_name,"at");   є		fprintf(in,"Node %d sent %d packets.\nAll soruces totally sent %d packets. (in \"gsrc->end\")\r\n",node_address,self_pk_num,pk_num);   A		fprintf(in,"The sending interval is %f seconds.\r\n",interval);   9		fprintf(in,"Simulation time: %f s.\r\n",op_sim_time());   		fprintf(in,"End.\r\n\r\n");   		fclose(in);   	}   }else       %if(traffic_load==0 && self_pk_num!=0)   {       /	if(op_ima_obj_attr_exists(node_id,"Log File"))   	{	   9		op_ima_obj_attr_get(node_id,"Log File",temp_file_name);   "		in = fopen(temp_file_name,"at");   є		fprintf(in,"Node %d sent %d packets.\nAll sources totally sent %d packets.(in \"gsrc->end\").\r\n",node_address,self_pk_num,pk_num);   7		//fprintf(in,"иб░Ч╝СИЗ╬фБ║%f seconds.\r\n",interval);   9		fprintf(in,"Simulation time: %f s.\r\n",op_sim_time());   		fprintf(in,"End.\r\n\r\n");   		fclose(in);   	}   }   J                                          pr_state                        Э        г    D            
   tr_0   
                                   
           
                                     pr_transition              P   │     Q   t  Q   Ы          
   tr_2   
                                   
           
                                     pr_transition                 ┤     F   ш  G   v          
   tr_3   
       
   SEND_DATA_S   
                     
           
                                     pr_transition                i     E  %  D  Д          
   tr_4   
       
   SEND_DATA_R   
                     
           
                                     pr_transition              T  g     S  Ф  T  #          
   tr_5   
                                   
           
                                     pr_transition              Ф       [    Щ            
   tr_6   
       
   END   
                     
           
                                     pr_transition                                             