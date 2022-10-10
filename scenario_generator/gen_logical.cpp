#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <assert.h>
#include "stdafx.h"
#include "gen_logical.h"
#include "gen_physical.h"
#include "graph_aux.h"
#include "mxdag.h"
using namespace std;

void MergeIntoNoName(PNGraph TargetG, PNGraph G2, int offset) {
  
  for (TNGraph::TNodeI NI = G2->BegNI(); NI < G2->EndNI(); NI++) {
  	int id = offset+NI.GetId();
  	TargetG->AddNode(id);
  }
  
  //traverse the edges
  for (TNGraph::TEdgeI EI = G2->BegEI(); EI < G2->EndEI(); EI++) {
    TargetG->AddEdge(offset+EI.GetSrcNId(), offset+EI.GetDstNId());    	         
  }
}

//PNGraph: a directed graph;

void generate_dag(PNGraph &G,std::vector<int> &ss,int nodes) {

  int in_degree = 3;  //constraint for in-degree
  int out_degree = 3;  //constraint for out-degree
  int total_messages_reqd = nodes+2;  //constraint for messages

  float f = 0;
  G  = TSnap::GenForestFire(nodes, 0.35 , 0.35);
  //TSnap::DelDegKNodes(G, 3, 3);
  //G  = TSnap::GenRndGnm<PNGraph>(nodes, 3);
  
  //TSnap::GenPrefAttach(nodes, 1);
  //TSnap::DelZeroDegNodes(G);
  //TSnap::AddSelfEdges(G);

/*
  G = TNGraph::New();
  G->AddNode(2);
  G->AddNode(5);
  G->AddNode(32);
  G->AddEdge(2,5);
  G->AddEdge(5,2);
  G->AddEdge(5,32);
  G->AddEdge(2,32);
  G->AddEdge(32,2);
  
*/


// Counting total edges, in-degree, out-degree of each node and total messages in the graph

  int arr1[nodes]; // array storing total edges
  int arr1_in[nodes]; // array storing in-degree
  int arr1_out[nodes]; // array storing out-degree
  int messages = 0; // array storing total messages
  int a = 0;
  int b = 0;
// Initializing the arrays 
  for (int i = 0; i < nodes; i++)  {
  	arr1[i] = 0;
	arr1_in[i] = 0;
  	arr1_out[i] = 0;
  }

// Iterating to consider all nodes
  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
   
  	a = NI.GetId(); 

  	for (int i = nodes-1; i >= 0; i--) {
   
// Considering Incoming edges 
		if (G->IsEdge(i,a)) {      
        		arr1_in[a] +=1;
			arr1[a] +=1;
			messages +=1;
        	} 

// Considering Outgoing edges
		if (G->IsEdge(a,i)) {
        		arr1_out[a] +=1;
			arr1[a] +=1;	
        	}    
    	}
  
  }


// Restricting in & out degree without creating degree zero nodes

// Iterating to consider all nodes
  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
   
  	a = NI.GetId();
	b = 0;

// Constraining In-degree

	while (arr1_in[a] > in_degree && b < nodes) {
		if (a == b) {
    			b +=1 ;	
        	}
		if (G->IsEdge(b,a) && arr1[b] > 1){
        		G->DelEdge(b,a);
			arr1_in[a] -=1;
			arr1_out[b] -=1;
			arr1[b] -=1;
			arr1[a] -=1;
			messages -=1;	
		}
		b += 1;
	} 

// Constraining Out-degree
	b = 0;
	
	while (arr1_out[a] > out_degree && b < nodes) {
		while (a == b) {
			b += 1;
		}
		if (G->IsEdge(a,b) && arr1[b] > 1){
        		G->DelEdge(a,b);
			arr1_out[a] -=1;
			arr1_in[b] -=1;
			arr1[b] -=1;
			arr1[a] -=1;
			messages -=1;	
		}
		b += 1;
	}   
  }
