// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#include <cstdio>
#include <cstring>
#include <fstream>
#include <cctype>
#include <algorithm>

#include <pvm3.h>

enum messages {
   BCP_CONFIG_CHANGE = 1,
   BCP_CONFIG_ERROR = 2,
   BCP_CONFIG_OK = 3,
   BCP_ARE_YOU_TREEMANAGER = 4,
   BCP_I_AM_TREEMANAGER = 5
};

static inline bool
str_eq(const char * str0, const char * str1)
{
   return strcmp(str0, str1) == 0;
}

static inline int
str_lt(const char * str0, const char * str1)
{
   return strcmp(str0, str1) < 0;
}

static void
find_tree_manager(const int my_tid, int &tm_tid);

static void
stop(const char * msg);

int main(int argc, char** argv)
{
   int pid = pvm_mytid();

   if (argc != 2 && argc != 3)
      stop("Usage: tm_driver <control_file> [TM_tid]\n");

   int tm_tid = 0;
   char * control_file = strdup(argv[1]);

   if (argc == 3)
      sscanf(argv[2], "t%x", &tm_tid);

   int info = 0;

   // Get the machine configuration
   int nhost = 0;
   int narch = 0;
   struct pvmhostinfo *hostp = 0;
   info = pvm_config(&nhost, &narch, &hostp);

   // Parse the control file
   int to_delete_size = 0;  // # of machsto delete
   char ** to_delete = 0;   // names of machs to delete
   int to_add_size = 0;     // # of machsto add
   char ** to_add = 0;      // names of machs to add

   int delete_proc_num = 0; // # of procs to delete
   int * tid_delete = 0;    // the tids of procs to delete

   // # of various procs to start
   int lp_num = 0;
   int cg_num = 0;
   int vg_num = 0;
   int cp_num = 0;
   int vp_num = 0;
   // the mach names where the procs shoud be started
   char ** lp_mach = 0;
   char ** cg_mach = 0;
   char ** vg_mach = 0;
   char ** cp_mach = 0;
   char ** vp_mach = 0;

   // Do the parsing. First count
   ifstream ctl(control_file);
   if (!ctl)
      stop("Cannot open parameter file... Aborting.\n");
   // Get the lines of the parameter file one-by-one and if a line contains a
   // (keyword, value) pair then interpret it.
   const int MAX_PARAM_LINE_LENGTH = 1024;
   char line[MAX_PARAM_LINE_LENGTH+1], *end_of_line, *keyword, *value, *ctmp;
   char ch;
   while (ctl) {
      ctl.get(line, MAX_PARAM_LINE_LENGTH);
      if (ctl) {
	 ctl.get(ch);
	 if (ch != '\n') {
	    printf("Too long (>= %i chars) line in the parameter file.\n",
		   MAX_PARAM_LINE_LENGTH);
	    stop("This is absurd. Aborting.\n");
	 }
      }
      end_of_line = line + strlen(line);
      //-------------------------- First separate the keyword and value ------
      keyword = find_if(line, end_of_line, isgraph);
      if (keyword == end_of_line) // empty line
	 continue;
      ctmp = find_if(keyword, end_of_line, isspace);
      if (ctmp == end_of_line) // line is just one word. must be a comment
	 continue;
      *ctmp = 0; // terminate the keyword with a 0 character
      ++ctmp;

      value = find_if(ctmp, end_of_line, isgraph);
      if (value == end_of_line) // line is just one word. must be a comment
	 continue;
      
      ctmp = find_if(value, end_of_line, isspace);
      *ctmp = 0; // terminate the value with a 0 character. this is good even
		 // if ctmp == end_ofline

      if (str_eq(keyword, "BCP_delete_machine")) {
	 ++to_delete_size;
      } else if (str_eq(keyword, "BCP_add_machine")) {
	 ++to_add_size;
      } else if (str_eq(keyword, "BCP_delete_proc")) {
	 ++delete_proc_num;
      } else if (str_eq(keyword, "BCP_lp_process")) {
	 ++lp_num;
      } else if (str_eq(keyword, "BCP_cg_process")) {
	 ++cg_num;
      } else if (str_eq(keyword, "BCP_vg_process")) {
	 ++vg_num;
      } else if (str_eq(keyword, "BCP_cp_process")) {
	 ++cp_num;
      } else if (str_eq(keyword, "BCP_vp_process")) {
	 ++vp_num;
      }
   }
   ctl.close();

   if (to_delete_size > 0) {
      to_delete = new char*[to_delete_size];
      to_delete_size = 0;
   }
   if (to_add_size > 0) {
      to_add = new char*[to_add_size];
      to_add_size = 0;
   }
   if (delete_proc_num > 0) {
      tid_delete = new int[delete_proc_num];
      delete_proc_num = 0;
   }
   if (lp_num) {
      lp_mach = new char*[lp_num];
      lp_num = 0;
   }
   if (cg_num) {
      cg_mach = new char*[cg_num];
      cg_num = 0;
   }
   if (vg_num) {
      vg_mach = new char*[vg_num];
      vg_num = 0;
   }
   if (cp_num) {
      cp_mach = new char*[cp_num];
      cp_num = 0;
   }
   if (vp_num) {
      vp_mach = new char*[vp_num];
      vp_num = 0;
   }

   ctl.open(control_file);
   while (ctl) {
      ctl.get(line, MAX_PARAM_LINE_LENGTH);
      if (ctl) {
	 ctl.get(ch);
	 if (ch != '\n') {
	    printf("Too long (>= %i chars) line in the parameter file.\n",
		   MAX_PARAM_LINE_LENGTH);
	    stop("This is absurd. Aborting.\n");
	 }
      }
      end_of_line = line + strlen(line);
      //-------------------------- First separate the keyword and value ------
      keyword = find_if(line, end_of_line, isgraph);
      if (keyword == end_of_line) // empty line
	 continue;
      ctmp = find_if(keyword, end_of_line, isspace);
      if (ctmp == end_of_line) // line is just one word. must be a comment
	 continue;
      *ctmp = 0; // terminate the keyword with a 0 character
      ++ctmp;

      value = find_if(ctmp, end_of_line, isgraph);
      if (value == end_of_line) // line is just one word. must be a comment
	 continue;
      
      ctmp = find_if(value, end_of_line, isspace);
      *ctmp = 0; // terminate the value with a 0 character. this is good even
		 // if ctmp == end_ofline

      if (str_eq(keyword, "BCP_delete_machine")) {
	 to_delete[to_delete_size++] = strdup(value);
      } else if (str_eq(keyword, "BCP_add_machine")) {
	 to_add[to_add_size++] = strdup(value);
      } else if (str_eq(keyword, "BCP_delete_proc")) {
	 sscanf(value, "t%x", &tid_delete[delete_proc_num++]);
      } else if (str_eq(keyword, "BCP_lp_process")) {
	 lp_mach[lp_num++] = strdup(value);
      } else if (str_eq(keyword, "BCP_cg_process")) {
	 cg_mach[cg_num++] = strdup(value);
      } else if (str_eq(keyword, "BCP_vg_process")) {
	 vg_mach[vg_num++] = strdup(value);
      } else if (str_eq(keyword, "BCP_cp_process")) {
	 cp_mach[cp_num++] = strdup(value);
      } else if (str_eq(keyword, "BCP_vp_process")) {
	 vp_mach[vp_num++] = strdup(value);
      }
   }
   ctl.close();

   // Check that machine deletions and additions are correct

   char ** last = 0;

   // Are there duplicates on the to be deleted list ?
   if (to_delete_size > 0) {
      sort(to_delete, to_delete + to_delete_size, str_lt);
      last = unique(to_delete, to_delete + to_delete_size, str_eq);
      if (to_delete_size != last - to_delete)
	 stop("A machine to be deleted is listed twice... Aborting.\n");
   }

   // Are there duplicates on the to be added list?
   if (to_add_size > 0) {
      sort(to_add, to_add + to_add_size, str_lt);
      last = unique(to_add, to_add + to_add_size, str_eq);
      if (to_add_size != last - to_add)
	 stop("A machine to be added is listed twice... Aborting.\n");
   }

   int i;
   char ** mach_list = new char*[nhost + to_add_size];
   for (i = 0; i < nhost; ++i)
      mach_list[i] = strdup(hostp[i].hi_name);
   sort(mach_list, mach_list + nhost, str_lt);

   char ** current_list = new char*[nhost + to_add_size];

   // Is there a nonexisting machine to be deleted?
   if (to_delete_size > 0) {
      last = set_difference(to_delete, to_delete + to_delete_size,
			    mach_list, mach_list + nhost,
			    current_list, str_lt);
      if (last != current_list)
	 stop("A nonexisting machine is to be deleted... Aborting.\n");
      last = set_difference(mach_list, mach_list + nhost,
			    to_delete, to_delete + to_delete_size,
			    current_list, str_lt);
      ::swap(mach_list, current_list);
   }

   // Is there an already existing machine to be added?
   if (to_add_size > 0) {
      last = set_intersection(to_add, to_add + to_add_size,
			      mach_list, mach_list + nhost,
			      current_list, str_lt);
      if (last != current_list)
	 stop("A machine to be added is already there... Aborting.\n");
      last = merge(to_add, to_add + to_add_size,
		   mach_list, mach_list + nhost,
		   current_list, str_lt);
      ::swap(mach_list, current_list);
   }

   const int mach_num = nhost - to_delete_size + to_add_size;

   // Check that the machines the new processes are supposed to be started on
   // really exist.

   if (lp_num > 0) {
      sort(lp_mach, lp_mach + lp_num, str_lt);
      if (set_difference(lp_mach, lp_mach + lp_num,
			 mach_list, mach_list + mach_num,
			 current_list, str_lt) != current_list)
	 stop("An lp machine is not in the final machine list... Aborting.\n");
   }
   if (cg_num > 0) {
      sort(cg_mach, cg_mach + cg_num, str_lt);
      if (set_difference(cg_mach, cg_mach + cg_num,
			 mach_list, mach_list + mach_num,
			 current_list, str_lt) != current_list)
	 stop("An cg machine is not in the final machine list... Aborting.\n");
   }
   if (vg_num > 0) {
      sort(vg_mach, vg_mach + vg_num, str_lt);
      if (set_difference(vg_mach, vg_mach + vg_num,
			 mach_list, mach_list + mach_num,
			 current_list, str_lt) != current_list)
	 stop("An vg machine is not in the final machine list... Aborting.\n");
   }
   if (cp_num > 0) {
      sort(cp_mach, cp_mach + cp_num, str_lt);
      if (set_difference(cp_mach, cp_mach + cp_num,
			 mach_list, mach_list + mach_num,
			 current_list, str_lt) != current_list)
	 stop("An cp machine is not in the final machine list... Aborting.\n");
   }
   if (vp_num > 0) {
      sort(vp_mach, vp_mach + vp_num, str_lt);
      if (set_difference(vp_mach, vp_mach + vp_num,
			 mach_list, mach_list + mach_num,
			 current_list, str_lt) != current_list)
	 stop("An vp machine is not in the final machine list... Aborting.\n");
   }

   // Find the tree manager
   find_tree_manager(pid, tm_tid);

   // Check that the TM is not on one of the machines to be deleted.
   if (to_delete_size > 0) {
      const int dtid = pvm_tidtohost(tm_tid);
      for (i = 0; i < nhost; ++i) {
	 if (hostp[i].hi_tid == dtid)
	    for (int j = 0; j < to_delete_size; ++j) {
	       if (str_eq(hostp[i].hi_name, to_delete[j]))
		  stop("Can't delete the machine the TM is on. Aborting.\n");
	    }
      }
   }

   // Check that the TM is not one of the processes to be deleted
   if (delete_proc_num > 0) {
      if (find(tid_delete, tid_delete + delete_proc_num, tm_tid) !=
	  tid_delete + delete_proc_num)
	 stop("Can't delete the TM... Aborting.\n");
   }

   // Modify the machine configuration
   if (to_delete_size > 0 || to_add_size > 0) {
      int * infos = new int[max(to_delete_size, to_add_size)];
      if (to_delete_size > 0)
	 if (pvm_delhosts(to_delete, to_delete_size, infos) < 0) {
	    printf("Failed to delete all specified machines...\n");
	    stop("Please check the situation manually... Aborting.\n");
	 }
      if (to_add_size > 0)
	 if (pvm_addhosts(to_add, to_add_size, infos) < 0) {
	    printf("Failed to add all specified machines...\n");
	    stop("Please check the situation manually... Aborting.\n");
	 }
   }

   // Kill the processes to be killed
   for (i = 0; i < delete_proc_num; ++i)
      pvm_kill(tid_delete[i]);

   // Put together a message to be sent to the TM that contains the machine
   // names on which the new processes should be spawned
   int len = (lp_num + cg_num + vg_num + cp_num + vp_num) * sizeof(int);
   if (len > 0) {
      len += 5 * sizeof(int);
      for (i = 0; i < lp_num; ++i) len += strlen(lp_mach[i]);
      for (i = 0; i < cg_num; ++i) len += strlen(cg_mach[i]);
      for (i = 0; i < vg_num; ++i) len += strlen(vg_mach[i]);
      for (i = 0; i < cp_num; ++i) len += strlen(cp_mach[i]);
      for (i = 0; i < vp_num; ++i) len += strlen(vp_mach[i]);

      char * buf = new char[len];

      memcpy(buf, &lp_num, sizeof(int));
      buf += sizeof(int);
      for (i = 0; i < lp_num; ++i) {
	 const int l = strlen(lp_mach[i]);
	 memcpy(buf, &l, sizeof(int));
	 buf += sizeof(int);
	 memcpy(buf, lp_mach[i], l);
	 buf += l;
      }

      memcpy(buf, &cg_num, sizeof(int));
      buf += sizeof(int);
      for (i = 0; i < cg_num; ++i) {
	 const int l = strlen(cg_mach[i]);
	 memcpy(buf, &l, sizeof(int));
	 buf += sizeof(int);
	 memcpy(buf, cg_mach[i], l);
	 buf += l;
      }

      memcpy(buf, &vg_num, sizeof(int));
      buf += sizeof(int);
      for (i = 0; i < vg_num; ++i) {
	 const int l = strlen(vg_mach[i]);
	 memcpy(buf, &l, sizeof(int));
	 buf += sizeof(int);
	 memcpy(buf, vg_mach[i], l);
	 buf += l;
      }

      memcpy(buf, &cp_num, sizeof(int));
      buf += sizeof(int);
      for (i = 0; i < cp_num; ++i) {
	 const int l = strlen(cp_mach[i]);
	 memcpy(buf, &l, sizeof(int));
	 buf += sizeof(int);
	 memcpy(buf, cp_mach[i], l);
	 buf += l;
      }

      memcpy(buf, &vp_num, sizeof(int));
      buf += sizeof(int);
      for (i = 0; i < vp_num; ++i) {
	 const int l = strlen(vp_mach[i]);
	 memcpy(buf, &l, sizeof(int));
	 buf += sizeof(int);
	 memcpy(buf, vp_mach[i], l);
	 buf += l;
      }

      buf -= len;

      pvm_initsend(PvmDataRaw);
      pvm_pkbyte(buf, len, 1);
      pvm_send(tm_tid, BCP_CONFIG_CHANGE);

      int bufid = pvm_recv(tm_tid, -1);
      int bytes = 0, msgtag = 0;
      pvm_bufinfo(bufid, &bytes, &msgtag, &tm_tid);
      if (msgtag == BCP_CONFIG_ERROR)
	 stop("TM had difficulties. Please check the situation manually.\n");
   }

   pvm_exit();
   return 0;
}

