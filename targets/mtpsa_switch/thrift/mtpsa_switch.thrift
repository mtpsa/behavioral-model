namespace cpp mtpswitch_runtime
namespace py mtpswitch_runtime

service MtPsaSwitch {

  i32 mirroring_mapping_add(1:i32 mirror_id, 2:i32 egress_port);
  i32 mirroring_mapping_delete(1:i32 mirror_id);
  i32 mirroring_mapping_get_egress_port(1:i32 mirror_id);

  i32 set_egress_queue_depth(1:i32 port_num, 2:i32 depth_pkts);
  i32 set_all_egress_queue_depths(1:i32 depth_pkts);
  i32 set_egress_queue_rate(1:i32 port_num, 2:i64 rate_pps);
  i32 set_all_egress_queue_rates(1:i64 rate_pps);
  i32 load_user_config(1:i32 user_id, 2:string config_str);

  // these methods are here as an experiment, prefer get_time_elapsed_us() when
  // possible
  i64 get_time_elapsed_us();
  i64 get_time_since_epoch_us();

}