/*
// Making sure graph remains connected

// Iterating to consider all nodes
  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
   
  	a = NI.GetId();
	b = 0;


// Constraining Out-degree
	
	while (a != 0 && arr1_out[a] == 0 && b < nodes) {
		while (a == b) {
			b += 1;
		}

		if (!(G->IsEdge(a,b)) && !(G->IsEdge(b,a)) && arr1_in[b] < in_degree){
			G->AddEdge(a,b);
			arr1_out[a] +=1;
			arr1_in[b] +=1;
			arr1[b] +=1;
			arr1[a] +=1;
			messages +=1;	
		}
		b += 1;
	}   
  }

*/
//Ensuring total messages = no.of nodes

//Adding messages

/*
// Formation of loop
  while (messages < total_messages_reqd) {
	
  	a = rand()%nodes;
  	b = rand()%nodes;
    	
  	while (a == b) {
    		b = rand()%nodes;	
    	}
  	if (a != 0 && !(G->IsEdge(a,b)) && !(G->IsEdge(b,a)) && arr1_in[b] < in_degree && arr1_out[a] < out_degree){
		G->AddEdge(a,b);
		arr1_out[a] +=1;
		arr1_in[b] +=1;
		arr1[b] +=1;
		arr1[a] +=1;
		messages +=1;	
	}	
   
  }
*/

// Iterating to consider all nodes
  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
   
  	a = NI.GetId();
	b = 0;

// Adding message

	while (messages < total_messages_reqd && a>b) {
		
		if (a != 0 && !(G->IsEdge(a,b)) && !(G->IsEdge(b,a)) && arr1_in[b] < in_degree && arr1_out[a] < out_degree){
		G->AddEdge(a,b);
		arr1_out[a] +=1;
		arr1_in[b] +=1;
		arr1[b] +=1;
		arr1[a] +=1;
		messages +=1;	
		}
		b += 1;
	}
  }

//Removing messages
  while (messages > total_messages_reqd) {
	
  	a = rand()%nodes;
  	b = rand()%nodes;

  	while (a == b) {
    		b = rand()%nodes;	
    	}
  	if (G->IsEdge(a,b) && arr1[b] > 1 && arr1[a] > 1){
		G->DelEdge(a,b);
		arr1_out[a] -=1;
		arr1_in[b] -=1;
		arr1[b] -=1;
		arr1[a] -=1;
		messages -=1;	
	}	 
  }


  std::vector< int > root_nodes;
  // Iterating to consider all nodes
  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
   
   	a = NI.GetId();
        b = 0;
// Saving the ids of the root nodes
  	if (arr1_in[a] == 0){
		root_nodes.push_back(a);
	}
  }

  //Add edges between two root nodes
  for (int i=0; i<(root_nodes.size()/3); i++){
	G->AddEdge(root_nodes[i*2],root_nodes[i*2+1]);
	arr1_out[root_nodes[i*2]] +=1;
        arr1_in[root_nodes[i*2+1]] +=1;
        arr1[root_nodes[i*2+1]] +=1;
        arr1[root_nodes[i*2]] +=1;
        messages +=1;  
  }

  //for (int i=0; i<root_nodes.size(); i++){
  //	cout << "Root node id " << root_nodes[i] << " ";
  //}
  //cout << "\n";

  cout<< "Total messages final= "<< messages<< " \n";
  int counter = 0;
  for (int x=0; x<nodes; x++) {
	if (arr1_in[x] == 0){
		counter++;}
  	//cout<< "In degrees_" << x <<": " << arr1_in[x] << "\n"; 
  }
  cout<< "Number of root nodes (nodes without parents): " << counter << "\n";

  for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
  	//int id = offset++;
  	//TargetG->AddNode(id);
  	ss.push_back(NI.GetId());
  }
  
  cout << "khak";
}


