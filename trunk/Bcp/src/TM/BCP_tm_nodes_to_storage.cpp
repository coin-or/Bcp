// Copyright (C) 2007, International Business Machines
// Corporation and others.  All Rights Reserved.

#include "BCP_os.hpp"
#include "BCP_tm.hpp"
#include "BCP_tm_node.hpp"
#include "BCP_tm_user.hpp"
#include "BCP_tm_functions.hpp"

#define NUMNODES_BASED_ON_BUFSIZE

//#############################################################################

static bool BCP_tm_scan_children(BCP_tm_prob& p, BCP_tm_node* node,
				 std::vector<BCP_tm_node*>& nodes_to_send,
				 const long bufsize)
{
#ifndef NUMNODES_BASED_ON_BUFSIZE
  const size_t send_size = 100;
#endif
  BCP_vec<BCP_tm_node*>& children = node->_children;
  for (int i = children.size() - 1; i >= 0; --i) {
    BCP_tm_node* s = children[i];
    if (s == NULL)
      continue;
    if (BCP_tm_scan_children(p, s, nodes_to_send, bufsize))
      return true;
  }
  if (node->_data._desc.IsValid()) {
    nodes_to_send.push_back(node);
    const bool def = p.param(BCP_tm_par::ReportWhenDefaultIsExecuted);
    p.msg_buf.pack(node->_index);
    node->_data._desc->pack(p.packer, def, p.msg_buf);
    bool has_user_data = node->_data._user.IsValid();
    p.msg_buf.pack(has_user_data);
    if (has_user_data) {
      p.packer->pack_user_data(node->_data._user.GetRawPtr(), p.msg_buf);
    }
  }
#ifndef NUMNODES_BASED_ON_BUFSIZE
  return (nodes_to_send.size() >= send_size);
#else
  return (p.msg_buf.size() > bufsize);
#endif
}

//#############################################################################

static bool BCP_tm_scan_siblings(BCP_tm_prob& p, BCP_tm_node* node,
				 std::vector<BCP_tm_node*>& nodes_to_send,
				 const long bufsize)
{
#ifndef NUMNODES_BASED_ON_BUFSIZE
  const size_t send_size = 100;
#endif
  if (node->_data._desc.IsValid()) {
    nodes_to_send.push_back(node);
    const bool def = p.param(BCP_tm_par::ReportWhenDefaultIsExecuted);
    p.msg_buf.pack(node->_index);
    node->_data._desc->pack(p.packer, def, p.msg_buf);
    bool has_user_data = node->_data._user.IsValid();
    p.msg_buf.pack(has_user_data);
    if (has_user_data) {
      p.packer->pack_user_data(node->_data._user.GetRawPtr(), p.msg_buf);
    }
#ifndef NUMNODES_BASED_ON_BUFSIZE
    if (nodes_to_send.size() >= send_size)
#else
    if (p.msg_buf.size() > bufsize)
#endif
      {
	return true;
      }
  }
  BCP_tm_node* parent = node->_parent;
  if (parent == NULL)
    return false;
  BCP_vec<BCP_tm_node*>& siblings = parent->_children;
  for (int i = siblings.size() - 1; i >= 0; --i) {
    BCP_tm_node* s = siblings[i];
    if (s == node || s == NULL)
      continue;
    if (BCP_tm_scan_children(p, s, nodes_to_send, bufsize))
      return true;
  }
  return BCP_tm_scan_siblings(p, parent, nodes_to_send, bufsize);
}

//#############################################################################

/** This function is invoked from exactly one place, the beginning of
    BCP_tm_unpack_node_description(). So any time when data is received we
    call this function to decide if data balancing is needed or not. There may
    be another call from BCP_tm_do_one_phase() to keep the TM busy if it's
    idle.
*/

bool BCP_tm_is_data_balanced(BCP_tm_prob& p)
{
  const int maxheap = p.param(BCP_tm_par::MaxHeapSize);
  if (maxheap == -1) {
    return true;
  }
  // FIXME--DELETE
  printf("local nodes: %i\n", BCP_tm_node::num_local_nodes);
  printf("remote nodes: %i\n", BCP_tm_node::num_remote_nodes);

//   return (BCP_tm_node::num_local_nodes < 10);

  const double usedheap = BCP_used_heap();
  if (usedheap == -1)
    return true;
  const double freeheap = maxheap - usedheap;
  printf("free: %f   used: %f   free/max: %f\n",
	 freeheap, usedheap, freeheap/maxheap);
  return (freeheap > 1<<24 /* 16M */ && freeheap / maxheap > 0.15);
}

//#############################################################################

