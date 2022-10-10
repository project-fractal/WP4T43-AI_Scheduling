#include "scheduler.hpp"

#include "utils/mlh.hpp"
#include <iostream>
#include <cmath>
#include <ga/GARealGenome.C>
#include <ga/GARealGenome.h>
#include <ga/GAAllele.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <galib/ga/ga.h>
#include <galib/ga/std_stream.h>
vector<float> makespans;

void RemoveWordFromLine(std::string &line, const std::string &word)
{
  auto n = line.find(word);
  if (n != std::string::npos)
  {
    line.erase(n, word.length());
  }
}

/**
 * Scheduler - constructs a new scheduler
 * @*model: a pointer to a Model object
 * @*config: a struct containing the parameters for the genetic algorithm
 *
 * Constructs a new scheduler and calls FractalScheduler::Init to
 * build the model for the genetic algorithm. If schedule is not null
 * it is used to fix certain genes to obtain semi-identical results.
 */
Fractal::Scheduler::Scheduler(Fractal::Model *model,
		Fractal::scheduler_config *config, Fractal::Horizon* horizon) :
		model_(model), popsz_(config->popsz), ngen_(config->ngen), pmut_(
				config->pmut), pcross_(config->pcross), pconv_(config->pconv), dir_(config->dir), horizon_(*horizon){

	init();
}

/**
 * ~Scheduler() - cleans up the schedulers mess
 */
Fractal::Scheduler::~Scheduler() {
	delete ga_;
	delete data_;
}

/**
 * evolve() - evolves the genetic algorithm
 *
 * This function is a wrapper around GABase::evolve.
 */
void Fractal::Scheduler::evolve() {
	ga_->evolve();
}

/**
 * best() - the best individual as schedule
 *
 * Return: the schedule represented by the fittest genome
 */
Fractal::Schedule Fractal::Scheduler::best() {
	const GARealGenome &g = GA_CAST(ga_->statistics().bestIndividual());
	return __genome_to_schedule(g);
}

/**
 * init() - builds the genetic model
 *
 * This methods builds the prototype of a GARealGenome to use within
 * the genetic algorithm. It further sets all parameters of the
 * genetic algorithm, like recombination and mutation probabilities,
 * population size, recombination methods and termination criteria.
 */
//void Fractal::Scheduler::init(Fractal::Horizon* horizon) {
void Fractal::Scheduler::init() {
	//const int kNcs = model_->nschemes();
	//const int kNnodes = model_->nnodes();
	const int kNjobs = model_->njobs();
	const int kNmsgs = model_->nmsgs();
	//const int kNmsgs = model_->nmsgs();
	//const int kMaxNRoutes = model_->max_routes();
	const int kNorders = model_->norders();
	//const int kNfreq = model_->nfrequencies();

	/* Allele set-array for our genome. It will hold all allele sets
	 needed for specific genes. They are ordered and
	 FractalScheduler::Fitness relies on that. */
	GAAlleleSetArray<float> alleles;

	/* Allocation is expressed with one gene per job. A gene can attain
	 all numbers (nodes) that are mentioned in the jobs
	 can_run_on-field. If this field is empty it can attain all
	 computing nodes. */
	/*To compute subsequent schedules, a new genome is constructed for jobs
		  within the reconvergence horizon.*/

	vector <Fractal::Job> pinned_ = {};
		for (auto& j : model_->jobs()){
			if (j.start_time>horizon_.start && j.start_time<horizon_.end) {
				alleles.add(Fractal::__vector_to_alleleset(j.can_run_on));
			}
			else{
				pinned_.push_back(j);
				alleles.add(Fractal::__atomic_to_alleleset(j.runs_on));
			}
		}

	/* We have to find an ordering for concurrent messages. Since the
	 model knows all possible orderings that maintain the precedence
	 relations, we can enumerate these orderings. If m is the number
	 of messages: 0<=g<m! */
	//GAAlleleSet<float> ordering(0, kNorders - 1, 1);

	//alleles.add(ordering);

	/* Allele set for possible routes. A gene bound to this allele set
	 is a natural number in [0,kMaxNRoutes). To speed things a little
	 up, we can set the number of available routes to a lower number,
	 allowing only the n shortest routes. We need one gene for each
	 message. */
	GAAlleleSet<float> routes(0, 4, 1);
	for (auto m : model_->msgs()) {
		if (m.injection_time>horizon_.start && m.arrival_time<horizon_.end) {
			alleles.add(routes);
		}
		else {
			//pinned_m.push_back(m);
			alleles.add(Fractal::__atomic_to_alleleset(m.route_idx));
		}
	}

	/* Allele set for possible routes. A gene bound to this allele set
	 is a natural number in [0,kMaxNRoutes). To speed things a little
	 up, we can set the number of available routes to a lower number,
	 allowing only the n shortest routes. We need one gene for each
	 message. */
	/*GAAlleleSet<float> routes(0, min(4, kMaxNRoutes - 1), 1);

	for (int msg = 0; msg < kNmsgs; ++msg)
		alleles.add(routes);

	 Allele for available compression schemes. A gene bound to this
	 allele set is a natrual number in [0,kNcs). We need one gene for
	 each message.
	GAAlleleSet<float> compressors(0, kNcs - 1, 1);

	for (int msg = 0; msg < kNmsgs; ++msg)
		alleles.add(compressors);

	 Allele for available frequencies. A gene bound to this allele set
	 is a natural number in [0, kNfre), meaning the corrsponding job
	 will run at the indexed frequency.
	GAAlleleSet<float> frequencies(0, kNfreq - 1, 1);

	for (int job = 0; job < kNjobs; ++job)
		alleles.add(frequencies);*/

	/* Allele for job selection. A gene bound to this allele set
	 is a natural number in [0, kNjobs), meaning activated jobs compete for
	 selection based on this gene. Each job has a corresponding gene.*/

	//GAAlleleSet<float> order(0, (kNjobs - 1)*2, 1);

	//for (int job = 0; job < kNjobs; ++job)
	//		alleles.add(order);

	GAAlleleSet<float> order(0, kNjobs - 1, 1);
	for (auto& j : model_->jobs()){
		if (j.start_time>horizon_.start && (j.start_time+j.wcet_fullspeed)<horizon_.end) {
			alleles.add(order);
		}
		else {
			alleles.add (Fractal::__atomic_to_alleleset(j.weight));
		}
	}

	GAAlleleSet<float> mw (0, kNmsgs-1, 1);
	for (auto m : model_->msgs()) {
		if (m.injection_time>horizon_.start && m.arrival_time<horizon_.end) {
			alleles.add(mw);
		}
	}	

	/* All genomes are cloned from this prototype. It will prodoce one
	 gene for each item in the allele set-array. The second parameter
	 is the objective function used to evaluate the genomes
	 fitness. */
	GARealGenome prototype(alleles, objective);

	/* Set the default crossover operation to perform a 1-point
	 crossover. This is not really needed, since it is the built-in
	 default. But if you want to change it, do it here. */
	prototype.crossover(GARealGenome::OnePointCrossover);

	/* Make the model (and possibly other data) available to the genome
	 and all static functions deadling with it. */
	data_ = new user_data(model_, horizon_);
	data_->pinned = pinned_;
	prototype.userData(data_);

	/* Create a new genetic algorithm using the prototype genome we have
	 just created. */
	ga_ = new GASteadyStateGA(prototype);

	/*Define the path of the statistics */
	RemoveWordFromLine(dir_, "/work/ws-tmp/cl603105-new_workspace/SNAP_and_Gen/NewGA/results1/");
	string stats = "/work/ws-tmp/cl603105-new_workspace/SNAP_and_Gen/NewGA/results1/statistics/" + dir_ + ".dat";	
	//string stats = dir_ + ".dat";
	const char * c  = stats.c_str();
	/* Set all the parameters we passed to the scheduler via the
	 main-function. */
	ga_->populationSize(popsz_);
	ga_->nGenerations(ngen_);
	ga_->pMutation(pmut_);
	ga_->pCrossover(pcross_);
	ga_->pConvergence(pconv_);
	ga_->nConvergence(ngen_);
	ga_->pReplacement(0.25);
	ga_->scoreFrequency(1);
	ga_->flushFrequency(1);
	ga_->scoreFilename(c);
	//Uncomment this later
	ga_->selectScores(GAStatistics::Maximum|GAStatistics::Minimum|GAStatistics::Mean);
	//ga_->mpi_rank(mpi_rank);
	//ga_->mpi_tasks(mpi_tasks);

	/* Make the evolution stop after convergence specified in pconv_ is
	 reached. Comment this line if you want a fixed number of
	 iterations. */
	// ga_->terminator (GAGeneticAlgorithm::TerminateUponConvergence);
	/* We want to minimize the genomes fitness (makespan or power) */
	ga_->minimize();
}

