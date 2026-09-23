# MeshCore RX fix

## Root cause
At BW=62.5 kHz, SF7 uses the SF11 streaming decoder (`sf11_mode_=true`).
That path rejected every candidate unless payload bytes 0..3 were a
Meshtastic destination (`0xFFFFFFFF` or local node id). MeshCore never
matches that, so energy was seen but nothing decoded.

## Fix (in proc_lora.cpp sf11_try_decode)
When `local_node_id_ == 0` (LoRa Test / PHY mode), accept any valid
explicit header. Keep Meshtastic dest filtering only when a node id is set.
