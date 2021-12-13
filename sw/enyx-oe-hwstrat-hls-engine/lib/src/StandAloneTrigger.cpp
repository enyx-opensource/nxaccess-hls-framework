#include <enyx/oe/hwstrat/demo/StandAloneTrigger.hpp>
#include <enyx/oe/hwstrat/demo/Helper.hpp>


namespace enyx {
namespace oe {
namespace hwstrat {
namespace demo {

StandAloneTrigger::StandAloneTrigger(uint8_t accelerator_index)
    : accelerator_(find_accelerator(accelerator_index))
    , stream_{find_c2a_stream(accelerator_, "user0")} {
}

bool
StandAloneTrigger::bind_arguments(TriggerWithArgsMessage& out,
    uint16_t collection_id,
    const DataView& arg0,
    const DataView& arg1,
    const DataView& arg2,
    const DataView& arg3,
    const DataView& arg4) {
    return bind_arguments(out, collection_id, {arg0, arg1, arg2, arg3, arg4});
}

bool
StandAloneTrigger::bind_arguments(TriggerWithArgsMessage& trigger_msg, uint16_t collection_id, const DataArgViews& args) {

    trigger_msg.collection_id = collection_id;

    // Fill arguments
    for (size_t i = 0; i < args.size(); ++i) {

        if (args[i].size() != 0 && args[i].data() != nullptr) {
            if (args[i].size() > trigger_msg.args[i].size()) {
                return false;
            }
            trigger_msg.arg_bitmap |= 1 << i;
            for (uint8_t i_val = 0; i_val< args[i].size() && i_val<TRIGGER_ARG_SIZE; ++i_val) {
                trigger_msg.args[i][i_val] = args[i].data()[i_val];
            }
        }
    }

    return true;
}

std::error_code
StandAloneTrigger::trigger(uint16_t collection_id, const DataArgViews& args) {
    return trigger_helper(stream_, collection_id, args);
}

std::error_code
StandAloneTrigger::trigger_helper(enyx::hw::c2a_stream& stream,
               uint16_t collection_id,
               const DataView& arg0,
               const DataView& arg1,
               const DataView& arg2,
               const DataView& arg3,
               const DataView& arg4) {
    return trigger_helper(stream, collection_id, {arg0, arg1, arg2, arg3, arg4});
}

inline
std::error_code
StandAloneTrigger::trigger_helper(enyx::hw::c2a_stream& stream, uint16_t collection_id, const DataArgViews& args) {

    TriggerWithArgsMessage trigger_msg;
    const bool binded = bind_arguments(trigger_msg, collection_id, args);
    if(not binded) {
        return std::make_error_code(std::errc::invalid_argument);
    }
    return stream.send(reinterpret_cast<const void*>(&trigger_msg), trigger_msg.header.length).error();
}

} // namespace demo
} // namespace hwstrat
} // namespace oe
} // namespace enyx