void CreateApps(application &app, str_logical_structure_parameters &logical) {
	
   for (int i=0;i<logical.apps;i++) {
   	
   	 int subsystem_index = 0;
   	 int id_offset = 0;	
   	 int id_rnd_src = 0;
   	 int id_rnd_dst = 0;
   	 int skipe_edge;
   	 PNGraph app_highlevel_graph = TSnap::GenRndGnm<PNGraph>(logical.app_N_Top, logical.app_M_Top, true);
		 app.G.push_back(  TNGraph::New() );
		  
		 app.info.push_back( app_info() ); 

     // copy ID nodes
	   for (TNGraph::TNodeI NI = app_highlevel_graph->BegNI(); NI < app_highlevel_graph->EndNI(); NI++) {  	 
		     	  	  	   	  			
  	   	PNGraph subsystem_graph; 
  	   	app.info[i].ss.push_back( subsystem() );
  	    generate_dag(subsystem_graph,app.info[i].ss[subsystem_index].node,logical.ss_nodes);
	   	  	
  	  	for (unsigned int n=0;n<app.info[i].ss[subsystem_index].node.size();n++) 
				  app.info[i].ss[subsystem_index].node[n] += id_offset;

	   	  subsystem_index++;
	   	  	   	 	   	
		    skipe_edge = 1;
		    if ((app.G[i]->GetNodes()>0) && (subsystem_graph->GetNodes()>0)) {		   	
		    	skipe_edge = 0;
			    id_rnd_src = app.G[i]->GetRndNId();	   	  
	   	    id_rnd_dst = subsystem_graph->GetRndNId();	   	  
	   	  }
	   	
	   	  std::cout << "1";
	   	  //subsystem_graph,app.info[i].ss[subsystem_index].offset = id_offset;
		    MergeIntoNoName(app.G[i], subsystem_graph, id_offset );

		    
		    if (!skipe_edge) {
		      app.G[i]->AddEdge(id_rnd_src,id_rnd_dst+id_offset);
		      
		      for (int ss=0;ss<subsystem_index;ss++) 
		      	for (unsigned int x=0;x<app.info[i].ss[ss].node.size();x++) {
		      	  if (app.info[i].ss[ss].node[x] == id_rnd_src) {
		      		 // found other linked subsystem		      				      		 
		      		 if (ss==subsystem_index-1) {
		      		 	  printf("error: cross-subsystem connection within the same subsystem\n");
		      		 	  exit(2);
		      	  }
		      		 app.info[i].ss[subsystem_index-1].incoming_borderlines_nodes.push_back(id_rnd_src); // borderline_node(ss,id_rnd_src)
		      		 app.info[i].ss[ss].outgoing_borderlines_nodes.push_back(id_rnd_dst+id_offset); // borderline_node(subsystem_index-1,id_rnd_dst)
		         	}		      			      			      	
		        }		      		    
		    }
		    
		    id_offset += app.G[i]->GetMxNId();   	   	 		  		    
		    
     }	   	 	       
     
     // establish map
     for (unsigned int ss=0;ss<app.info[i].ss.size() ;ss++) 
       for (unsigned int x=0;x<app.info[i].ss[ss].node.size();x++) {
       	  app.info[i].node_ss_map[ app.info[i].ss[ss].node[x] ]   = ss;
       }
  }
}


void Apps_Vis(application &app) {
  
  char filename[32];
  
  for (unsigned int a=0;a<app.G.size();a++) {


  std::ofstream myfile;  	
  sprintf(filename,"graphs/logical%d.txt",a);	  	
  myfile.open (filename);
  
  myfile << "digraph G {\n";

  for (unsigned int i=0;i<app.info[a].ss.size();i++) {
  	
  	myfile << "  subgraph cluster_ss" << i << " {\n";  	  	
  	//myfile << "  style=filled;color=lightgrey;node [style=filled,color=blue];\n";		  	
  	myfile << "  label=\"Subsystem " << i << "\";\n";
  	
  	unsigned int components =  app.info[a].ss[i].node.size();
  	
  	for (unsigned int n=0;n<components;n++) { 
  		myfile << "  N" << app.info[a].ss[i].node[n] << " [label=\"Service" << i << "/" << n << "|" << app.info[a].ss[i].node[n] << "\"]; \n";
    }   		
	  myfile << "  }\n";  	  	    	
  }
  
  
  // conntect IDs for each edge in SoS_Graph
  
  for (TNGraph::TEdgeI EI = app.G[a]->BegEI(); EI < app.G[a]->EndEI(); EI++) {
    myfile << "N" << EI.GetSrcNId() << "->" << "N" << EI.GetDstNId() << "\n"; 
  }	 	  
   
  myfile << "}\n"; 	     
  myfile.close();	
 
  }
}
	
	

