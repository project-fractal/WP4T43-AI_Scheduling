#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <sstream>
#include <string>
#include <bits/stdc++.h>
#include "stdafx.h"
#include "gen_physical.h"
#include "gen_logical.h"
#include "graph_aux.h"

#undef NDEBUG 
#include <assert.h>

#define CONSTANT_PERIOD 200//1100
#define CONSTANT_HOPTRANSMISSION 2
#define CONSTANT_EXECUTION 3+rand()%7   //3
using namespace std;

application app;
physical    phy;

std::ofstream schedfile;

std::map<int,int> ss_cs_map;	
	
void schedule_ID(int id, std::vector<int> &switch_path) {
  // now the timing should be allocated
  
}


void schedule_id_comunication(int app_id, int src_ss, int dst_ss) {

  std::vector<ID>     logical_exit_nodes;
  std::map<int, int>  hops_to_exit_nodes;  // maximum over exit nodes of number of hops to the exit node

  int src_id = app.info[app_id].ss[src_ss].id;
  int src_cs = app.info[app_id].ss[src_ss].cs;
  
  int dst_id = app.info[app_id].ss[dst_ss].id;
  int dst_cs = app.info[app_id].ss[dst_ss].cs;
  	  
 
   
  int min_path = -1;
  struct { int src,  dst; } endpoints;
  		  	  		
  		
  // we look now for the shortest path between a node from <outgoing_borderlines_nodes> to <incoming_borderlines_nodes>
  for (unsigned int o=0;o<phy.id[src_id].cs[src_cs].borderline_switch.size();o++) 
    for (unsigned int i=0;i<phy.id[dst_id].cs[dst_cs].borderline_switch.size();i++) {

      int SrcNId = app.info[app_id].ss[src_ss].outgoing_borderlines_nodes[o];
      int DstNId = app.info[app_id].ss[dst_ss].incoming_borderlines_nodes[i];

      TBreathFS<PUNGraph> BFS(phy.G);
      BFS.DoBfs(SrcNId, true, true, DstNId, TInt::Mx);
      	
      if ((min_path == -1) || (min_path>BFS.GetHops(SrcNId, DstNId))) {
      	min_path = BFS.GetHops(SrcNId, DstNId);
      	endpoints.src = SrcNId;
      	endpoints.dst = DstNId;
      }
      	
 
    }

   assert(min_path != -1);
  
   // get minimum path
   TIntV NIdV;
   
   TBreathFS<PUNGraph> BFS(phy.G);
   BFS.DoBfs(endpoints.src, true, true, endpoints.dst, TInt::Mx);   	
   BFS.GetVisitedNIdV(NIdV);
   
   int current_ID = -1;
   std::vector<int> NId; 
   
   for (int n=0;n<NIdV.Len();n++) {
   	
   	  if (pnode_info[NIdV[n]].id!=current_ID) {

				 if (current_ID != -1) {   			
     			 schedule_ID(current_ID,NId);  	
     		 }
   		  	  
   	  	 NId.clear();
   	  	 
   	  	 // start new 
   	  	 current_ID = pnode_info[NIdV[n]].id;
   	  	 
   	  }   	  
   	  NId.push_back(NIdV[n]);   	     	  
   }  	    
}