//#############################################################################
// Find the TreeManager

static void
find_tree_manager(const int my_tid, int &tm_tid)
{
   struct pvmtaskinfo *taskp = 0;
   int ntask = 0;
   pvm_tasks(0, &ntask, &taskp);
   int * tids = new int[ntask];
   int i, k;
   
   for (i = 0, k = 0; i < ntask; ++i) {
      if (taskp[i].ti_ptid != 0)
	 continue; // has a parent, can't be the TM
      if (taskp[i].ti_tid == my_tid)
	 continue; // self
      // Otherwise it could be the TM (might be a console...)
      tids[k++] = taskp[i].ti_tid;
   }

   if (tm_tid != 0) {
      // Check that the given tid is among the candidates
      for (i = 0; i < k; ++i)
	 if (tids[i] == tm_tid)
	    break;
      if (i == k)
	 stop("No TM candidate has the given tid... Aborting.\n");
   } else {
      // Broadcast a query to the candidates
      pvm_initsend(PvmDataRaw);
      pvm_mcast(tids, k, BCP_ARE_YOU_TREEMANAGER);
      // Wait for an answer
      struct timeval tout = {15, 0};
      int bufid = pvm_trecv(-1, BCP_I_AM_TREEMANAGER, &tout);
      if (bufid == 0)
	 stop("No TM candidates replied within 30 seconds... Aborting.\n");
      int bytes = 0, msgtag = 0;
      pvm_bufinfo(bufid, &bytes, &msgtag, &tm_tid);
   }

   delete[] tids;
}

//#############################################################################

static void
stop(const char * msg)
{
   printf("%s", msg);
   pvm_exit();
   abort();
}
