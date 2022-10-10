

typedef struct {  	
  	int ss_nodes; 
  	
  	int app_N_Top;
  	int app_M_Top;
  	int app_N;
  	int app_M;
  	
  	int apps;
} str_logical_structure_parameters;


class CS_ident {
	public:
	int id, cs, es, ss, component;	
	CS_ident(int id_ini, int cs_ini, int es_ini, int ss_ini, int component_ini) : id(id_ini), cs(cs_ini), es(es_ini),ss(ss_ini),component(component_ini) {};	
	CS_ident() : id(-1), cs(-1), es(-1), ss(-1), component(-1) {};	
};


class subsystem {
  public:
  std::vector<int> node;
  int id,cs;
  std::vector<CS_ident> depends_on;
  std::vector<int> outgoing_borderlines_nodes; // nodes of other subsystem that receive input from this subsystem  	
  std::vector<int> incoming_borderlines_nodes; // nodes of other subsystem that provide input to this subsystem  	
};	 

class app_info {
 public: 
 std::vector<subsystem> ss;
 std::map<int,int>      node_ss_map;	
};

class application {
	public:
	std::vector<app_info> info;
	std::vector<PNGraph> G;
};


extern std::map<int,int> ss_cs_map;	
	

extern void CreateApps(application &app, str_logical_structure_parameters &logical);
extern void Apps_Vis(application &app);
extern void generate_dag(PNGraph &G,std::vector<int> &ss,int nodes);
	
extern application app;