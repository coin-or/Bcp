#include "MCF2_par.hpp"
#include "MCF2_init.hpp"
#include "MCF2_tm.hpp"
#include "MCF2_lp.hpp"

using std::make_pair;

//#############################################################################

int main(int argc, char* argv[])
{
  MCF2_init init;
  return bcp_main(argc, argv, &init);
}

//#############################################################################

template <>
void BCP_parameter_set<MCF2_par>::create_keyword_list()
{
  // Create the list of keywords for parameter file reading
  keys.push_back(make_pair(BCP_string("MCF2_AddDummySourceSinkArcs"),
			   BCP_parameter(BCP_CharPar, AddDummySourceSinkArcs)));
  keys.push_back(make_pair(BCP_string("MCF2_InputFilename"),
			   BCP_parameter(BCP_StringPar, InputFilename)));
}

template <>
void BCP_parameter_set<MCF2_par>::set_default_entries()
{
  set_entry(InputFilename, "small");
  set_entry(AddDummySourceSinkArcs, true);
}

//#############################################################################

USER_initialize * BCP_user_init()
{
  return new MCF2_init;
}

/*---------------------------------------------------------------------------*/

BCP_user_pack*
MCF2_init::packer_init(BCP_user_class* p)
{
  return new MCF2_packer;
}

/*---------------------------------------------------------------------------*/

BCP_tm_user *
MCF2_init::tm_init(BCP_tm_prob& p,
		   const int argnum, const char * const * arglist)
{
  MCF2_tm* tm = new MCF2_tm;
  tm->par.read_from_arglist(argnum, arglist);
  std::ifstream ifs(tm->par.entry(MCF2_par::InputFilename).c_str());
  tm->data.readDimacsFormat(ifs,
			    tm->par.entry(MCF2_par::AddDummySourceSinkArcs));
  return tm;
}

/*---------------------------------------------------------------------------*/

BCP_lp_user *
MCF2_init::lp_init(BCP_lp_prob& p)
{
  return new MCF2_lp;
}
