#ifndef _BM_SWITCH_H_
#define _BM_SWITCH_H_

#include <memory>
#include <string>
#include <fstream>

#include "P4Objects.h"
#include "queue.h"
#include "packet.h"
#include "learning.h"
#include "runtime_interface.h"
#include "pre.h"

#include <boost/thread/shared_mutex.hpp>

class Switch : public RuntimeInterface {
public:
  Switch(bool enable_swap = false);

  void init_objects(const std::string &json_path);

  P4Objects *get_p4objects() { return p4objects.get(); }

  virtual int receive(int port_num, const char *buffer, int len) = 0;

  virtual void start_and_return() = 0;

public:
  MatchErrorCode 
  mt_add_entry(const std::string &table_name,
	       const std::vector<MatchKeyParam> &match_key,
	       const std::string &action_name,
	       ActionData action_data,
	       entry_handle_t *handle,
	       int priority = -1/*only used for ternary*/) override;
  
  MatchErrorCode
  mt_set_default_action(const std::string &table_name,
			const std::string &action_name,
			ActionData action_data) override;
  
  MatchErrorCode
  mt_delete_entry(const std::string &table_name,
		  entry_handle_t handle) override;

  MatchErrorCode
  mt_modify_entry(const std::string &table_name,
		  entry_handle_t handle,
		  const std::string &action_name,
		  ActionData action_data) override;

  MatchErrorCode
  mt_indirect_add_member(const std::string &table_name,
			 const std::string &action_name,
			 ActionData action_data,
			 mbr_hdl_t *mbr) override;
  
  MatchErrorCode
  mt_indirect_delete_member(const std::string &table_name,
			    mbr_hdl_t mbr) override;
  
  MatchErrorCode
  mt_indirect_add_entry(const std::string &table_name,
			const std::vector<MatchKeyParam> &match_key,
			mbr_hdl_t mbr,
			entry_handle_t *handle,
			int priority = 1) override;

  MatchErrorCode
  mt_indirect_modify_entry(const std::string &table_name,
			   entry_handle_t handle,
			   mbr_hdl_t mbr) override;
  
  MatchErrorCode
  mt_indirect_delete_entry(const std::string &table_name,
			   entry_handle_t handle) override;

  MatchErrorCode
  mt_indirect_set_default_member(const std::string &table_name,
				 mbr_hdl_t mbr) override;
  
  MatchErrorCode
  mt_indirect_ws_create_group(const std::string &table_name,
			      grp_hdl_t *grp) override;
  
  MatchErrorCode
  mt_indirect_ws_delete_group(const std::string &table_name,
			      grp_hdl_t grp) override;
  
  MatchErrorCode
  mt_indirect_ws_add_member_to_group(const std::string &table_name,
				     mbr_hdl_t mbr, grp_hdl_t grp) override;
 
  MatchErrorCode
  mt_indirect_ws_remove_member_from_group(const std::string &table_name,
					  mbr_hdl_t mbr, grp_hdl_t grp) override;

  MatchErrorCode
  mt_indirect_ws_add_entry(const std::string &table_name,
			   const std::vector<MatchKeyParam> &match_key,
			   grp_hdl_t grp,
			   entry_handle_t *handle,
			   int priority = 1) override;

  MatchErrorCode
  mt_indirect_ws_modify_entry(const std::string &table_name,
			      entry_handle_t handle,
			      grp_hdl_t grp) override;

  MatchErrorCode
  mt_indirect_ws_set_default_group(const std::string &table_name,
				   grp_hdl_t grp) override;

  MatchErrorCode
  table_read_counters(const std::string &table_name,
		      entry_handle_t handle,
		      MatchTableAbstract::counter_value_t *bytes,
		      MatchTableAbstract::counter_value_t *packets) override;

  MatchErrorCode
  table_reset_counters(const std::string &table_name) override;

  RuntimeInterface::ErrorCode
  load_new_config(const std::string &new_config) override;

  RuntimeInterface::ErrorCode
  swap_configs() override;

  LearnEngine *get_learn_engine();

  McPre *get_pre() { return pre.get(); }

protected:
  int swap_requested() { return swap_ordered; }
  // TODO: should I return shared_ptrs instead of raw_ptrs?
  // invalidate all pointers obtained with get_pipeline(), get_parser(),...
  int do_swap();

  // do these methods need any protection
  Pipeline *get_pipeline(const std::string &name) {
    return p4objects->get_pipeline(name);
  }

  Parser *get_parser(const std::string &name) {
    return p4objects->get_parser(name);
  }

  Deparser *get_deparser(const std::string &name) {
    return p4objects->get_deparser(name);
  }

private:
  MatchErrorCode get_mt_indirect(const std::string &table_name,
				 MatchTableIndirect **table);
  MatchErrorCode get_mt_indirect_ws(const std::string &table_name,
				    MatchTableIndirectWS **table);

protected:
  std::unique_ptr<McPre> pre{nullptr};

private:
  mutable boost::shared_mutex request_mutex{};
  std::atomic<bool> swap_ordered{false};

  std::shared_ptr<P4Objects> p4objects{nullptr};
  std::shared_ptr<P4Objects> p4objects_rt{nullptr};

  bool enable_swap{false};
};

#endif
