#include "metascheduler.hpp"

extern Fractal::MsGraph msg;
extern Fractal::Calendar cal;

static int scheduleSequence_nr = 1;
extern int reconvHorizon;

void createCalendar(Fractal::Schedule *schedule, Fractal::Context *CM) {
	//Copy events in CM to the calendar
	for (Fractal::Event E : CM->events)
		cal.addEvent(E);

	//For each event in calendar, update event time
	cal.updateEvents(schedule);

	//Sort the calendar by time of event
	cal.sortCalendar();

	//print contents of calendar
	//cal.printCalendar();
}

void reorderCalendar(Fractal::Event *E, Fractal::Calendar *cal, Fractal::Schedule *schedule) {
	if (cal->getEvents().empty())
		return;

	//copy non-mutually exclusive events in calendar to temporary list
	list <Fractal::Event> temp;
	for (Fractal::Event event : cal->getEvents()) {
		if (event.target != E->target || event.eventType != E->eventType) {
			temp.push_back(event);
		}
	}

	//remove all objects from previous calendar and copy contents of temporary calendar to previous
	cal->clearCalendar();
	for (Fractal::Event event : temp) {
		cal->addEvent(event);
	}

	//cal->printCalendar();

	//reorder mutually exclusive calendar
	cal->updateEvents(schedule);

	//Sort the calendar by time of event
	cal->sortCalendar();
	//delete &temp;
}

Fractal::Model apply_event(Fractal::Event* E, Fractal::Model model) {
	//modify AM, PM based on event E
	switch (E->eventType) {
	case SLACK:
		model.job(E->target).wcet_fullspeed = E->value;
		break;

	case FAULT:
		model.remove_node(model.node(E->target).id);
		break;
	}
	return model;
}

void metaSchedule(Fractal::Model model, Fractal::Calendar cal,
		Fractal::MsgNode *prev) {
	//end recursion
	if (cal.getEvents().empty())
		return;

	//Take earliest event and remove from calendar
	Fractal::Event E = cal.getEvents().front();
	cal.removeEvent(E.ID);

	//recursion
	metaSchedule(model, cal, prev);

	//apply event E to change AM, PM
	Fractal::Model newModel = apply_event(&E, model);

	//get time unit for re-convergence horizon
	const int reconv = reconvHorizon;

	//set reconvergence window for re-scheduling
	Fractal::Horizon *horizon = new Fractal::Horizon(E.eventTime, E.eventTime+reconv);

	//initialize crash config needed by the GA
	Fractal::scheduler_config crashconfig = {100, 100, 0.1, 0.9, 1.0};

	//auto t1 = high_resolution_clock::now();
	//invoke GA to obtain root schedule
	Fractal::Scheduler activeScheduler(&newModel, &crashconfig, horizon);
	activeScheduler.evolve();

	//if (mpi_rank == 0) {
		Fractal::Schedule *Schedule = new Fractal::Schedule(activeScheduler.best());

		if (Schedule->makespan()>prev->getSchedule()->makespan())
			Schedule = prev->getSchedule();

	/*auto t2 = high_resolution_clock::now();

	 //Getting number of milliseconds as an integer.
	    auto ms_int = duration_cast<milliseconds>(t2 - t1);

	    cout << "Time to compute New Schedule: " << ms_int.count() << "ms\n";*/

	//Set new schedule ID
	Schedule->set_uid(scheduleSequence_nr++);
	//Export new schedule to json file.
	Schedule->export_json("results/");

	//create new node and add key (schedule)
	Fractal::MsgNode *Node = new Fractal::MsgNode(Schedule->uid(), Schedule, E);
	prev->addChild(Node);

/*	cout << "New Node: " << Node->getID() << " : " << "Jobs: "
			<< Node->getSchedule()->jobs()[0].wcet_fullspeed << ", "
			<< Node->getSchedule()->jobs()[1].wcet_fullspeed << ", "
			<< Node->getSchedule()->jobs()[2].wcet_fullspeed << " : "
			<< " Makespan: " << Node->getSchedule()->makespan() << endl << endl;*/

	cout << "New Node: " << Node->getID() << " : "
			<< " Makespan: " << Node->getSchedule()->makespan() << endl << endl;

	//reorder calendar of mutually exclusive events of E
	reorderCalendar(&E, &cal, Schedule);
	Node->saveCalendar(&cal);

	//If node is not in multi-schedule graph, add to MSG and meta-schedule
	if (!msg.dejavu(Node)){
		//add node to multi-schedule graph
		msg.addNode(Node);

		//recursion
		metaSchedule(newModel, cal, Node);}
	//}
}

void Fractal::createMSG(Fractal::Model *model, string path, Fractal::Context *context) {
	//Set boundaries of reconvergence horizon to numerical limits to compute the base schedule
	Fractal::Horizon horizon(numeric_limits<int>::min(), numeric_limits<int>::max());

	//initialize crash config needed by the GA
	//Fractal::scheduler_config crashconfig = {2000, 100, 0.01, 0.4, 1.0, path};
	Fractal::scheduler_config crashconfig = {2000, 300, 0.01, 0.4, 1.0, path};
        //800 3000
	model->horizonStart = 0;
	model->horizonEnd = 999;
	auto t1 = high_resolution_clock::now();
	//invoke GA to obtain root schedule
	Fractal::Scheduler activeScheduler(model, &crashconfig, &horizon);
	activeScheduler.evolve();

	Fractal::Schedule *rootSchedule = new Fractal::Schedule(activeScheduler.best());

	auto t2 = high_resolution_clock::now();
	// Getting number of milliseconds as an integer.
	auto ms_int = duration_cast<milliseconds>(t2 - t1);
	cout << "Time to compute Base Schedule: " << ms_int.count() << "ms\n";

	//Set the ID of the base schedule
	rootSchedule->set_uid(0);
	//Export base schedule to a json file.
	rootSchedule->export_json(path);

	Event ne (-1);
	//create root node, add root schedule to root node
	Fractal::MsgNode *rootNode = new Fractal::MsgNode(rootSchedule->uid(),
			rootSchedule, ne);

/*	cout << "Root Node: " << rootNode->getID() << " : " << "Jobs: "
				<< rootNode->getSchedule()->jobs()[0].wcet_fullspeed << ", "
				<< rootNode->getSchedule()->jobs()[1].wcet_fullspeed << ", "
				<< rootNode->getSchedule()->jobs()[2].wcet_fullspeed << ", "
				<< "Makespan: " << rootNode->getSchedule()->makespan() << endl << endl;*/

	cout << "Root Node: " << rootNode->getID() << " : "
			<< " Makespan: " << rootNode->getSchedule()->makespan() << endl << endl;

	//Establish calendar of events from CM
	createCalendar(rootSchedule, context);

	//Save calendar in the node. This will be needed to reduce state space explosion.
	rootNode->saveCalendar(&cal);

	//add root node to multi-schedule graph
	msg.addNode(rootNode);

	//invoke meta-scheduler
	metaSchedule(*model, cal, rootNode);

	//if (mpi_rank == 0) {
		cout << "size of MsG: " << msg.numOfNodes() << endl;
		//output file for visualization
		msg.exportMSG();
	//}
	//MPI_Finalize();
}
