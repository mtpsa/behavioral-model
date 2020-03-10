#ifndef MTPSA_SWITCH_MTPSA_SWITCH_H_
#define MTPSA_SWITCH_MTPSA_SWITCH_H_

#include <bm/bm_sim/queue.h>
#include <bm/bm_sim/queueing.h>
#include <bm/bm_sim/packet.h>
#include <bm/bm_sim/switch.h>
#include <bm/bm_sim/event_logger.h>
#include <bm/bm_sim/simple_pre_lag.h>

#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include <functional>

#include "externs/mtpsa_counter.h"

using ts_res = std::chrono::microseconds;
using std::chrono::duration_cast;
using ticks = std::chrono::nanoseconds;

namespace bm {

namespace mtpsa {

class MtPsaSwitch : public Switch {
 public:
  using mirror_id_t = int;

  using TransmitFn = std::function<void(port_t, packet_id_t, const char *, int)>;

 private:
  using clock = std::chrono::high_resolution_clock;

 public:

  explicit MtPsaSwitch();

  ~MtPsaSwitch();

  int receive_(port_t port_num, const char *buffer, int len) override;

  void start_and_return_() override;

  void reset_target_state_() override;

  int mirroring_mapping_add(mirror_id_t mirror_id, port_t egress_port) {
    mirroring_map[mirror_id] = egress_port;
    return 0;
  }

  int mirroring_mapping_delete(mirror_id_t mirror_id) {
    return mirroring_map.erase(mirror_id);
  }

  bool mirroring_mapping_get(mirror_id_t mirror_id, port_t *port) const {
    return get_mirroring_mapping(mirror_id, port);
  }

  int set_egress_queue_depth(size_t port, const size_t depth_pkts);
  int set_all_egress_queue_depths(const size_t depth_pkts);

  int set_egress_queue_rate(size_t port, const uint64_t rate_pps);
  int set_all_egress_queue_rates(const uint64_t rate_pps);
  int load_user_config(size_t user_id, const std::string &new_config);

  uint64_t get_time_elapsed_us() const;

  uint64_t get_time_since_epoch_us() const;

  static packet_id_t get_packet_id() {
    return (packet_id-1);
  }

  Pipeline *get_user_pipeline(size_t user_id, const std::string &name) {
    return get_context(user_id)->get_pipeline(name);
  }

  Parser *get_user_parser(size_t user_id, const std::string &name) {
    return get_context(user_id)->get_parser(name);
  }

  Deparser *get_user_deparser(size_t user_id, const std::string &name) {
    return get_context(user_id)->get_deparser(name);
  }

  void set_transmit_fn(TransmitFn fn);

  // overriden interfaces
  Counter::CounterErrorCode
  read_counters(cxt_id_t cxt_id,
                const std::string &counter_name,
                size_t index,
                MatchTableAbstract::counter_value_t *bytes,
                MatchTableAbstract::counter_value_t *packets) override
  {
    auto *context = get_context(cxt_id);
    auto *ex = context->get_extern_instance(counter_name).get();
    if (!ex) return Counter::CounterErrorCode::INVALID_COUNTER_NAME;
    auto *counter = static_cast<MTPSA_Counter*>(ex);
    if (index >= counter->size())
      return Counter::CounterErrorCode::INVALID_INDEX;
    counter->get_counter(index).query_counter(bytes, packets);
    return Counter::CounterErrorCode::SUCCESS;
  }

  Counter::CounterErrorCode
  write_counters(cxt_id_t cxt_id,
                 const std::string &counter_name,
                 size_t index,
                 MatchTableAbstract::counter_value_t bytes,
                 MatchTableAbstract::counter_value_t packets) override
  {
    auto *context = get_context(cxt_id);
    auto *ex = context->get_extern_instance(counter_name).get();
    if (!ex) return Counter::CounterErrorCode::INVALID_COUNTER_NAME;
    auto *counter = static_cast<MTPSA_Counter*>(ex);
    if (index >= counter->size())
      return Counter::CounterErrorCode::INVALID_INDEX;
    counter->get_counter(index).write_counter(bytes, packets);
    return Counter::CounterErrorCode::SUCCESS;
  }

  Counter::CounterErrorCode
  reset_counters(cxt_id_t cxt_id,
                 const std::string &counter_name) override
  {
    Context *context = get_context(cxt_id);
    ExternType *ex = context->get_extern_instance(counter_name).get();
    if (!ex) return Counter::CounterErrorCode::INVALID_COUNTER_NAME;
    MTPSA_Counter *counter = static_cast<MTPSA_Counter*>(ex);
    return counter->reset_counters();
  }

  void set_nbusers(unsigned value) {
    nbusers = value;
  }

 private:
  static constexpr size_t nb_user_threads = 16u;
  static constexpr port_t MTPSA_PORT_RECIRCULATE = 0xfffffffa;
  static packet_id_t packet_id;

  enum PktInstanceType {
    PACKET_PATH_NORMAL,
    PACKET_PATH_NORMAL_UNICAST,
    PACKET_PATH_NORMAL_MULTICAST,
    PACKET_PATH_CLONE_I2E,
    PACKET_PATH_CLONE_E2E,
    PACKET_PATH_RESUBMIT,
    PACKET_PATH_RECIRCULATE,
  };

  struct EgressThreadMapper {
    explicit EgressThreadMapper(size_t nb_threads)
        : nb_threads(nb_threads) { }

    size_t operator()(size_t egress_port) const {
      return egress_port % nb_threads;
    }

    size_t nb_threads;
  };

 private:
  void ingress_thread();
  void egress_thread(size_t user_id);
  void transmit_thread();

  bool get_mirroring_mapping(mirror_id_t mirror_id, port_t *port) const {
    const auto it = mirroring_map.find(mirror_id);
    if (it != mirroring_map.end()) {
      *port = it->second;
      return true;
    }
    return false;
  }

  ts_res get_ts() const;

  void enqueue(int user_id, port_t egress_port, std::unique_ptr<Packet> &&packet);

  void check_queueing_metadata();

 private:
  std::vector<std::thread> threads_;
  Queue<std::unique_ptr<Packet>> input_buffer;
  bm::QueueingLogicRL<std::unique_ptr<Packet>, EgressThreadMapper> egress_buffers;
  Queue<std::unique_ptr<Packet>> output_buffer;
  TransmitFn my_transmit_fn;
  std::shared_ptr<McSimplePreLAG> pre;
  clock::time_point start;
  std::unordered_map<mirror_id_t, port_t> mirroring_map;
  bool with_queueing_metadata{false};
  unsigned nbusers;
};

}  // namespace bm::mtpsa
}  // namespace bm

#endif  // MTPSA_SWITCH_MTPSA_SWITCH_H_
