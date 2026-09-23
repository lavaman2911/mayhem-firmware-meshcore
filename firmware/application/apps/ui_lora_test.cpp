#include "ui_lora_test.hpp"

#include "baseband_api.hpp"
#include "portapack.hpp"
#include "transmitter_model.hpp"
#include "receiver_model.hpp"
#include "radio.hpp"

#include <cstdio>
#include <cstring>

using namespace portapack;

namespace ui {

LoRaTestView::LoRaTestView(NavigationView& nav)
    : nav_{nav} {
    add_children({&labels_, &field_frequency, &rssi_, &channel_,
                  &field_sf, &field_bw, &field_cr, &field_lna, &field_vga,
                  &button_rx, &button_send, &text_status, &text_stats,
                  &text_meta, &text_hex0, &text_hex1, &text_hex2, &text_ascii});

    field_frequency.set_step(1000);
    field_frequency.set_value(869618000);

    field_sf.set_by_value(7);
    field_bw.set_by_value(62500);
    field_cr.set_by_value(8);
    field_lna.set_value(32);
    field_vga.set_value(20);

    field_frequency.on_change = [this](rf::Frequency) { apply_config(); };
    field_sf.on_change = [this](size_t, OptionsField::value_t) { start_rx(); };
    field_bw.on_change = [this](size_t, OptionsField::value_t) { start_rx(); };
    field_cr.on_change = [this](size_t, OptionsField::value_t) { start_rx(); };
    field_lna.on_change = [this](int32_t) { apply_config(); };
    field_vga.on_change = [this](int32_t) { apply_config(); };

    button_rx.on_select = [this](Button&) { start_rx(); };
    button_send.on_select = [this](Button&) { send_test_packet(); };

    start_rx();
}

void LoRaTestView::focus() {
    field_frequency.focus();
}

void LoRaTestView::apply_config() {
    receiver_model.set_target_frequency(field_frequency.value());
    receiver_model.set_sampling_rate(2'000'000);
    receiver_model.set_baseband_bandwidth(field_bw.selected_index_value());
    radio::set_lna_gain(field_lna.value());
    radio::set_vga_gain(field_vga.value());

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
    text_status.set("RX listening");
}

void LoRaTestView::send_test_packet() {
    receiver_model.disable();
    baseband::shutdown();

    static const uint8_t payload[] = "PORTAPACK LORA TEST";

    transmitter_model.set_target_frequency(field_frequency.value());
    transmitter_model.set_sampling_rate(2'500'000);
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

void LoRaTestView::format_hex_line(Text& field, const uint8_t* data, size_t len, size_t offset) {
    char line[31];
    size_t n = 0;
    for (size_t i = 0; i < 15 && (offset + i) < len && n + 2 < sizeof(line); i++) {
        static const char hex[] = "0123456789ABCDEF";
        const uint8_t b = data[offset + i];
        line[n++] = hex[b >> 4];
        line[n++] = hex[b & 0x0F];
    }
    line[n] = 0;
    field.set(line);
}

void LoRaTestView::on_packet(const LoRaPacketMessage& packet) {
    packet_count_++;
    if (packet.crc_state == LoRaPacketMessage::CRC_OK)
        crc_ok_++;
    else if (packet.crc_state == LoRaPacketMessage::CRC_BAD)
        crc_bad_++;

    const char* crc =
        packet.crc_state == LoRaPacketMessage::CRC_OK ? "OK" : packet.crc_state == LoRaPacketMessage::CRC_BAD ? "BAD"
                                                                                                              : "--";

    char stats[32];
    std::snprintf(stats, sizeof(stats), "RX#%lu CRC %s  ok%lu/bad%lu",
                  static_cast<unsigned long>(packet_count_), crc,
                  static_cast<unsigned long>(crc_ok_),
                  static_cast<unsigned long>(crc_bad_));
    text_stats.set(stats);

    const int snr = packet.snr_tenths / 10;
    char meta[32];
    std::snprintf(meta, sizeof(meta), "LEN %u  RSSI %d  SNR %d",
                  packet.length, packet.rssi, snr);
    text_meta.set(meta);

    format_hex_line(text_hex0, packet.data, packet.length, 0);
    format_hex_line(text_hex1, packet.data, packet.length, 15);
    format_hex_line(text_hex2, packet.data, packet.length, 30);

    char ascii[31];
    const size_t alen = packet.length < 30 ? packet.length : 30;
    for (size_t i = 0; i < alen; i++) {
        const char c = static_cast<char>(packet.data[i]);
        ascii[i] = (c >= 32 && c < 127) ? c : '.';
    }
    ascii[alen] = 0;
    text_ascii.set(ascii);

    text_status.set("LoRa PHY decoded");
}

LoRaTestView::~LoRaTestView() {
    receiver_model.disable();
    transmitter_model.disable();
    baseband::shutdown();
}

}  // namespace ui
