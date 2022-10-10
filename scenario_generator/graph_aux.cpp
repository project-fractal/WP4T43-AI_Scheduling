#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include <assert.h>
#include <algorithm>
#include "stdafx.h"
#include "gen_logical.h"
#include "gen_physical.h"
#include "graph_aux.h"
//#include "stdlib.h"

int naming = 0;

using namespace std;

extern int is_sink_node(PNGraph  G, int node_id);
extern int is_source_node(PNGraph  G, int node_id);


int is_sink_node(PNGraph  G, int node_id) {
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
     if (EI.GetSrcNId()== node_id)
     	 return 0;
  }
  return 1;
}

int is_source_node(PNGraph  G, int node_id) {
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
     if (EI.GetDstNId()== node_id)
     	 return 0;
  }
  return 1;
}


void add_to_graph(PUNGraph subgraph, std::vector<int> &IDs) {
  for (unsigned int n=0;n<IDs.size();n++) 
    subgraph->AddNode(IDs[n]);          
}

void add_to_graph(PUNGraph subgraph, PUNGraph overallgraph, std::vector<int> &IDs) {
  
  add_to_graph(subgraph,IDs);
                
  for (TUNGraph::TEdgeI EI = overallgraph->BegEI(); EI < overallgraph->EndEI(); EI++) {
    	int src = EI.GetSrcNId();
    	int dst = EI.GetDstNId();  	
    
      if ((subgraph->IsNode(src)) && (subgraph->IsNode(dst)))     	
        subgraph->AddEdge(src, dst);    	           		  		    	
  }    	 
}

void add_to_graph(PNGraph subgraph, std::vector<int> &IDs) {
  for (unsigned int n=0;n<IDs.size();n++) 
    subgraph->AddNode(IDs[n]);           
}


void add_to_graph(PNGraph subgraph, PNGraph overallgraph, std::vector<int> &IDs) {
  
  add_to_graph(subgraph,IDs);  
  
  for (TNGraph::TEdgeI EI = overallgraph->BegEI(); EI < overallgraph->EndEI(); EI++) {
   int src = EI.GetSrcNId();
   int dst = EI.GetDstNId();  	
    
    if ((subgraph->IsNode(src)) && (subgraph->IsNode(dst)))     	
      subgraph->AddEdge(src, dst);    	           		  		    	
  }    	    
}


void print_graph(std::ofstream &schedfile, PNGraph G) {

  std::map <int,int> node_id;
  int node_index = 0;

  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
   node_id[node_index++] = NI.GetId();   
  }

  for (int i=0;i<node_index;i++) {
    for (int j=0;j<node_index;j++) {
    	 schedfile  << (G->IsEdge(node_id[i],node_id[j]) ? "1" : "0");
    }      
    schedfile  << "\n";
  }
}

		
void cout_print_nodes(std::vector<int> &v) {
	for (unsigned int n=0;n<v.size();n++)
	 std::cout << v[n] << " ";
	std::cout << "\n";
}		
		
void cout_print_nodes(PNGraph &G) {
	for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
      std::cout << NI.GetId() << " ";         
  }
  std::cout << "\n";
}

void cout_print_nodes(PUNGraph &G) {
	for (TUNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
      std::cout << NI.GetId() << " ";         
  }
  std::cout << "\n";
}

void print_graph(std::ofstream &schedfile, PUNGraph G, std::vector<ID> &SoS_Info) {

  std::map <int,int> node_id;
  int node_index = 0;



  //schedfile << "# ";
  for (TUNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
      node_id[node_index++] = NI.GetId();   
            
      //if (pnode_info[NI.GetId()].t == T_SW) 
      	//schedfile << "S ";
      //else	
      	//schedfile << "E ";
      	
  }
  
 // schedfile << "\n";
  
  for (int i=0;i<node_index;i++) {
  	
  	if (i==0)
  	  schedfile  << "[[";    
  	else 
  	  schedfile  << ",\n [";    
  	  
    for (int j=0;j<node_index;j++) {
    	 if (j>0) schedfile  << ",";
    	 schedfile  << (G->IsEdge(node_id[i],node_id[j]) ? "1" : "0");
    }      
	  schedfile  << "]";    
  }
  schedfile  << "]\n";    
}