void schedule_subsystem(int app_id, int ss) {
	
	// subgraphs of application and platform
  PUNGraph G_CS = TUNGraph::New();
  PNGraph  G_SS = TNGraph::New();
	
	std::vector<int> SS_source_nodes;
	std::vector<int> SS_sink_nodes;
	
	int id         = app.info[app_id].ss[ss].id;
	int cs         = app.info[app_id].ss[ss].cs;

  
  add_to_graph(G_SS,app.info[app_id].ss[ss].node);  
  add_to_graph(G_SS,app.G[app_id],app.info[app_id].ss[ss].outgoing_borderlines_nodes);   // add outgoing borderline nodes and edges to graph
  

  add_to_graph(G_SS,app.G[app_id],app.info[app_id].ss[ss].incoming_borderlines_nodes);   // add incoming borderline nodes and edges to graph

  

  
schedfile << "[" << ss << "," << cs << "]\n";
  ss_cs_map[ss]=cs;

  
  add_to_graph(G_CS,phy.id[id].cs[cs].es);  
  add_to_graph(G_CS,phy.id[id].cs[cs].sw);
  add_to_graph(G_CS,phy.G,phy.id[id].cs[cs].borderline_switch); // asd

 


schedfile << "[" <<phy.id[id].cs[cs].borderline_switch.size() << "," << app.info[app_id].ss[ss].incoming_borderlines_nodes.size() << "," << app.info[app_id].ss[ss].outgoing_borderlines_nodes.size() << "]" << "\n";


  //invoke aux function here
  print_CPLEX_structures(schedfile,G_CS,G_SS, CONSTANT_PERIOD,CONSTANT_HOPTRANSMISSION,CONSTANT_EXECUTION, phy.id,app_id); // qwe


  
  

  print_nodes_in_vector(schedfile, G_CS, phy.id[id].cs[cs].es);


 schedfile  << "[";
  for (TNGraph::TEdgeI EI = G_SS->BegEI(); EI < G_SS->EndEI(); EI++) {
    if (EI!=G_SS->BegEI()) schedfile << ",";
    schedfile << "1";       
  }
   schedfile  << "]\n\n";

}


void schedule_app(int app_id) {
    
  // map each application subsystem to a random CS    	
  std::vector<int>        unscheduled_subsystem;  	
  std::vector<CS_ident>   CS;  	
  std::map<int, CS_ident> node;
    	    	
	for (unsigned int i=0;i<phy.id.size();i++) 
	  for (unsigned int c=0;c<phy.id[i].cs.size();c++)  
      CS.push_back(CS_ident(i,c,0,0,0));
  
  for (unsigned int i=0;i<app.info[app_id].ss.size();i++) {
  		
  	unscheduled_subsystem.push_back(i);
  		  	    		
  	 int target = rand() % CS.size();
  	 
  	 if (CS.size() == 0) { printf("out of CS\n");exit(1); }
  		  	 
  	 app.info[app_id].ss[i].id = CS[target].id;
  	 app.info[app_id].ss[i].cs = CS[target].cs;

     ss_cs_map[i]=CS[target].cs; // redundant? already set during mapping, would be too late here

  	 
  	 for (unsigned int n=0;n<app.info[app_id].ss[i].node.size();n++)  {
		
  	   node[app.info[app_id].ss[i].node[n]] = CS_ident(CS[target].id,CS[target].cs,0, i /* subsystem*/, n /* service */ );
  	 }
  	   	     	  	 
  	 CS.erase(CS.begin() + target);
  }       

  // determines dependencies   
  for (TNGraph::TEdgeI EI = app.G[app_id]->BegEI(); EI < app.G[app_id]->EndEI(); EI++) {
  	
  	int src = EI.GetSrcNId();
  	int dst = EI.GetDstNId();  	
  	
  	if ((node[src].id != node[dst].id) || (node[src].cs!=node[dst].cs)) {
  		
  		// dependency between two services, which are allocated to different CS  		  		
  		  		
  		app.info[app_id].ss[node[dst].ss].depends_on.push_back(node[src]);
  		
  	}  	  	      	  
  }  

  // print information  
  for (unsigned int i=0;i<app.info[app_id].ss.size();i++) {
  	  printf("subsystem %d [ID=%d,CS=%d], depends on ",i,app.info[app_id].ss[i].id,app.info[app_id].ss[i].cs);
  	  for (unsigned int n=0;n<app.info[app_id].ss[i].depends_on.size();n++) {
  	  	CS_ident *ss = &(app.info[app_id].ss[i].depends_on[n]);
  	    printf("SS%d [ID=%d,CS=%d]",ss->ss,ss->id,ss->cs);
  	  }
  	  printf("\n");
  }


  // schedule application subsystems
  while ( unscheduled_subsystem.size() > 0 ) {

     int scheduled_subsytems = 0;
     
		 for (unsigned int i=0;i<unscheduled_subsystem.size();i++) {
		   if (app.info[app_id].ss[unscheduled_subsystem[i]].depends_on.size() == 0) {
		   	  // schedule it, no dependencies
		   	  schedule_subsystem(app_id, unscheduled_subsystem[i]);
		   	  ++scheduled_subsytems;   	  
		   			   	  
		   	  // now scheduled, remove from other SS dependency vectors
		   	  for (unsigned  int j=0;j<unscheduled_subsystem.size();j++) {		   	  	
		   	  	std::vector<CS_ident> *dep = &(app.info[app_id].ss[unscheduled_subsystem[j]].depends_on);		   	  	
		        for (unsigned int n=0;n<dep->size();n++) {
		        	 if ((*dep)[n].ss == unscheduled_subsystem[i]) {
								 //printf("erasing SS%d in SS%d (total %d)\n",(*dep)[n].ss,j,dep->size());		        	   
		    		    
		   		   	   // schedule now the communication in the interaction domain
		   		   	   schedule_id_comunication(app_id,(*dep)[n].ss, unscheduled_subsystem[j]);
		     				
		        	   dep->erase(dep->begin() + n);			        	  
		        	   break;
  		         }		        	 	 
		        }
		   	  }
		   	  
		   	  // remove the scheduled subsytem
		   	  unscheduled_subsystem.erase(unscheduled_subsystem.begin() + i);		   	  
  		  }
		 }
		   	
		 if (scheduled_subsytems==0) {
		 	 printf("circular dependency, stopping scheduling: ");
		 	 for (unsigned int i=0;i<unscheduled_subsystem.size();i++) printf("%d,",unscheduled_subsystem[i]);		 	 
		 	 exit(2);
		}		  		
  }   
}



