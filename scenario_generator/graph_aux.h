


extern void add_to_graph(PUNGraph subgraph, std::vector<int> &IDs);
extern void add_to_graph(PUNGraph subgraph, PUNGraph overallgraph, std::vector<int> &IDs);
extern void add_to_graph(PNGraph subgraph, std::vector<int> &IDs);
extern void add_to_graph(PNGraph subgraph, PNGraph overallgraph, std::vector<int> &IDs);
extern void print_graph(std::ofstream &schedfile, PNGraph G);
extern void print_graph(std::ofstream &schedfile, PUNGraph G);
extern void print_nodes_in_vector(std::ofstream &schedfile, PUNGraph &G, std::vector<int> &v);
extern void print_nodes_in_vector(std::ofstream &schedfile, PNGraph &G, std::vector<int> &v);
extern void cout_print_nodes(PUNGraph &G);
extern void cout_print_nodes(PNGraph &G);
extern void cout_print_nodes(std::vector<int> &v);
extern void print_CPLEX_structures(std::ofstream &schedfile, PUNGraph &G_CS, PNGraph &G_SS, int period,int hop_transmit, int execution, std::vector<ID> &SoS_Info, int app);
extern void print_CPLEX_structures_to_json(std::ofstream &schedfile, PUNGraph &G_CS, PNGraph &G_SS, int period,int hop_transmit, int execution, std::vector<ID> &SoS_Info, int app, TIntStrH &name);
	