void print_nodes_in_vector(std::ofstream &schedfile, PUNGraph &G, std::vector<int> &v) {
 
  std::vector <int> is_in_vector;
  std::map <int,int> index;
 	unsigned int n = 0;
 	
  for (TUNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {   
   is_in_vector.push_back(0);
   index[NI.GetId()] = n++;   
  }
  	  
  schedfile << "nodes in vector\n";
  schedfile << "["; 	  
   	  
  for (unsigned int i=0;i<v.size();i++) 
    is_in_vector[index[v[i]]]=1;
  	  	
  for (unsigned int i=0;i<n;i++) {
    if (i>0) schedfile << ",";
	
    if (is_in_vector[i]) 
    	 schedfile << "1";
    else
    	 schedfile << "0";
  }
     
  schedfile << "]\n";
  		
}
void print_nodes_in_vector(std::ofstream &schedfile, PNGraph &G, std::vector<int> &v) {
 
  std::vector <int> is_in_vector;
  std::map <int,int> index;
 	unsigned int n = 0;
 	
 	
  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {   
   is_in_vector.push_back(0);
   index[NI.GetId()] = n++;   
  }
  	  
 schedfile << "["; 	  
  	  
  for (unsigned int i=0;i<v.size();i++) 
    is_in_vector[index[v[i]]]=1;
  	  	
  for (unsigned int i=0;i<n;i++) {
    
    if (i>0) schedfile << ",";
    
    if (is_in_vector[i]) 
    	 schedfile << "1";
    else
    	 schedfile << "0";
     
  }   
     
  schedfile << "]\n";
  		
}


void print_CPLEX_sender_job(std::ofstream &schedfile, PNGraph &G, int app_id) {
  std::map <int,int> node_id;
  int node_index = 0;

  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
      node_id[NI.GetId()] = node_index++;   
  }  
  
  schedfile << "# Sender Job of Msg.\n[";
    schedfile << "[";
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
    if (EI!=G->BegEI()) schedfile << ",";
    schedfile << node_id[EI.GetSrcNId()];      
  }

  schedfile <<  "]\n";

  schedfile << "# Sender Job of Msg. (Globally Unique Job-ID)\n[";
   schedfile << "[";
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
    if (EI!=G->BegEI()) schedfile << ",";
    schedfile << EI.GetSrcNId();      
  }

  schedfile <<  "]\n";
  
  schedfile << "# Sender SS of Msg.\n[";
    schedfile << "[";
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
    if (EI!=G->BegEI()) schedfile << ",";
    schedfile << app.info[app_id].node_ss_map[EI.GetSrcNId()];       
  }

  schedfile <<  "]\n";

  schedfile << "# Destination SS of Msg.\n[";
    schedfile << "[";
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
    if (EI!=G->BegEI()) schedfile << ",";
    schedfile << app.info[app_id].node_ss_map[EI.GetDstNId()];       
  }

  schedfile <<  "]\n";

  schedfile << "# Globally Unique Message IDs \n[";
    schedfile << "[";
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
    if (EI!=G->BegEI()) schedfile << ",";    
    schedfile << EI.GetSrcNId()*2+EI.GetDstNId()*3;          
  }
   

  schedfile <<  "]\n";    
  
  schedfile << "# Sender CS of Msg.\n[";
    schedfile << "[";
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
    if (EI!=G->BegEI()) schedfile << ",";
    schedfile << ss_cs_map[app.info[app_id].node_ss_map[EI.GetSrcNId()]];       
  }

  schedfile <<  "]\n";    
  
  //schedfile << "# Destination CS of Msg.\n[";
    schedfile << "[";
  for (TNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI++) {
    if (EI!=G->BegEI()) schedfile << ",";
    schedfile << ss_cs_map[app.info[app_id].node_ss_map[EI.GetDstNId()]];       
  }
        

  
  
}

 
void print_CPLEX_structures(std::ofstream &schedfile, PUNGraph &G_CS, PNGraph &G_SS, int period, int hop_trans, int execution, std::vector<ID> &SoS_Info, int app_id) {
  schedfile << "# ================== Scheduling Problem ==================\n";                     
  //schedfile << "[" << G_SS->GetNodes() << "," << G_SS->GetEdges() << "," << G_CS->GetNodes() << "] # SS-Nodes, SS-Edges, CS-Nodes\n"; 

schedfile << "[" << G_SS->GetNodes() << "," << G_SS->GetEdges() << "," << G_CS->GetNodes() << "]\n";

  schedfile << "# Connection / Phys. Graph\n";   
  // needs to be print in sorted manner !!! 123
  
  /*****************/


  std::map <int,int> node_id;
  int node_index = 0;
  //int app_id = app;
  for (TNGraph::TNodeI NI = G_SS->BegNI(); NI < G_SS->EndNI(); NI++) {
      node_id[NI.GetId()] = node_index++;   
  }  
  
  schedfile << "# Sender Job of Msg.\n[";
    schedfile << "[";
  for (TNGraph::TEdgeI EI = G_SS->BegEI(); EI < G_SS->EndEI(); EI++) {
    if (EI!=G_SS->BegEI()) schedfile << ",";
    schedfile << node_id[EI.GetSrcNId()];      
  }

  schedfile <<  "]\n";

  schedfile << "# Sender Job of Msg. (Globally Unique Job-ID)\n[";
   schedfile << "[";
  for (TNGraph::TEdgeI EI = G_SS->BegEI(); EI < G_SS->EndEI(); EI++) {
    if (EI!=G_SS->BegEI()) schedfile << ",";
    schedfile << EI.GetSrcNId();      
  }

  schedfile <<  "]\n";
  
  /****************/
  // Sender Job Output  
  //print_CPLEX_sender_job(schedfile, G_SS,app);
  
  
  schedfile << "]\n"; 
  schedfile << "# Destination\n";
  for (TNGraph::TEdgeI mEI = G_SS->BegEI(); mEI < G_SS->EndEI(); mEI++) {	
  	
  	if (mEI==G_SS->BegEI())
  	  schedfile  << "[[";    
  	else 
  	  schedfile  << ",\n [";    
  	  	
    for (TNGraph::TNodeI jNI = G_SS->BegNI(); jNI < G_SS->EndNI(); jNI++) {
			 if (jNI!=G_SS->BegNI()) schedfile << ",";
       if (mEI.GetDstNId() == jNI.GetId())
       	  schedfile << "1"; 
       	else
       		schedfile << "0";        
    }
    schedfile  << "]";  
  }
  schedfile  << "]\n";  
  
  schedfile << "# Period\n[";
  schedfile << "[";
  for (TNGraph::TEdgeI EI = G_SS->BegEI(); EI < G_SS->EndEI(); EI++) {
    if (EI!=G_SS->BegEI()) schedfile << ",";
    schedfile << period;      
  }
  schedfile  << "]\n";  

 schedfile << "# message size\n [";
  for (TNGraph::TEdgeI EI = G_SS->BegEI(); EI < G_SS->EndEI(); EI++) {
    if (EI!=G_SS->BegEI()) schedfile << ",";
    schedfile << hop_trans;      
  }
  schedfile  << "]\n";
  
  schedfile << "# job execution\n [";
 for (TNGraph::TNodeI jNI = G_SS->BegNI(); jNI < G_SS->EndNI(); jNI++) {
			 if (jNI!=G_SS->BegNI()) schedfile << ",";
              		schedfile << execution;        
    }
    schedfile  << "]\n";  

  //  schedfile << "# Log. Graph AppId=" << app_id << " " << "SS=" << ss << "\n";   
  // print_graph(schedfile, G_SS);
  // schedfile  << "\n";    
  
  

}



