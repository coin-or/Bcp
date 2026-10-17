/***************************************************************************/
/*           HERE DEFINE THE PROJECT SPECIFIC PUBLIC MACROS                */
/*    These are only in effect in a setting that doesn't use configure     */
/***************************************************************************/

/* Version number of project */
#define BCP_VERSION      "devel"

/* Major Version number of project */
#define BCP_VERSION_MAJOR   9999

/* Minor Version number of project */
#define BCP_VERSION_MINOR   9999

/* Release Version number of project */
#define BCP_VERSION_RELEASE 9999

/* Define to 1 if the Mpi package is used */
/* #define BCP_HAS_MPI 1 */

#ifndef BCPLIB_EXPORT
#if defined(_WIN32) && defined(DLL_EXPORT)
#define BCPLIB_EXPORT __declspec(dllimport)
#else
#define BCPLIB_EXPORT
#endif
#endif
