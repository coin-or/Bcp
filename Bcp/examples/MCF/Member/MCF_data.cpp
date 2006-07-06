#include <fstream>
#include <cmath>
#include "MCF_data.hpp"

//#############################################################################

void MCF_data::pack(BCP_buffer& buf) const
{
    int namelen = strlen(problem_name);
    buf.pack(problem_name, namelen);
    buf.pack(numnodes);
    buf.pack(numarcs);
    buf.pack(numcommodities);
    int i;
    for (i = 0; i < numarcs; ++i) {
	buf.pack(arcs[i].tail);
	buf.pack(arcs[i].head);
	buf.pack(arcs[i].lb);
	buf.pack(arcs[i].ub);
	buf.pack(arcs[i].weight);
    }
    for (i = 0; i < numcommodities; ++i) {
	buf.pack(commodities[i].source);
	buf.pack(commodities[i].sink);
	buf.pack(commodities[i].demand);
    }
}

/*---------------------------------------------------------------------------*/

void MCF_data::unpack(BCP_buffer& buf)
{
    int namelen;
    buf.unpack(problem_name, namelen);
    buf.unpack(numnodes);
    buf.unpack(numarcs);
    buf.unpack(numcommodities);
	
    int i;
    for (i = 0; i < numarcs; ++i) {
	buf.unpack(arcs[i].tail);
	buf.unpack(arcs[i].head);
	buf.unpack(arcs[i].lb);
	buf.unpack(arcs[i].ub);
	buf.unpack(arcs[i].weight);
    }
    for (i = 0; i < numcommodities; ++i) {
	buf.unpack(commodities[i].source);
	buf.unpack(commodities[i].sink);
	buf.unpack(commodities[i].demand);
    }
}

/*
  #############################################################################
  #    Here is the method that reads in the input file
  #############################################################################
*/

int MCF_data::readDimacsFormat(std::istream& s, bool addDummyArcs)
{
    double maxweight = 0;

    bool size_read = true;
    int arcs_read = 0;
    int commodities_read = 0;;

    char line[1000];
    char name[1000];

    while (s.good()) {
	s.getline(line, 1000);
	if (s.gcount() >= 999) {
	    printf("Input file is incorrect. A line more than 1000 characters is found.\n");
	    return 1;
	}
	switch (line[0]) {
	case 'p':
	    if (sscanf(line, "p%s%i%i%i",
		       name, &numnodes, &numarcs, &numcommodities) != 4) {
		printf("Input file is incorrect. (p line)\n");
		return 1;
	    }
	    problem_name = new char[strlen(name)+1];
	    memcpy(problem_name, name, strlen(name)+1);
	    arcs = new arc[numarcs + (addDummyArcs ? numcommodities : 0)];
	    commodities = new commodity[numcommodities];
	    break;
	case 'c':
	    break;
	case 'd':
	    if (sscanf(line, "d%i%i%i",
		       &commodities[commodities_read].source,
		       &commodities[commodities_read].sink,
		       &commodities[commodities_read].demand) != 3) {
		printf("Input file is incorrect. (d line)\n");
		return 1;
	    }
	    ++commodities_read;
	    break;
	case 'a':
	    if (sscanf(line, "a%i%i%i%i%lf",
		       &arcs[arcs_read].tail,
		       &arcs[arcs_read].head,
		       &arcs[arcs_read].lb,
		       &arcs[arcs_read].ub,
		       &arcs[arcs_read].weight) != 5) {
		printf("Input file is incorrect. (a line)\n");
		return 1;
	    }
	    if (fabs(arcs[arcs_read].weight) > maxweight) {
		maxweight = fabs(arcs[arcs_read].weight);
	    }
	    ++arcs_read;
	    break;
	default:
	    if (sscanf(line+1, "%s", name) <= 0) {
		printf("Input file is incorrect. (non-recognizable line)\n");
		return 1;
	    }
	    break;
	}
    }

    if (!size_read || arcs_read!=numarcs || commodities_read!=numcommodities) {
	printf("Input file is incorrect. size_read=%i arcs_read=%i commodities_read=%i\n",
	       size_read, arcs_read, commodities_read);
	return 1;
    }

    if (addDummyArcs) {
	for (int i = 0; i < numcommodities; ++i) {
	    arcs[numarcs].tail   = commodities[i].source;
	    arcs[numarcs].head   = commodities[i].sink;
	    arcs[numarcs].lb     = 0;
	    arcs[numarcs].ub     = commodities[i].demand;
	    arcs[numarcs].weight = maxweight+1;
	    ++numarcs;
	}
    }
    return 0;
}

//#############################################################################

