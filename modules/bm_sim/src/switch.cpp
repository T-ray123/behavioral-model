#include <cassert>

#include "bm_sim/switch.h"
#include "bm_sim/P4Objects.h"

Switch::Switch(bool enable_swap)
  : enable_swap(enable_swap) {
  p4objects = std::make_shared<P4Objects>();
  p4objects_rt = p4objects;
  // p4objects = std::unique_ptr<P4Objects>(new P4Objects());
  pre = std::unique_ptr<McPre>(new McPre());
}

void Switch::init_objects(const std::string &json_path) {
  std::fstream fs(json_path);
  p4objects->init_objects(fs);
  Packet::set_phv_factory(p4objects->get_phv_factory());
}

MatchErrorCode Switch::mt_add_entry(
    const std::string &table_name,
    const std::vector<MatchKeyParam> &match_key,
    const std::string &action_name,
    ActionData action_data,
    entry_handle_t *handle,
    int priority
) {
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  MatchTableAbstract *abstract_table = 
    p4objects_rt->get_abstract_match_table(table_name);
  assert(abstract_table);
  MatchTable *table = dynamic_cast<MatchTable *>(abstract_table);
  if(!table) return MatchErrorCode::WRONG_TABLE_TYPE;
  const ActionFn *action = p4objects_rt->get_action(action_name);
  assert(action);
  return table->add_entry(
    match_key, action, std::move(action_data), handle, priority
  );
}

MatchErrorCode Switch::mt_set_default_action(
    const std::string &table_name,
    const std::string &action_name,
    ActionData action_data
) {
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  MatchTableAbstract *abstract_table = 
    p4objects_rt->get_abstract_match_table(table_name);
  assert(abstract_table);
  MatchTable *table = dynamic_cast<MatchTable *>(abstract_table);
  if(!table) return MatchErrorCode::WRONG_TABLE_TYPE;
  const ActionFn *action = p4objects_rt->get_action(action_name);
  assert(action);
  return table->set_default_action(action, std::move(action_data));
}

MatchErrorCode Switch::mt_delete_entry(
    const std::string &table_name,
    entry_handle_t handle
) {
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  MatchTableAbstract *abstract_table = 
    p4objects_rt->get_abstract_match_table(table_name);
  assert(abstract_table);
  MatchTable *table = dynamic_cast<MatchTable *>(abstract_table);
  if(!table) return MatchErrorCode::WRONG_TABLE_TYPE;
  return table->delete_entry(handle);
}

MatchErrorCode Switch::mt_modify_entry(
    const std::string &table_name,
    entry_handle_t handle,
    const std::string &action_name,
    const ActionData action_data
) {
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  MatchTableAbstract *abstract_table = 
    p4objects_rt->get_abstract_match_table(table_name);
  assert(abstract_table);
  MatchTable *table = dynamic_cast<MatchTable *>(abstract_table);
  if(!table) return MatchErrorCode::WRONG_TABLE_TYPE;
  const ActionFn *action = p4objects_rt->get_action(action_name);
  assert(action);
  return table->modify_entry(handle, action, std::move(action_data));
}

MatchErrorCode
Switch::get_mt_indirect(
  const std::string &table_name, MatchTableIndirect **table
)
{
  MatchTableAbstract *abstract_table = 
    p4objects_rt->get_abstract_match_table(table_name);
  if(!abstract_table) return MatchErrorCode::INVALID_TABLE_NAME;
  *table = dynamic_cast<MatchTableIndirect *>(abstract_table);
  if(!(*table)) return MatchErrorCode::WRONG_TABLE_TYPE;
  return MatchErrorCode::SUCCESS;
}

MatchErrorCode
Switch::mt_indirect_add_member(
  const std::string &table_name, const std::string &action_name,
  ActionData action_data, mbr_hdl_t *mbr
)
{
  MatchErrorCode rc;
  MatchTableIndirect *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  const ActionFn *action = p4objects_rt->get_action(action_name);
  if(!action) return MatchErrorCode::INVALID_ACTION_NAME;
  return table->add_member(action, std::move(action_data), mbr);
}

MatchErrorCode
Switch::mt_indirect_delete_member(
  const std::string &table_name, mbr_hdl_t mbr
)
{
  MatchErrorCode rc;
  MatchTableIndirect *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->delete_member(mbr);
}
 
MatchErrorCode
Switch::mt_indirect_add_entry(
  const std::string &table_name,
  const std::vector<MatchKeyParam> &match_key,
  mbr_hdl_t mbr, entry_handle_t *handle, int priority
)
{
  MatchErrorCode rc;
  MatchTableIndirect *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->add_entry(match_key, mbr, handle, priority);
}

MatchErrorCode
Switch::mt_indirect_modify_entry(
  const std::string &table_name, entry_handle_t handle, mbr_hdl_t mbr
)
{
  MatchErrorCode rc;
  MatchTableIndirect *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->modify_entry(handle, mbr);
}
 
MatchErrorCode
Switch::mt_indirect_delete_entry(
  const std::string &table_name, entry_handle_t handle
)
{
  MatchErrorCode rc;
  MatchTableIndirect *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->delete_entry(handle);
}