void generate_simple_graph(int n_jobs, int index, int T,int nr_es,int nr_rs_c,int nr_rs_r, int min_es,int max_es) {
  cout << " function simple graph" << endl;
  char fname[128];
  sprintf(fname,"scenarios/example_N%d.json",index); 
  schedfile.open(fname);
  
  // generate one CS
  str_physical_structure_parameters physical;  
    
  TIntStrH logical_name;
  TIntStrH physical_name;

  PUNGraph CS = TUNGraph::New();
  PNGraph  SS = TNGraph::New();

  std::vector<int> nodes;
  int id;
  int i = 0;
 

  // Parameters of Physical Model   
  physical.CS_MIN_nodes_per_switch = min_es; 
  physical.CS_MAX_nodes_per_switch = max_es;
  physical.CS_MIN_switches         = nr_rs_r; // number of rows;
  physical.CS_MAX_switches         = nr_rs_c; // number of columns);
 
  cout<<index;

  cout << "1.1 " << endl;
  CreateConstituentSystem(physical_name, CS, physical.CS_MIN_nodes_per_switch,physical.CS_MAX_nodes_per_switch,physical.CS_MIN_switches,physical.CS_MAX_switches, 0, physical,0,T, nr_es, nr_rs_r,nr_rs_c);
  cout << "1.2 " << endl;
  
  // Parameters of Logical Model
  str_logical_structure_parameters logical;
  logical.ss_nodes = n_jobs;

  generate_dag(SS,nodes,logical.ss_nodes);
    
  cout << "1.3 " << endl;  
    
  // Generate Constant Files for CPLEX
  print_CPLEX_structures_to_json(schedfile,CS,SS, CONSTANT_PERIOD,CONSTANT_HOPTRANSMISSION,CONSTANT_EXECUTION, phy.id,0, physical_name);
  cout << " 1.4 " << endl;
  
  // Generate Graphics
 
  id = 0;
  for (TNGraph::TNodeI NI = SS->BegNI(); NI < SS->EndNI(); NI++) {
    char name_string[20];
    sprintf(name_string,"%d",id);
    logical_name.AddDat(NI.GetId())=name_string;
    id++;
  }
 
  
  id = 0;
  for (TUNGraph::TNodeI NI = CS->BegNI(); NI < CS->EndNI(); NI++) {
    char name_string[128];
    char level[32];
    char type[32];
    int Nid;
    int cs;
    int manager;  		

    sscanf(physical_name.GetDat(NI.GetId()).CStr(),"%s %s %d %d %d",type, level, &Nid, &cs, &manager);  	      	  
    sprintf(name_string,"%d %s",id,type);
    physical_name.AddDat(NI.GetId())=name_string;  	
    id++;
  }
 
  sprintf(fname,"graphs/logical_N%d.png",index);  //Remove _graphs
  TSnap::DrawGViz<PNGraph>(SS, gvlDot, fname, "", logical_name);
  	
  sprintf(fname,"graphs/physical_N%d.png",index);  //Remove _graphs
  TSnap::DrawGViz<PUNGraph>(CS, gvlDot, fname, "", physical_name);
  cout << "finish" << endl;
  schedfile.close(); 
}



