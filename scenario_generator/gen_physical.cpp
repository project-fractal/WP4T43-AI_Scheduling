#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <assert.h>
#include "stdafx.h"
#include "gen_physical.h"
using namespace std;

std::map<int,physical_node_info> pnode_info;
	

TIntStrH SoS_name;  	

class PrioritizedNodeId {
	private:
	public:
	int m_priority;
	int node_id; 
			
	explicit PrioritizedNodeId()  {}
	explicit PrioritizedNodeId(int id, const int priority = 0) : m_priority(priority), node_id(id)  {}
			
	const bool operator <(const PrioritizedNodeId& pN) const
	
	{
		return (m_priority > pN.m_priority);
	}
};
			
void MergeInto(PUNGraph TargetG, PUNGraph G2, TIntStrH *TargetName, TIntStrH *SourceName, int offset) {
  
  for (TUNGraph::TNodeI NI = G2->BegNI(); NI < G2->EndNI(); NI++) {
  	int id = offset+NI.GetId();
  	TargetG->AddNode(id);
  	TargetName->AddDat(id)=SourceName->GetDat(NI.GetId()).CStr();
  }
  
  //traverse the edges
  for (TUNGraph::TEdgeI EI = G2->BegEI(); EI < G2->EndEI(); EI++) {
    TargetG->AddEdge(offset+EI.GetSrcNId(), offset+EI.GetDstNId());    	         
  }
};

int rnd_minmax(int min,int max) 
{
  return rand() % (max-min+1) + min;
}

int sufficient_nodes(int Nodes, int Edges, int IsDir)
{
 return (1.0 * (Nodes-1) / 2 * (IsDir ? 2 : 1)) >= (1.0 * Edges / Nodes);
}

// Function which is resposible for physical model creation
void CreateConstituentSystem(TIntStrH &name, PUNGraph CS, int MIN_nodes_per_switch, int MAX_nodes_per_switch, int MIN_switches, int MAX_switches, int cs_id, str_physical_structure_parameters &physical,int ID_id, int topology, int nr_es, int nr_rs_r, int nr_rs_c) 
{
	
  char switch_name[32];
  char es_name[32];  
  int manager;
  sprintf(switch_name,"sw CS %d %d 0",ID_id,cs_id);
  sprintf(es_name,"es CS %d %d 0",ID_id,cs_id);
  

  int val_1;
  int val_2;
  int n_switches = rnd_minmax(MIN_switches,MAX_switches);  
  int n_edges    = (int) (n_switches*1.5);

 //for bus and Grid topology 

  if(topology==0)
  {
    val_1=MIN_switches;
  }
  else if(topology==1)
  {
    val_1=1;
  }
  PUNGraph CS_Switches = TSnap::GenGrid<PUNGraph>(val_1,MAX_switches,false);  

  int id_offset        = CS_Switches->GetNodes();

  for (TUNGraph::TNodeI NI = CS_Switches->BegNI(); NI < CS_Switches->EndNI(); NI++) 
  {  	
		CS->AddNode(NI.GetId());
		name.AddDat(NI.GetId(),switch_name);
 // For Custom need to uncomment the following 
 /*
  PUNGraph CS_Switches = PUNGraph::New();
  CS_Switches->AddNode(200);
  CS_Switches->AddNode(201);
  CS_Switches->AddNode(202);
  CS_Switches->AddNode(203);
  CS_Switches->AddNode(204);
  CS_Switches->AddNode(205);
  CS_Switches->AddNode(206);
  CS_Switches->AddNode(207);
  CS_Switches->AddNode(208);

  CS_Switches->AddEdge(200,201);
  CS_Switches->AddEdge(201,202);
  CS_Switches->AddEdge(200,203);
  CS_Switches->AddEdge(203,204);
  CS_Switches->AddEdge(204,205);
  CS_Switches->AddEdge(203,206);
  CS_Switches->AddEdge(206,207);
  CS_Switches->AddEdge(207,208);
  CS_Switches->AddEdge(205,208);
  CS_Switches->AddEdge(202,205);
 */

// Creating end-systems on corner switches in grid
		
PUNGraph Star = TSnap::GenStar<PUNGraph>( rnd_minmax(MIN_nodes_per_switch,MAX_nodes_per_switch),false);  	  
	if(topology==0){
	if (NI.GetId()==0 && nr_es>=1|| NI.GetId()==nr_rs_c-1 && nr_es>1||NI.GetId()==(nr_rs_r*nr_rs_c)-nr_rs_c && nr_es>2|| NI.GetId()==(nr_rs_r*nr_rs_c)-1 && nr_es>3){
		for (TUNGraph::TNodeI SNI = Star->BegNI(); SNI < Star->EndNI(); SNI++){	
			int id = id_offset+SNI.GetId();
			CS->AddNode(id);
			name.AddDat(id,es_name);
			CS->AddEdge(id, NI.GetId());    	             	  
		} 
		id_offset += Star->GetNodes();
	}
	}
// Creating end-systems on corner switches in bus
	else{
	if (NI.GetId()<nr_es){ 
		for (TUNGraph::TNodeI SNI = Star->BegNI(); SNI < Star->EndNI(); SNI++){	
			int id = id_offset+SNI.GetId();
			CS->AddNode(id);
			name.AddDat(id,es_name);
			CS->AddEdge(id, NI.GetId());    	             	  
		} 
		id_offset += Star->GetNodes();
	}
   	}
    }
do {
manager = CS->GetRndNId();	   	  
} 
while (name.GetDat(manager).CStr()[0]=='s'); 
		char es_name_manager[32];
		sprintf(es_name_manager,"es CS %d %d 1",ID_id,cs_id);
		name.AddDat(manager,es_name_manager);

// Custom build end-system
/*	
PUNGraph Star = TSnap::GenStar<PUNGraph>( rnd_minmax(MIN_nodes_per_switch,MAX_nodes_per_switch),false); 
    if(NI.GetId()==200 || NI.GetId()==202 || NI.GetId()==206 || NI.GetId()==208)    { 
	for (TUNGraph::TNodeI SNI = Star->BegNI(); SNI < Star->EndNI(); SNI++) {
	  	int id = id_offset+SNI.GetId();
    		CS->AddNode(id);
    		name.AddDat(id,es_name);
    		CS->AddEdge(id, NI.GetId());    	             	  
  	} 
  	id_offset += Star->GetNodes();
    }
*/

//traverse the edges
for (TUNGraph::TEdgeI EI = CS_Switches->BegEI(); EI < CS_Switches->EndEI(); EI++){
	CS->AddEdge(EI.GetSrcNId(), EI.GetDstNId());    	         
}
}