/** This function is invoked after data from an LP is unpacked (and only if \c
    p.need_a_TS is true). And maybe from BCP_tm_do_one_phase() to keep the TM
    busy if it's idle. It returns true/false depending on whether further
    balancing is needed.

    First we try to get hold of a TS. This might come from an existing TS or
    we might try to convert an LP into a TS. If neither succeeds we leave the
    flag on and return. The flag will ensure that no LP will be allowed to
    dive until the flag is cleared. Therefore the LP that has sent the data
    that triggered the call to to this function through the call to
    BCP_tm_unpack_node_description() will be free when the next LP sends some
    data. So at that time we will be able to balance. (NOTE: if the TM is
    idle, we might invoke this routine to do some useful work.)

    If we managed to get hold of a TS then we do the balancing
*/
bool BCP_tm_balance_data(BCP_tm_prob& p)
{
  if (! p.need_a_TS)
    return false;

  int pid = -1;

  /* check if any of the TS processes accept data */
  int max_space = 0;
  for (std::map<int,int>::const_iterator tsi = p.ts_space.begin();
       tsi != p.ts_space.end(); ++tsi) {
    if (tsi->second > max_space) {
      max_space = tsi->second;
      pid = tsi->first;
    }
  }
  if (max_space < 1 << 22 /* 4M */) {
    pid = -1;
  }

  BCP_buffer& buf = p.msg_buf;

  if (pid == -1) {
    /* None of the TS processes accept data, we need a new one. */
    BCP_proc_array* lp = p.slaves.lp;
    if (lp->procs().size() > 1) {
      BCP_proc_array* ts = p.slaves.ts;
      if (!ts) {
        p.slaves.ts = ts = new BCP_proc_array;
      }
      pid = lp->get_free_proc();
      lp->delete_proc(pid);
      ts->add_proc(pid);
      BCP_vec<int> pid_vec(1, pid);
      printf("Turning LP (#%i) into a TS\n", pid);
      BCP_tm_notify_process_type(p, BCP_ProcessType_TS, &pid_vec);
    }
  }

  if (pid == -1)
    return true;

  /* OK we can balance */

  /*
    FIXME: For now we just balance node data. When we run hybrid (cuts are
    FIXME: added) then we must balance cuts, too
  */
  std::vector<BCP_tm_node*> nodes_to_send;
  const std::vector<CoinTreeSiblings*>& candList =
    p.candidate_list.getTree()->getCandidates();
  BCP_tm_node* node =
    dynamic_cast<BCP_tm_node*>(candList.back()->currentNode());

  /* 'node' is a leaf. Starting from it traverse the tree until there is
     enough data to be sent off. The "enough" means that the message buffer
     takes up 25% of the free space. */
  int usedheap = BCP_used_heap();
  assert(usedheap > 0);
  const int maxheap = p.param(BCP_tm_par::MaxHeapSize);
  assert(maxheap > 0);
  int freeheap = maxheap - usedheap;
  printf("Before sending off: freeheap: %i   usedheap: %i\n",
	 freeheap, usedheap);
  buf.clear();
  BCP_tm_scan_siblings(p, node, nodes_to_send, freeheap >> 2);
  int num = nodes_to_send.size();
  if (num == 0) {
    // Everything is already sent out, but we are still having memory problems
    throw BCP_fatal_error("No memory left in TM\n");
  }
  int fake_index = -1;
  buf.pack(fake_index);
  p.msg_env->send(pid, BCP_Msg_NodeList, buf);
  buf.clear();
  int saved = 0, fm = 0;
  p.msg_env->receive(pid, BCP_Msg_NodeListReply, buf, -1);
  buf.unpack(saved);
  buf.unpack(fm);
  p.ts_space[pid] = fm;
  for (int i = 0; i < saved; ++i) {
    node = nodes_to_send[i];
    node->_locally_stored = false;
    node->_data._desc = NULL;
    node->_data._user = NULL;
    node->_data_location = pid;
  }
  nodes_to_send.clear();

  usedheap = BCP_used_heap();
  freeheap = maxheap - usedheap;
  printf("After sending off: freeheap: %i   usedheap: %i\n",
	 freeheap, usedheap);

  if (saved == 0) {
    // Something is wrong. The least full TS did not accept anything...
    //    sleep(10000);
    throw BCP_fatal_error("TS did not accept anything\n");
  }
  
  BCP_tm_node::num_local_nodes -= saved;
  BCP_tm_node::num_remote_nodes += saved;

  // FIXME--DELETE
  for (size_t k = 0; k < p.search_tree.size(); ++k) {
    BCP_tm_node* n = p.search_tree[k];
    if (n && n->_locally_stored) {
      assert(n->_data._desc.IsNull() && n->_data._user.IsNull());
    }
  }

  return false;
}