int main(int argc, char* argv[]) {
  int nr_nodes;
  int nr_scenerios;
  srand (time(NULL)); 
  int topology;
  int nr_es;
  int nr_rs_c;
  int nr_rs_r;
  int min_es;
  int max_es;
  
  
  ifstream f("Parameters.txt");
  string line;
  

  int line_num = 0;
  while(getline(f,line)) {
   
    line_num++;
    
    
    if (line_num == 1)  {
      stringstream(line) >> nr_nodes;
    }
    
    else if (line_num == 2)  {
      stringstream(line) >> nr_scenerios;
    }
    
    else if (line_num == 3)  {  
// converting all to lower case and removing extra spaces 
      transform(line.begin(), line.end(), line.begin(), ::tolower); 
      line.erase(remove(line.begin(), line.end(), ' '), line.end());
      std::cout<<"LINEEEE ="<< line << endl;
      if (line=="grid")  {
        topology=0;
        
    }
     else if (line=="bus")  {
        topology=1;
        
    }
    else if (line=="manual")  {
        topology=2;
        
    }
    else {
       cerr<< "topology not found. Input 'Bus'or 'Grid'" <<  endl;   
    }
    }
    
    else if (line_num == 4)  {
      stringstream(line) >> nr_es;
    }
    else if (line_num == 5)  {
      stringstream(line) >> min_es;
    }
    else if (line_num == 6)  {
      stringstream(line) >> max_es;
    }
    
    else if (line_num == 7)  {
      stringstream(line) >> nr_rs_c;
    }
   else if (line_num == 8)  {
      stringstream(line) >> nr_rs_r;
    }

  }
   std::cout<<"Nodes = "<< nr_nodes << endl;
   std::cout<<"No of Scenerios = "<< nr_scenerios << endl;

   std::cout<<"Topology ="<< topology << endl;
   std::cout<<"No of es = "<< nr_es << endl;
   std::cout<<"No of rs_c = "<< nr_rs_c << endl;
   std::cout<<"No of rs_r = "<< nr_rs_r << endl;
 
   for (int i=1;i<=nr_scenerios;i++ ) {  	
     generate_simple_graph(nr_nodes,i,topology,nr_es,nr_rs_c,nr_rs_r,min_es,max_es);
     
   }


  // *** PHYSICAL 
  
   str_physical_structure_parameters physical;  
  physical.number_of_interaction_domains = 1;
	
	// parameters of an interaction domain
	physical.MIN_switches_per_interaction_domain = 0;
	physical.MAX_switches_per_interaction_domain = 2;
	physical.MIN_CS_per_interaction_domain       = 10;
	physical.MAX_CS_per_interaction_domain       = 10;
	   
	// parameters of a constituent system
	physical.CS_MIN_nodes_per_switch = 0;
	physical.CS_MAX_nodes_per_switch = 2;
	physical.CS_MIN_switches         = 10;
	physical.CS_MAX_switches         = 10;
	
	   
 CreateSoS(phy.G,physical,topology,nr_es, nr_rs_r,  nr_rs_c);  
  CreateSoS_Info(phy.G,phy.id,physical);  
  CreateSoS_GraphVis(phy.G,phy.id,physical, (char*) "example_graphs/physical.dot");  
  
  
  
  // Logical
  str_logical_structure_parameters logical;
  logical.ss_nodes = 6;
  	
  logical.app_N_Top    = 5;
  logical.app_M_Top    = 6;
  logical.app_N        = 5;
  logical.app_M        = 6;
  	
  logical.apps         = 5;
  	    
  
  CreateApps(app,logical);
  Apps_Vis(app);
 


  // schedule
    	  
  schedfile.open ("example_graphs/schedule.txt");
     
  schedule_app(0);
  
  schedfile.close();
  
  
  return 0;
}



	





