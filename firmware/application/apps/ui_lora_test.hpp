#ifndef __LORA_TEST_APP_H__
#define __LORA_TEST_APP_H__

#include "ui_navigation.hpp"
#include "ui_freq_field.hpp"
#include "ui_receiver.hpp"
#include "receiver_model.hpp"
#include "radio.hpp"
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
    uint32_t packet_count_{0};
    uint32_t crc_ok_{0};
    uint32_t crc_bad_{0};
    bool rx_busy_{false};

    Labels labels_{{
        {{UI_POS_X(0), UI_POS_Y(1)}, "SF  BW     CR   LNA VGA", Color::light_grey()},
        {{UI_POS_X(0), UI_POS_Y(7)}, "RAW PHY payload (hex)", Color::light_grey()},
    }};

    RxFrequencyField field_frequency{{UI_POS_X(0), UI_POS_Y(0)}, nav_};
    RSSI rssi_{{UI_POS_X(21), UI_POS_Y(0), UI_POS_WIDTH_REMAINING(21), 4}};
    Channel channel_{{UI_POS_X(21), 5, UI_POS_WIDTH_REMAINING(21), 4}};

    OptionsField field_sf{{UI_POS_X(0), UI_POS_Y(2)}, 4,
                          {{"SF7", 7}, {"SF8", 8}, {"SF9", 9}, {"SF10", 10}, {"SF11", 11}}};
    OptionsField field_bw{{UI_POS_X(5), UI_POS_Y(2)}, 7,
                          {{"62.5k", 62500}, {"125k", 125000}, {"250k", 250000}, {"500k", 500000}}};
    OptionsField field_cr{{UI_POS_X(13), UI_POS_Y(2)}, 4,
                          {{"4/5", 5}, {"4/6", 6}, {"4/7", 7}, {"4/8", 8}}};
    LNAGainField field_lna{{UI_POS_X(18), UI_POS_Y(2)}};
    VGAGainField field_vga{{UI_POS_X(23), UI_POS_Y(2)}};

    Button button_rx{{UI_POS_X(0), UI_POS_Y(4), 9 * 8, 2 * 16}, "RX"};
    Button button_send{{UI_POS_X(10), UI_POS_Y(4), 9 * 8, 2 * 16}, "SEND"};

    Text text_status{{UI_POS_X(0), UI_POS_Y(6), screen_width, 16}, "Stopped"};
    Text text_stats{{UI_POS_X(0), UI_POS_Y(8), screen_width, 16}, "RX# 0  CRC -/-"};
    Text text_meta{{UI_POS_X(0), UI_POS_Y(9), screen_width, 16}, "LEN 0  RSSI --  SNR --"};
    Text text_hex0{{UI_POS_X(0), UI_POS_Y(10), screen_width, 16}, ""};
    Text text_hex1{{UI_POS_X(0), UI_POS_Y(11), screen_width, 16}, ""};
    Text text_hex2{{UI_POS_X(0), UI_POS_Y(12), screen_width, 16}, ""};
    Text text_ascii{{UI_POS_X(0), UI_POS_Y(13), screen_width, 16}, ""};

    void start_rx();
    void send_test_packet();
    void apply_config();
    void on_packet(const LoRaPacketMessage& packet);
    void format_hex_line(Text& field, const uint8_t* data, size_t len, size_t offset);

    MessageHandlerRegistration packet_handler{
        Message::ID::LoRaPacket,
        [this](Message* const message) {
            on_packet(*static_cast<const LoRaPacketMessage*>(message));
        }};

    MessageHandlerRegistration status_handler{
        Message::ID::LoRaRxStatus,
        [this](Message* const message) {
            const auto* status = static_cast<const LoRaRxStatusMessage*>(message);
            rx_busy_ = status->receiving;
            if (status->receiving)
                text_status.set("RX activity / preamble");
            else if (packet_count_ == 0)
                text_status.set("RX listening");
        }};
};

}  // namespace ui

#endif
