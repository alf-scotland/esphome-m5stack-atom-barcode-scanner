#pragma once

#include <cstddef>
#include <cstdint>

namespace esphome {
namespace m5stack_barcode {

/// V2 (Atomic QRCode2 Base) UART protocol constants.
///
/// Packet format: TYPE (1 byte) + PID (1 byte) + FID (1 byte) [+ PARAM bytes]
///
///   TYPE 0x21 = Config Write  → scanner replies TYPE 0x22 (same bytes as error — see below)
///   TYPE 0x23 = Config Read   → scanner replies TYPE 0x24 + PID + FID + value bytes
///   TYPE 0x32 = Control       → some commands get TYPE 0x33 reply; start scan is fire-and-forget
///
/// FID upper 2 bits encode parameter length for Config Write / Read:
///   00 = no param, 01 = 1 byte, 10 = 2 bytes, 11 = length-prefixed
///
/// Barcode output: raw ASCII bytes with no framing, terminated by an inter-byte idle gap.
namespace CommandsV2 {

// ── Trigger mode (PID=0x61, FID=0x41) ────────────────────────────────────────
// Values
static const uint8_t TRIGGER_MODE_MANUAL = 0x00;  // manual / HOST equivalent
static const uint8_t TRIGGER_MODE_AUTO = 0x05;    // auto-sense / continuous equivalent

// Write trigger mode: {TYPE=0x21, PID=0x61, FID=0x41, value}
static const uint8_t SET_TRIGGER_MANUAL[] = {0x21, 0x61, 0x41, TRIGGER_MODE_MANUAL};
static const size_t SET_TRIGGER_MANUAL_SIZE = sizeof(SET_TRIGGER_MANUAL);

static const uint8_t SET_TRIGGER_AUTO[] = {0x21, 0x61, 0x41, TRIGGER_MODE_AUTO};
static const size_t SET_TRIGGER_AUTO_SIZE = sizeof(SET_TRIGGER_AUTO);

// Read trigger mode: {TYPE=0x23, PID=0x61, FID=0x41}
static const uint8_t READ_TRIGGER_MODE[] = {0x23, 0x61, 0x41};
static const size_t READ_TRIGGER_MODE_SIZE = sizeof(READ_TRIGGER_MODE);

// ── Scan control (PID=0x75) ───────────────────────────────────────────────────
// Start scan — fire-and-forget, no reply: {TYPE=0x32, PID=0x75, 0x01}
static const uint8_t START_SCAN[] = {0x32, 0x75, 0x01};
static const size_t START_SCAN_SIZE = sizeof(START_SCAN);

// Stop scan — scanner replies {0x33, 0x75, 0x02}: {TYPE=0x32, PID=0x75, 0x02}
static const uint8_t STOP_SCAN[] = {0x32, 0x75, 0x02};
static const size_t STOP_SCAN_SIZE = sizeof(STOP_SCAN);

// ── Device control (PID=0x76) ─────────────────────────────────────────────────
// Factory reset — fire-and-forget: {TYPE=0x32, PID=0x76, 0x01}
static const uint8_t FACTORY_RESET[] = {0x32, 0x76, 0x01};
static const size_t FACTORY_RESET_SIZE = sizeof(FACTORY_RESET);

// ── Known response byte sequences ─────────────────────────────────────────────
namespace Responses {

// Trigger mode read reply: {0x24, 0x61, 0x41, <value>} — 4 bytes total
static const uint8_t TRIGGER_MODE_PREFIX[] = {0x24, 0x61, 0x41};
static const size_t TRIGGER_MODE_PREFIX_SIZE = sizeof(TRIGGER_MODE_PREFIX);
static const size_t TRIGGER_MODE_TOTAL = TRIGGER_MODE_PREFIX_SIZE + 1;

// Stop-scan / failed-decode reply: {0x33, 0x75, 0x02}
static const uint8_t STOP_OR_FAILED[] = {0x33, 0x75, 0x02};
static const size_t STOP_OR_FAILED_SIZE = sizeof(STOP_OR_FAILED);

// Config-write reply (TYPE=0x22): signals ACK or error depending on context.
// The scanner echoes PID+FID unchanged; the M5Stack library does not distinguish
// ACK from error at the byte level.
static const uint8_t CONFIG_WRITE_REPLY_TYPE = 0x22;

}  // namespace Responses

}  // namespace CommandsV2

}  // namespace m5stack_barcode
}  // namespace esphome