//void print_CPLEX_structures_to_json(std::ofstream &schedfile, PUNGraph &G_CS, PNGraph &G_SS, int period, int hop_trans, int execution, std::vector<ID> &SoS_Info, int app_id, TIntStrH &name, int nsubs)
void print_CPLEX_structures_to_json(std::ofstream &schedfile, PUNGraph &G_CS, PNGraph &G_SS, int period, int hop_trans, int execution, std::vector<ID> &SoS_Info, int app_id, TIntStrH &name) {
  //srand(time(NULL));
  naming = naming +  1;
  int index = 1;
  int nsubs = 4;
  int j;
  schedfile << "{\n";
  //schedfile << "    \"Name\": {\"example_N";
  //schedfile <<  naming;
  //schedfile << ".json\"";
  //schedfile << "},\n";                    
  schedfile << "    \"application\": {\n";
  schedfile << "\t\"jobs\": [\n";

 
  index = 0;
  std::map <int,int> node_id;
  std::vector <int>  ES;
  std::vector <int>  runs_on;
  std::map <int, vector<int> > can_run_on;
  
  for (TUNGraph::TNodeI NI = G_CS->BegNI(); NI < G_CS->EndNI(); NI++) 
      node_id[index++] = NI.GetId();
  index = 0;
  
  for (TUNGraph::TNodeI NI = G_CS->BegNI(); NI < G_CS->EndNI(); NI++) {

    char level[32];
    char type[32];
    int id;
    int cs;
    int manager;

    sscanf(name.GetDat(NI.GetId()).CStr(),"%s %s %d %d %d",type, level, &id, &cs, &manager);  	
 
    if (strcmp(type,"es")==0) 
      ES.push_back(index);
    index++;
  }

   index = 0;
   runs_on.clear();

     
   for (TNGraph::TNodeI jNI = G_SS->BegNI(); jNI < G_SS->EndNI(); jNI++) {
     
	//srand(time(NULL));
       if (jNI!=G_SS->BegNI()) schedfile << ",\n";
          schedfile << "\t    {\n";
       	  schedfile << "\t\t\"id\": "; 
       	  schedfile <<  jNI.GetId();
	  schedfile << ",\n";
	  schedfile << "\t\t\"wcet_fullspeed\": "; 
       	  //schedfile <<  execution;
	  schedfile <<  2+(rand()%(100-2+1));
	  schedfile << ",\n";
	  //schedfile << "\t\t\"mcet\": 0,\n ";
          schedfile << "\t\t\"mcet\": 0,\n ";

	  schedfile << "\t\t\"deadline\": "; 
       	    schedfile << "10000";

	  schedfile << ",\n";
	  schedfile << "\t\t\"can_run_on\": [";
         runs_on.clear();
	   int src;
	  for (TNGraph::TEdgeI EI = G_SS->BegEI(); EI < G_SS->EndEI(); EI++)
	    {
	      if (EI.GetDstNId() ==jNI.GetId())
		{
		  src = EI.GetSrcNId();
		}
	      else
		src = jNI.GetId();
	    }
	  j = 0;
	  std::vector <int>  machine;
	  machine.push_back(1);
	  machine.push_back(2);
	  machine.push_back(4);
	  machine.push_back(5);
          machine.push_back(7);
          machine.push_back(8);


	  std::vector <int>  machine2;		  
	  for(int i = 0; i < ((rand() % (ES.size()-3 + 1))+2) ;i++) //rand() % ES.size()
	    {
	      int endsystem = machine[rand() % machine.size()];
	      // if(std::find (runs_on.begin(), runs_on.end(), endsystem) != runs_on.end())
		// {
		   //if (std::find (can_run_on[src].begin(), can_run_on[src].end(), endsystem) == can_run_on[src].end())
		    // {
		 //      i--;
		     //  if(j != 0) schedfile << ",";
		     //  runs_on.push_back(endsystem);
		     //  can_run_on[index].push_back (endsystem);
		     //  schedfile << endsystem;
		     //  j++;
		    // }
		// }
		if(std::find(machine2.begin(), machine2.end(), endsystem) != machine2.end()){
			i--;
		}
		else{
		       if(j != 0) schedfile << ",";
		       machine2.push_back(endsystem);
		       machine.erase(std::remove(machine.begin(), machine.end(), endsystem), machine.end());
                       runs_on.push_back(endsystem);
                       can_run_on[index].push_back (endsystem);
                       schedfile << endsystem;
                       j++;
		}
	    }
	  schedfile << "]\n";

          schedfile << "\n";       
	  schedfile << "\t    }";

	  index++;

   }

   schedfile << "\n\t],\n";
 
  schedfile << "\t\"messages\": [\n";
   index = 0;

   for (TNGraph::TEdgeI EI = G_SS->BegEI(); EI < G_SS->EndEI(); EI++)
     {


       if (EI!=G_SS->BegEI()) schedfile << ",\n";
       schedfile << "\t    {\n";
       schedfile << "\t\t\"id\": "; 
       schedfile <<  index;
       schedfile << ",\n";
       schedfile << "\t\t\"sender\": "; 
       schedfile <<  EI.GetSrcNId();
       schedfile << ",\n";
       schedfile << "\t\t\"receiver\": ";
       schedfile <<  EI.GetDstNId();
       schedfile << ",\n";
       schedfile << "\t\t\"size\": "; 
       // schedfile <<  (hop_trans * (1 + ( index %3)));// ((index + 1)%5)) % G_SS->GetEdges() ;
       //schedfile << "20";
       schedfile << 15+(rand()%(20-10+1));
       schedfile << ",\n";
       schedfile << "\t\t\"timetriggered\": true,\n ";
       schedfile << "\t\t\"period\": ";
       schedfile <<    10 + (20 *( index %3));//1000;// * ( (index+1) %5);//(600 + (200 * ( index %5)));
       schedfile << "\n\t    }";
       index++;
     }

   

   schedfile << "\n\t]\n";
   schedfile << "    },\n";
   schedfile << "    \"platform\": {\n";
   schedfile << "\t\"nodes\": [\n";
   index = 0;
 


  for (TUNGraph::TNodeI NI = G_CS->BegNI(); NI < G_CS->EndNI() ; NI++) 
      node_id[index++] = NI.GetId();
  index = 0;
  for (TUNGraph::TNodeI NI = G_CS->BegNI(); NI < G_CS->EndNI(); NI++) {
    
    char level[32];
    char type[32];
    int id;
    int cs;
    int manager;  		

    sscanf(name.GetDat(NI.GetId()).CStr(),"%s %s %d %d %d",type, level, &id, &cs, &manager);  	
 
    if (NI!=G_CS->BegNI()) schedfile  << ",\n";
    schedfile << "\t    {\n";
    schedfile << "\t\t\"id\": "; 
    schedfile <<  index;
    schedfile << ",\n";
    schedfile << "\t\t\"is_router\": "; 
    if (strcmp(type,"es")==0) 
      schedfile << "false\n";
    else
      schedfile << "true\n";
    schedfile << "\t    }";
    index++;

  }
  schedfile << "\n\t],\n";
  schedfile << "\t\"links\": [\n";
  int edge_index = 0;
  for (int i = 0;i < index ; i++) { 
    for (int j = 0;j < index ;j++) {
    	 
      if(G_CS->IsEdge(node_id[i],node_id[j]) && node_id[i] < node_id[j])
	{
	  if (edge_index != 0)  schedfile << ",\n";
	  schedfile << "\t    {\n";
	  schedfile << "\t\t\"start\": "; 
	  schedfile <<  i;
	  schedfile << ",\n";
	  schedfile << "\t\t\"end\": ";
	  schedfile <<  j;
	  schedfile << "\n";
	  schedfile << "\t    }";
	  edge_index++;
	}
    }          
  }


  schedfile << "\n\t],\n";
  schedfile << "\t\"frequencies\": [500, 1000],\n";
  schedfile << "\t\"schemes\": [\n\t    {\n";
  schedfile << "\t\t\"id\": 0,\n ";
  schedfile << "\t\t\"wcdt\": 0,\n ";
  schedfile << "\t\t\"wcct\": 0,\n ";
  schedfile << "\t\t\"wccr\": 1\n ";
  schedfile << "\t    }";
  schedfile << "\n\t]\n";
  schedfile << "    }\n";
  schedfile << "}\n";

}
