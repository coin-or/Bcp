#include "MCF3_par.hpp"
#include "MCF3_init.hpp"
#include "MCF3_tm.hpp"
#include "MCF3_lp.hpp"

using std::make_pair;

//#############################################################################

int main(int argc, char* argv[])
{
    MCF3_init init;
    return bcp_main(argc, argv, &init);
}

//#############################################################################

template <>
void BCP_parameter_set<MCF3_par>::create_keyword_list()
{
    // Create the list of keywords for parameter file reading
    keys.push_back(make_pair(BCP_string("MCF3_AddDummySourceSinkArcs"),
			     BCP_parameter(BCP_CharPar, AddDummySourceSinkArcs)));
    keys.push_back(make_pair(BCP_string("MCF3_InputFilename"),
			     BCP_parameter(BCP_StringPar, InputFilename)));
}

template <>
void BCP_parameter_set<MCF3_par>::set_default_entries()
{
    set_entry(InputFilename, "small");
    set_entry(AddDummySourceSinkArcs, true);
}

//#############################################################################

USER_initialize * BCP_user_init()
{
    return new MCF3_init;
}

/*---------------------------------------------------------------------------*/

BCP_tm_user *
MCF3_init::tm_init(BCP_tm_prob& p,
		  const int argnum, const char * const * arglist)
{
    MCF3_tm* tm = new MCF3_tm;
    tm->par.read_from_arglist(argnum, arglist);
    std::ifstream ifs(tm->par.entry(MCF3_par::InputFilename).c_str());
    tm->data.readDimacsFormat(ifs,tm->par.entry(MCF3_par::AddDummySourceSinkArcs));
    return tm;
}

/*---------------------------------------------------------------------------*/

BCP_lp_user *
MCF3_init::lp_init(BCP_lp_prob& p)
{
    return new MCF3_lp;
}
