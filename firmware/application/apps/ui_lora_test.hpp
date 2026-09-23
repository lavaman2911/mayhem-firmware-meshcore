#ifndef __LORA_TEST_APP_H__
#define __LORA_TEST_APP_H__

#include "ui_navigation.hpp"
#include "ui_freq_field.hpp"
#include "receiver_model.hpp"
#include "message.hpp"

namespace ui {

class LoRaTestView : public View {
public:
    LoRaTestView(NavigationView& nav);
    ~LoRaTestView();

    void focus() override;
    std::string title() const override { return "LoRa Test"; }

private:
    NavigationView& nav_;

    RxFrequencyField field_frequency{{UI_POS_X(0), UI_POS_Y(0)}, nav_};
    OptionsField field_sf{{UI_POS_X(0), UI_POS_Y(2)}, 4,
                          {{"SF7", 7}, {"SF8", 8}, {"SF9", 9}, {"SF10", 10},
                           {"SF11", 11}}};
    OptionsField field_bw{{UI_POS_X(8), UI_POS_Y(2)}, 7,
                          {{"62.5k", 62500}, {"125k", 125000}, {"250k", 250000}, {"500k", 500000}}};
    OptionsField field_cr{{UI_POS_X(19), UI_POS_Y(2)}, 4,
                          {{"4/5", 5}, {"4/6", 6}, {"4/7", 7}, {"4/8", 8}}};

    Button button_rx{{UI_POS_X(0), UI_POS_Y(4), 9 * 8, 2 * 16}, "RX"};
    Button button_send{{UI_POS_X(10), UI_POS_Y(4), 9 * 8, 2 * 16}, "SEND"};
    Text text_status{{UI_POS_X(0), UI_POS_Y(7), screen_width, 16}, "Stopped"};
    Text text_packet{{UI_POS_X(0), UI_POS_Y(8), screen_width, 32}, ""};

    uint32_t packet_count_{0};

    void start_rx();
    void send_test_packet();
    void apply_config();

    MessageHandlerRegistration packet_handler{
        Message::ID::LoRaPacket,
        [this](Message* const message) {
            const auto* packet = static_cast<const LoRaPacketMessage*>(message);
            packet_count_++;
            text_packet.set("RX #" + std::to_string(packet_count_) +
                            " LEN " + std::to_string(packet->length) +
                            " RSSI " + std::to_string(packet->rssi) +
                            " SNR " + std::to_string(packet->snr_tenths / 10.0f));
            text_status.set("Packet received");
        }};
};

}  // namespace ui

#endif
