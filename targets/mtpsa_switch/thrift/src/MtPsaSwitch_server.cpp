#include <bm/config.h>
#include <bm/MtPsaSwitch.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

namespace thrift_provider = apache::thrift;

#include <bm/bm_sim/switch.h>
#include <bm/bm_sim/logger.h>
#include <bm/thrift/stdcxx.h>

#include "mtpsa_switch.h"

using namespace bm::mtpsa;

namespace mtpswitch_runtime {

class MtPsaSwitchHandler : virtual public MtPsaSwitchIf {
 public:
  explicit MtPsaSwitchHandler(MtPsaSwitch *sw) : switch_(sw) { }

  int32_t mirroring_mapping_add(const int32_t mirror_id, const int32_t egress_port) {
    bm::Logger::get()->trace("mirroring_mapping_add");
    return switch_->mirroring_mapping_add(mirror_id, egress_port);
  }

  int32_t mirroring_mapping_delete(const int32_t mirror_id) {
    bm::Logger::get()->trace("mirroring_mapping_delete");
    return switch_->mirroring_mapping_delete(mirror_id);
  }

  int32_t mirroring_mapping_get_egress_port(const int32_t mirror_id) {
    bm::Logger::get()->trace("mirroring_mapping_get_egress_port");
    bm::port_t port;
    if (switch_->mirroring_mapping_get(mirror_id, &port)) {
      return port;
    }
    return -1;
  }

  int32_t set_egress_queue_depth(const int32_t port_num, const int32_t depth_pkts) {
    bm::Logger::get()->trace("set_egress_queue_depth");
    return switch_->set_egress_queue_depth(port_num, static_cast<uint32_t>(depth_pkts));
  }

  int32_t set_all_egress_queue_depths(const int32_t depth_pkts) {
    bm::Logger::get()->trace("set_all_egress_queue_depths");
    return switch_->set_all_egress_queue_depths(static_cast<uint32_t>(depth_pkts));
  }

  int32_t set_egress_queue_rate(const int32_t port_num,
                                const int64_t rate_pps) {
    bm::Logger::get()->trace("set_egress_queue_rate");
    return switch_->set_egress_queue_rate(port_num, static_cast<uint64_t>(rate_pps));
  }

  int32_t set_all_egress_queue_rates(const int64_t rate_pps) {
    bm::Logger::get()->trace("set_all_egress_queue_rates");
    return switch_->set_all_egress_queue_rates(static_cast<uint64_t>(rate_pps));
  }

  int32_t load_user_config(const int32_t user_id, const std::string &new_config) {
    bm::Logger::get()->trace("load_user_config");
    return switch_->load_user_config(user_id, new_config);
  }

  int64_t get_time_elapsed_us() {
    bm::Logger::get()->trace("get_time_elapsed_us");
    // cast from unsigned to signed
    return static_cast<int64_t>(switch_->get_time_elapsed_us());
  }

  int64_t get_time_since_epoch_us() {
    bm::Logger::get()->trace("get_time_since_epoch_us");
    // cast from unsigned to signed
    return static_cast<int64_t>(switch_->get_time_since_epoch_us());
  }

 private:
  MtPsaSwitch *switch_;
};

stdcxx::shared_ptr<MtPsaSwitchIf> get_handler(MtPsaSwitch *sw) {
  return stdcxx::shared_ptr<MtPsaSwitchHandler>(new MtPsaSwitchHandler(sw));
}

}  // namespace mtpswitch_runtime
