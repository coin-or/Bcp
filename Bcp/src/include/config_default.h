/* include the COIN-OR-wide system specific configure header */
#include "configall_system.h"

/* include the public project specific macros */
#include "config_bcp_default.h"

/***************************************************************************/
/*             HERE DEFINE THE PROJECT SPECIFIC MACROS                     */
/*    These are only in effect in a setting that doesn't use configure     */
/***************************************************************************/

/* If defined, debug sanity checks are performed during runtime */
/* #define COIN_DEBUG 1 */

/* Define to 1 if the CoinUtils package is used */
#define COIN_HAS_COINUTILS 1

/* Define to 1 if the Mpi package is used */
/* #define COIN_HAS_MPI 1 */

/* a few things that exists under unix, but not under windows */
#include <Windows.h>
#define gethostname(c, l) { DWORD cnlen = l; GetComputerName(c, &cnlen); }
#include <process.h>

