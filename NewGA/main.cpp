#include <iostream>
#include <sstream>
#include <string>
#include "utils/json.hpp"
#include "json_tools.hpp"
#include "model.hpp"
#include "metascheduler.hpp"
#include "context.hpp"
#include "scheduler.hpp"

using json = nlohmann::json;
using namespace std;
extern vector<float> makespans;
int reconvHorizon;
//int mpi_tasks, mpi_rank;

Fractal::MsGraph msg;
Fractal::Calendar cal;

string getFileName(const string &s) {
	char sep = '/';
	char dot = '.';

//#ifdef _WIN32
//    sep = '\\';
//#endif
	size_t i = s.rfind(sep, s.length());
	size_t j = s.rfind(dot, s.length());
	if (i != string::npos) {
		return (s.substr(i + 1, j - i - 1));
	}
	return ("");
}

int main(int argc, char **argv) {
    string inputModel, contextModel, line;
    ifstream f("Parameters.txt");

	int line_num = 1;
	while (getline(f, line)) {
		stringstream g (line);

		if (line_num == 1)
			g >> contextModel;

		if (line_num == 2) {
			g >> reconvHorizon;
		}

		line_num++;
	}

	/* Variable to store application model*/
	json app;
	json ctx;
	if (argc != 2) {
		printf("Usage: scheduler <application>\n");
		exit(EXIT_FAILURE);
	} else {
		app = ludwig::read_json(argv[1]);
	}	

	//app = ludwig::read_json(inputModel);
	ctx = ludwig::read_json(contextModel);

	Fractal::Model model(app);
	Fractal::Context context (ctx);
	auto t1 = high_resolution_clock::now();
	string str(argv[1]);	
	RemoveWordFromLine(str, ".json");
	
	//When need a whole dataset

	//string str2 = str.substr(0,54);
	////string str3 = str.substr(96,str2.size());
	//string str3 = str.substr(76,str2.size());
	//cout << "  Creating schedule  " << endl;
	//string input_name = str2 + "/results1/" + str3;
	////string input_name = "results1/" + str;
	////cout << input_name;

	// When need an individual result	
	cout << " Creating schedule " << endl;
	string input_name = "individual_results/" + str;
	cout << input_name;

	Fractal::createMSG(&model, input_name, &context);

	auto t2 = high_resolution_clock::now();
	// Getting number of milliseconds as an integer.
	auto ms_int = duration_cast<milliseconds>(t2 - t1);
	cout << "Time to compute MSG: " << ms_int.count()/1000 << "s\n";
	//ofstream file("/vectors/" + str + ".txt");	
	//for(int i=0;i<makespans.size();++i){
	//	file<<makespans[i]<<endl;
	//}
	//file.close();
	return 0;
}