void min_degree_switch(TUNGraph::TNodeI &min_degree_NI, PUNGraph &G,TIntStrH *Name) {

  min_degree_NI = G->BegNI(); 
		 
	 for (TUNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {  	 
	 	  if ((NI.GetOutDeg()<min_degree_NI.GetOutDeg()) && (Name->GetDat(NI.GetId()).CStr()[0]=='S')   )
        min_degree_NI = NI;
	 }
}

void CreateInteractionDomain(TIntStrH &ID_name, PUNGraph ID, int MIN_switches, int MAX_switches, int MIN_CS, int MAX_CS, int *min_degree_ID_switch, int ID_sequence_number, str_physical_structure_parameters &physical, int Topology, int nr_es, int nr_rs_r, int nr_rs_c) { 
	
	char switch_name[32];
	char manager_name[32];
	int n_switches       = rnd_minmax(MIN_switches,MAX_switches);
	int n_CS             = rnd_minmax(MIN_CS,MAX_CS);
	PUNGraph CS_Switches = TSnap::GenPrefAttach(n_switches, (int) (n_switches*1.5));	
	int id_offset        = CS_Switches->GetNodes();

	std::priority_queue <PrioritizedNodeId> ID_node_degree;
		
		
  sprintf(switch_name,"sw ID %d 0 0",ID_sequence_number);		
  sprintf(manager_name,"es ID %d 0 1",ID_sequence_number);		
		
  // copy ID nodes
	for (TUNGraph::TNodeI NI = CS_Switches->BegNI(); NI < CS_Switches->EndNI(); NI++) {  	 
	  ID->AddNode(NI.GetId());	 	  	 		
	 	ID_name.AddDat(NI.GetId()) = switch_name;	 	 	 	  
	 	
	 	// queue to find lowest degree nodes
	 	ID_node_degree.push(PrioritizedNodeId(NI.GetId(), NI.GetOutDeg()));
	 }
	 
	// add manager
	int switch_connected_to_manager = ID->GetRndNId();	   	  	
	ID->AddNode(id_offset);	 	  	 		 
  ID->AddEdge(switch_connected_to_manager, id_offset);    	         
  ID_name.AddDat(id_offset) = manager_name;	 	 	 	    	

	id_offset++; // manager added

  // copy ID edges
  for (TUNGraph::TEdgeI EI = CS_Switches->BegEI(); EI < CS_Switches->EndEI(); EI++) {
    ID->AddEdge(EI.GetSrcNId(), EI.GetDstNId());    	         
  }
	

	for (int n=0;	n<n_CS; n++) {		
			
  	int id_lowest_degree_ID_switch = ID_node_degree.top().node_id;			 	 
	  ID_node_degree.push(PrioritizedNodeId(id_lowest_degree_ID_switch, ID_node_degree.top().m_priority + 1));
	  ID_node_degree.pop();
		 		 		 		        	        	      
	  // create constituent system
    TIntStrH name;
    PUNGraph CS = TUNGraph::New();  
    CreateConstituentSystem(name, CS, physical.CS_MIN_nodes_per_switch,physical.CS_MAX_nodes_per_switch,physical.CS_MIN_switches,physical.CS_MAX_switches, n, physical,ID_sequence_number,  Topology,  nr_es,  nr_rs_r,  nr_rs_c);
		MergeInto(ID,CS,&ID_name,&name,id_offset);
   

	  //  connect to ID
	  TUNGraph::TNodeI CS_min_degree_NI; 
    min_degree_switch(CS_min_degree_NI,CS,&name);
		ID->AddEdge(id_lowest_degree_ID_switch,id_offset+CS_min_degree_NI.GetId());    	             	  

	  // new offset for next CS
		id_offset += CS->GetNodes();
	}
	    						
  (*min_degree_ID_switch) = ID_node_degree.top().node_id;
}






void CreateSoS(PUNGraph SoS, str_physical_structure_parameters &physical,int topology, int nr_es, int nr_rs_r, int nr_rs_c) { 
	
	TVec<TInt> MinDegree_ID_Switch;
	
	PUNGraph SoS_Graph = TSnap::GenPrefAttach(physical.number_of_interaction_domains, (int) (physical.number_of_interaction_domains*1.5));	
		
	int id_offset          = 0;	
	int ID_sequence_number = 0;
	
  // go through IDs (SoS_Graph nodes are IDs) and create an ID for each
	for (TUNGraph::TNodeI NI = SoS_Graph->BegNI(); NI < SoS_Graph->EndNI(); NI++) {  	 

	  int min_degree_ID_switch;
	  PUNGraph ID = TUNGraph::New();  
    TIntStrH ID_name;
 		CreateInteractionDomain(ID_name,ID,physical.MIN_switches_per_interaction_domain,physical.MAX_switches_per_interaction_domain,physical.MIN_CS_per_interaction_domain,physical.MAX_CS_per_interaction_domain,&min_degree_ID_switch,ID_sequence_number,physical, topology,  nr_es,  nr_rs_r,  nr_rs_c);
    MergeInto(SoS,ID,&SoS_name,&ID_name,id_offset);    
    MinDegree_ID_Switch.Add(min_degree_ID_switch + id_offset); // [ID_sequence_number]
	
		// new offset for next CS
		id_offset += ID->GetNodes();	  
	 
    ID_sequence_number++;
	 }
	 		 	
  // conntect IDs for each edge in SoS_Graph
  for (TUNGraph::TEdgeI EI = SoS_Graph->BegEI(); EI < SoS_Graph->EndEI(); EI++) {
    SoS->AddEdge(MinDegree_ID_Switch[EI.GetSrcNId()], MinDegree_ID_Switch[EI.GetDstNId()]);    	         
  }	 	
}

void CreateSoS_Info(PUNGraph SoS, std::vector<ID> &SoS_Info, str_physical_structure_parameters &physical) { 
	for (int i=0;i<physical.number_of_interaction_domains;i++) {
	  SoS_Info.push_back(ID());
	  SoS_Info[i].number_constituent_systems=0;
	}
	
	// first pass: add ID
  for (TUNGraph::TNodeI NI = SoS->BegNI(); NI < SoS->EndNI(); NI++) {  	 						
  	
  		char level[32];
  		char type[32];
  		int id;
  		int cs;
  		int manager;  		
  		sscanf(SoS_name.GetDat(NI.GetId()).CStr(),"%s %s %d %d %d",type, level, &id, &cs, &manager);  		
  		
  		
  		if (strcmp(level,"ID")==0) {
					// Interaction Domain  			
  			  if (strcmp(type,"sw")==0) {
			
						 //printf("level=%s type=%s id=%d cS=%d\n", level, type, id, cs);			
						 if (id>=physical.number_of_interaction_domains) { printf("error ID\n");exit(2); }
						 SoS_Info[id].sw.push_back(NI.GetId());		
  					 pnode_info.insert(std::pair<int,physical_node_info>(NI.GetId(),physical_node_info(id,-1,T_SW,NI.GetId())));  			  							 
  			  } else {
					  if (strcmp(type,"es")==0) {
						 if (id>=physical.number_of_interaction_domains) { printf("error ID\n");exit(2); }					  	
					  	SoS_Info[id].manager=NI.GetId();
						  pnode_info.insert(std::pair<int,physical_node_info>(NI.GetId(),physical_node_info(id,-1,T_ES,NI.GetId())));  			  							 
					  } else {
					    printf("invalid type %s\n",type);
  				    exit(1);
  				  }
  			  }
  			    	  			
  		   //printf("level=%s type=%s id=%d cS=%d\n", level, type, id, cs);
  		} else {
  		if (strcmp(level,"CS")==0) {
  			// Constituent System 
  			//printf("level=%s type=%s id=%d cS=%d\n", level, type, id, cs);

  		  if (SoS_Info[id].number_constituent_systems<(cs+1))
  		  	 SoS_Info[id].number_constituent_systems = cs+1;  
   		} else {
  			//printf("invalid level\n");
  			exit(1);
  	  }
  		}		  	
  }

  for (int i=0;i<physical.number_of_interaction_domains;i++) {
  	for (int j=0;j<SoS_Info[i].number_constituent_systems;j++) {
 	     SoS_Info[i].cs.push_back(CS());
  	}
  }
  		   
  // second pass: add CS
  for (TUNGraph::TNodeI NI = SoS->BegNI(); NI < SoS->EndNI(); NI++) {  	 						
  	
  	  int manager;
  		char level[32];
  		char type[32];
  		int id;
  		int cs;  		
  		sscanf(SoS_name.GetDat(NI.GetId()).CStr(),"%s %s %d %d %d",type, level, &id, &cs, &manager);  		
  		
  		if (strcmp(level,"ID")==0) {
					// Interaction Domain  			  			  
  		} else {
  		if (strcmp(level,"CS")==0) {
  			// Constituent System 
  			 			
  			if (strcmp(type,"sw")==0) {						
  				   SoS_Info[id].cs[cs].sw.push_back(NI.GetId());					
						 pnode_info.insert(std::pair<int,physical_node_info>(NI.GetId(),physical_node_info(id,cs,T_SW,NI.GetId())));
				} else {
				
  			  if (strcmp(type,"es")==0) {  			  	
  			  	SoS_Info[id].cs[cs].es.push_back(NI.GetId());					
  			  	//1node_info[NI.GetId()]=physical_node_info(id,cs,T_ES);
						pnode_info.insert(std::pair<int,physical_node_info>(NI.GetId(),physical_node_info(id,cs,T_ES,NI.GetId())));  			  	
  			  	if (manager) { SoS_Info[id].cs[cs].manager=NI.GetId();} ;
  			  } else {
					  printf("invalid type %s\n",type);
  				  exit(1);
  			  }  			   			  			
        } 			
  		} else {
  			printf("invalid level\n");
  			exit(1);
  	  }
  		}		  	
  }
  
  printf("Number of IDs: %d\n", physical.number_of_interaction_domains);    
  for (int i=0;i<physical.number_of_interaction_domains;i++) {
  	printf("ID#%d with %d CS and %d switches\n",i,(int) SoS_Info[i].cs.size(),(int) SoS_Info[i].sw.size());   	
  	for (int j=0;j<SoS_Info[i].number_constituent_systems;j++) {
  		  printf("  CS#%d with %d switches and %d end systems\n",j,(int) SoS_Info[i].cs[j].sw.size(),(int) SoS_Info[i].cs[j].es.size());   	
  	}  	
  }    
  
  // determine borderline nodes
   for (TUNGraph::TEdgeI EI = SoS->BegEI(); EI < SoS->EndEI(); EI++) {
   	
   	/*
   	 int src = EI.GetSrcNId();
   	 int dst = EI.GetDstNId();
   	
   	 if (pnode_info[src].id==pnode_info[dst].id) ;
   	  */
   	
   	 physical_node_info *src = &pnode_info[EI.GetSrcNId()];
   	 physical_node_info *dst = &pnode_info[EI.GetDstNId()];
   	 
   	 if ((src->cs!=-1) && (dst->cs==-1)) {   	 	  
   	 	
   	 	  if (src->id!=dst->id) {
   	 	  	 std::cout << "error inconsistent ids\n";
   	 	  	 exit(2);
   	 	  }

			 SoS_Info[src->id].cs[src->cs].borderline_switch.push_back(EI.GetDstNId());   	   	   	 	
   	 	 //  SoS_Info[src->id].cs[src->cs].incoming_borderline_switch.push_back(physical_node_info(dst->id,src->cs,T_SW,EI.GetDstNId()));   	   	
   	 }
   	   	
   	 if ((src->cs==-1) && (dst->cs!=-1)) {

   	 	  if (src->id!=dst->id) {
   	 	  	 std::cout << "error inconsistent ids\n";
   	 	  	 exit(2);
   	 	  }

				SoS_Info[dst->id].cs[dst->cs].borderline_switch.push_back(EI.GetSrcNId());   		  
   		  // SoS_Info[dst->id].cs[dst->cs].incoming_borderline_switch.push_back(physical_node_info(dst->id,dst->cs,T_SW,EI.GetSrcNId()));   		  
   	 }   
  }	 	       
}
	
void CreateSoS_GraphVis(PUNGraph SoS, std::vector<ID> &SoS_Info,str_physical_structure_parameters &physical, const char *filename) {

  std::ofstream myfile;
  myfile.open (filename);
  
  myfile << "graph G {\n";

  for (int i=0;i<physical.number_of_interaction_domains;i++) {
  	
  	myfile << "  subgraph cluster_ID" << i << " {\n";  	  	
  	myfile << "  style=filled;color=lightgrey;node [style=filled,color=white];\n";		  	
  	myfile << "  label=\"ID " << i << "\";\n";
  	
  	unsigned int switches = SoS_Info[i].sw.size();
  	for (unsigned int n=0;n<switches;n++) { 
  		myfile << "  N" << SoS_Info[i].sw[n] << " [label=\"sw\"]; \n";
  		//SoS_Info[i].sw.pop();
    }   		
  	myfile << "  N" << SoS_Info[i].manager << " [label=\"es\",color=red]; \n";  	
	  myfile << "  }\n";
  	
  	  	
  	for (int j=0;j<SoS_Info[i].number_constituent_systems;j++) {
  		
  			myfile << "  subgraph cluster_CS" << j << "I" << i << " {\n";  	
  			myfile << "  label=\"CS " << j << " I " << i << "\";\n";
  			unsigned int switches = SoS_Info[i].cs[j].sw.size();
  	    for (unsigned int n=0;n<switches;n++) {   			
          myfile << "  N" << SoS_Info[i].cs[j].sw[n] << " [label=\"sw" << SoS_Info[i].cs[j].sw[n] << "\"]; \n";
  		    //SoS_Info[i].cs[j].sw.pop();  	    	
  	    }
  	    unsigned int endsystems = SoS_Info[i].cs[j].es.size();
  	    for (unsigned int n=0;n<endsystems;n++) {   			

					if  (SoS_Info[i].cs[j].es.front() == SoS_Info[i].cs[j].manager) 
						 myfile << "  N" << SoS_Info[i].cs[j].es[n] << " [label=\"es" << SoS_Info[i].cs[j].es[n] <<  "\", color=red]; \n";
 					else
             myfile << "  N" << SoS_Info[i].cs[j].es[n] << " [label=\"es" << SoS_Info[i].cs[j].es[n] << " \"]; \n";
          
  		    //SoS_Info[i].cs[j].es.pop();  	    	
  	    }
			    			  	 	     
		   myfile << "}\n"; 	     
  	}
  }
  
  
  // conntect IDs for each edge in SoS_Graph
  for (TUNGraph::TEdgeI EI = SoS->BegEI(); EI < SoS->EndEI(); EI++) {
    myfile << "N" << EI.GetSrcNId() << "--" << "N" << EI.GetDstNId() << "\n"; 
  }	 	  
  
  
  
  myfile << "}\n"; 	     
  myfile.close();
  	
	
}

	
void print_graph_allocability(std::ofstream &schedfile, PUNGraph G, TIntStrH &name) {

  std::map <int,int> node_id;
  int node_index = 0;

  schedfile << "# Phys. Graph Allocability\n";
  //schedfile  << "router or end system[";    
  
  for (TUNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {

     char level[32];
     char type[32];
     int id;
     int cs;
     int manager;  		

     sscanf(name.GetDat(NI.GetId()).CStr(),"%s %s %d %d %d",type, level, &id, &cs, &manager);  	
 
     if (NI!=G->BegNI()) schedfile  << ",";
 
     if (strcmp(type,"es")==0) 
    		 schedfile << "1";
    	else
	   		 schedfile << "0";
	   		     
      node_id[node_index++] = NI.GetId();   
  }
  
  schedfile  << "]\n";    
}





