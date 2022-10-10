
enum node_type { T_SW = 0, T_ES = 1, T_INVALID = 2 };

class physical_node_info {
	public:
  int id;
  int cs;
  node_type t;
  int node_id;  
  physical_node_info(int iid,int ics,node_type it, int nid) : id(iid), cs(ics), t(it), node_id(nid) {};
  physical_node_info() : id(-1), cs(-1), t(T_INVALID), node_id(-1) {};
  //physical_node_info(const physical_node_info &cop) : id(cop.id), cs(cop.cs), t(cop.t) {};
};

class CS {
	public:
  int manager;
  std::vector <int> sw;
  std::vector <int> es;  	  	    	
  //std::vector<int> outgoing_borderline_switch;
  std::vector<int> borderline_switch;
};

class ID {
	public:
  int number_constituent_systems;
  int manager;
  std::vector <int> sw;  	
  std::vector <CS> cs;
};

class physical {
  public:
  PUNGraph G;
  std::vector<ID> id;
  physical() : G(TUNGraph::New()) {};
};


typedef struct str_physical_structure_parameters {

	   // SoS
	   int number_of_interaction_domains;
	   
	   // parameters of an interaction domain
	   int MIN_switches_per_interaction_domain; 
	   int MAX_switches_per_interaction_domain;
	   int MIN_CS_per_interaction_domain; 
	   int MAX_CS_per_interaction_domain;
	   
	   // parameters of a constituent system
	   int CS_MIN_nodes_per_switch;
	   int CS_MAX_nodes_per_switch;
	   int CS_MIN_switches;
	   int CS_MAX_switches;
} physical_structure_parameters;

extern std::map<int,physical_node_info> pnode_info;

extern void CreateSoS(PUNGraph SoS, str_physical_structure_parameters &physical,int topology, int nr_es,int nr_rs_r, int nr_rs_c);
extern void CreateSoS_GraphVis(PUNGraph SoS, std::vector<ID> &SoS_Info,str_physical_structure_parameters &physical, const char *filename);
extern void CreateSoS_Info(PUNGraph SoS, std::vector<ID> &SoS_Info,str_physical_structure_parameters &physical);
extern void CreateConstituentSystem(TIntStrH &name, PUNGraph CS, int MIN_nodes_per_switch, int MAX_nodes_per_switch, int MIN_switches, int MAX_switches, int cs_id, str_physical_structure_parameters &physical,int ID_id,int Topology, int nr_es,int nr_rs_r, int nr_rs_c);
extern void print_graph_allocability(std::ofstream &schedfile, PUNGraph G_CS, TIntStrH &name);
		