MatchErrorCode
Switch::mt_indirect_set_default_member(
  const std::string &table_name, mbr_hdl_t mbr
)
{
  MatchErrorCode rc;
  MatchTableIndirect *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->set_default_member(mbr);
}

MatchErrorCode
Switch::get_mt_indirect_ws(
  const std::string &table_name, MatchTableIndirectWS **table
)
{
  MatchTableAbstract *abstract_table = 
    p4objects_rt->get_abstract_match_table(table_name);
  if(!abstract_table) return MatchErrorCode::INVALID_TABLE_NAME;
  *table = dynamic_cast<MatchTableIndirectWS *>(abstract_table);
  if(!(*table)) return MatchErrorCode::WRONG_TABLE_TYPE;
  return MatchErrorCode::SUCCESS;
}

MatchErrorCode
Switch::mt_indirect_ws_create_group(
  const std::string &table_name, grp_hdl_t *grp
)
{
  MatchErrorCode rc;
  MatchTableIndirectWS *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect_ws(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->create_group(grp);
}
 
MatchErrorCode
Switch::mt_indirect_ws_delete_group(
  const std::string &table_name, grp_hdl_t grp
)
{
  MatchErrorCode rc;
  MatchTableIndirectWS *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect_ws(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->delete_group(grp);
}
 
MatchErrorCode
Switch::mt_indirect_ws_add_member_to_group(
  const std::string &table_name, mbr_hdl_t mbr, grp_hdl_t grp
)
{
  MatchErrorCode rc;
  MatchTableIndirectWS *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect_ws(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->add_member_to_group(mbr, grp);
}

MatchErrorCode
Switch::mt_indirect_ws_remove_member_from_group(
  const std::string &table_name, mbr_hdl_t mbr, grp_hdl_t grp
)
{
  MatchErrorCode rc;
  MatchTableIndirectWS *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect_ws(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->remove_member_from_group(mbr, grp);
}

MatchErrorCode
Switch::mt_indirect_ws_add_entry(
  const std::string &table_name,
  const std::vector<MatchKeyParam> &match_key,
  grp_hdl_t grp, entry_handle_t *handle, int priority
)
{
  MatchErrorCode rc;
  MatchTableIndirectWS *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect_ws(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->add_entry_ws(match_key, grp, handle, priority);
}

MatchErrorCode
Switch::mt_indirect_ws_modify_entry(
  const std::string &table_name, entry_handle_t handle, grp_hdl_t grp
)
{
  MatchErrorCode rc;
  MatchTableIndirectWS *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect_ws(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->modify_entry_ws(handle, grp);
}

MatchErrorCode
Switch::mt_indirect_ws_set_default_group(
  const std::string &table_name, grp_hdl_t grp
)
{
  MatchErrorCode rc;
  MatchTableIndirectWS *table;
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  if((rc = get_mt_indirect_ws(table_name, &table)) != MatchErrorCode::SUCCESS)
    return rc;
  return table->set_default_group(grp);
}

MatchErrorCode Switch::table_read_counters(
    const std::string &table_name,
    entry_handle_t handle,
    MatchTableAbstract::counter_value_t *bytes,
    MatchTableAbstract::counter_value_t *packets
) {
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  MatchTableAbstract *abstract_table = 
    p4objects_rt->get_abstract_match_table(table_name);
  assert(abstract_table);
  return abstract_table->query_counters(handle, bytes, packets);
}

MatchErrorCode Switch::table_reset_counters(
    const std::string &table_name
) {
  boost::shared_lock<boost::shared_mutex> lock(request_mutex);
  MatchTableAbstract *abstract_table = 
    p4objects_rt->get_abstract_match_table(table_name);
  assert(abstract_table);
  return abstract_table->reset_counters();
}

RuntimeInterface::ErrorCode Switch::load_new_config(const std::string &new_config) {
  if(!enable_swap) return CONFIG_SWAP_DISABLED;
  boost::unique_lock<boost::shared_mutex> lock(request_mutex);
  // check that there is no ongoing config swap
  if(p4objects != p4objects_rt) return ONGOING_SWAP;
  p4objects_rt = std::make_shared<P4Objects>();
  std::stringstream ss(new_config);
  p4objects_rt->init_objects(ss);
  return SUCCESS;
}
 
RuntimeInterface::ErrorCode Switch::swap_configs() {
  if(!enable_swap) return CONFIG_SWAP_DISABLED;
  boost::unique_lock<boost::shared_mutex> lock(request_mutex);
  // no ongoing swap
  if(p4objects == p4objects_rt) return NO_ONGOING_SWAP;
  swap_ordered = true;
  return SUCCESS;
}

int Switch::do_swap() {
  if(!swap_ordered) return 1;
  boost::unique_lock<boost::shared_mutex> lock(request_mutex);
  p4objects = p4objects_rt;
  Packet::swap_phv_factory(p4objects->get_phv_factory());
  swap_ordered = false;
  return 0;
}

LearnEngine *Switch::get_learn_engine()
{
  return p4objects->get_learn_engine();
}
