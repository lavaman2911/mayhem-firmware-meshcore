#include "ui_lora_test.hpp"

#include "baseband_api.hpp"
#include "portapack.hpp"
#include "transmitter_model.hpp"
#include "receiver_model.hpp"

using namespace portapack;

namespace ui {

LoRaTestView::LoRaTestView(NavigationView& nav)
    : nav_{nav} {
    add_children({&field_frequency, &field_sf, &field_bw, &field_cr,
                  &button_rx, &button_send, &text_status, &text_packet});

    field_frequency.set_step(100000);
    field_frequency.set_value(869525000);

    field_sf.set_by_value(7);
    field_bw.set_by_value(125000);
    field_cr.set_by_value(5);

    button_rx.on_select = [this](Button&) { start_rx(); };
    button_send.on_select = [this](Button&) { send_test_packet(); };

    start_rx();
}

void LoRaTestView::focus() {
    field_frequency.focus();
}

void LoRaTestView::apply_config() {
    receiver_model.set_target_frequency(field_frequency.value());
    receiver_model.set_sampling_rate(2'048'000);
    receiver_model.set_baseband_bandwidth(500'000);

    baseband::set_lora_config(
        static_cast<uint8_t>(field_sf.selected_index_value()),
        static_cast<uint32_t>(field_bw.selected_index_value()),
        static_cast<uint8_t>(field_cr.selected_index_value()),
        0);
}

void LoRaTestView::start_rx() {
    baseband::shutdown();
    baseband::run_image(portapack::spi_flash::image_tag_lora_rx);
    apply_config();
    receiver_model.enable();
    text_status.set("RX running");
}

void LoRaTestView::send_test_packet() {
    static const uint8_t payload[] = "PORTAPACK LORA TEST";

    transmitter_model.set_target_frequency(field_frequency.value());
    transmitter_model.set_sampling_rate(2'048'000);
    transmitter_model.set_baseband_bandwidth(500'000);
    transmitter_model.enable();

    baseband::shutdown();
    baseband::run_image(portapack::spi_flash::image_tag_lora_tx);
    baseband::set_lora_config(
        static_cast<uint8_t>(field_sf.selected_index_value()),
        static_cast<uint32_t>(field_bw.selected_index_value()),
        static_cast<uint8_t>(field_cr.selected_index_value()),
        0);
    baseband::send_lora_packet(payload, sizeof(payload) - 1);

    transmitter_model.disable();
    text_status.set("TX done");
    start_rx();
}

LoRaTestView::~LoRaTestView() {
    receiver_model.disable();
    transmitter_model.disable();
    baseband::shutdown();
}

}  // namespace ui