/**
 * objective() - objective function for the (meta-)scheduler
 * @g: the genome to evaluate
 *
 * This function takes a genome and transforms it into schedule. If
 * the schedule is valid, its power consumption is returned. Otherwise
 * the function returns inf.
 *
 * Return: the genomes fitness
 */
float Fractal::Scheduler::objective(GAGenome &g) {
	/* get user data */
	user_data *data = (user_data*) g.userData();

	/* transform genome into schedule */
	Fractal::Schedule schedule = __genome_to_schedule(g);

	if (!schedule.is_valid()) {
		cout << "Invalid" << "\n";
		return INFINITY;
		}
	else {
		int actualMakespan = 0;
		int maxMakespan =0, minMakespan=0;
		float energy = 0.0;

		for (auto &job : data->model->jobs()) {
			if (job.start_time>data->horizon_.start && job.start_time<data->horizon_.end) {
			int finish_time = job.start_time + job.wcet_with_compression; //job.wcet_at_freq;

			/* update the makespan */
			maxMakespan = max(maxMakespan, finish_time);
			minMakespan = min(minMakespan, job.start_time);

			//bool valid = true;
			//valid = valid && (actualMakespan <= job.deadline);
			//cout << make << endl;
			//energy += job.wcet_at_freq * pow(job.nu, 2);
			}
		}
		actualMakespan = maxMakespan-minMakespan;

		//float fitness = actualMakespan * pow(energy, 2);
		float fitness = maxMakespan;
		//cout << "Fitness: " << fitness << "\n";
		//makespans.push_back (fitness);
		return fitness;
	}
}

/**
 * __genome_to_schedule() - transforms a genome into a schedule
 * @g: the genome
 *
 * Return: a schedule
 */
Fractal::Schedule Fractal::Scheduler::__genome_to_schedule(const GAGenome &g) {
	const GARealGenome &kGenome = GA_CAST(g);

	/* write genes to an array */
	int *tmp = Fractal::__genome_to_array(kGenome);

	/* each genome contains a pointer to the user data */
	Fractal::user_data *data = (Fractal::user_data*) g.userData();

	/* transform the genes into a solution */
	Fractal::Schedule schedule = data->model->schedule(tmp, data->pinned);

	/* free the array */
	delete[] tmp;

	return schedule;
}
