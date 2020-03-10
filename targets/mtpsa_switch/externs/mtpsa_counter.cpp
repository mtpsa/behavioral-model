#include "mtpsa_counter.h"

namespace bm {

namespace mtpsa {

void
MTPSA_Counter::count(const Data &index) {
  const Packet &packet = get_packet();
  const PHV *phv = packet.get_phv();
  const unsigned permissions = phv->get_packet_permissions();
  if (!(permissions & MTPSA_PERM_COUNTER))
    _counter->get_counter(index.get<size_t>()).increment_counter(packet);
}

Counter &
MTPSA_Counter::get_counter(size_t idx) {
  return _counter->get_counter(idx);
}

const Counter &
MTPSA_Counter::get_counter(size_t idx) const {
  return _counter->get_counter(idx);
}

Counter::CounterErrorCode
MTPSA_Counter::reset_counters(){
  return _counter->reset_counters();
}

BM_REGISTER_EXTERN_W_NAME(Counter, MTPSA_Counter);
BM_REGISTER_EXTERN_W_NAME_METHOD(Counter, MTPSA_Counter, count, const Data &);

}  // namespace bm::mtpsa

}  // namespace bm

int import_counters(){
  return 0;
}
