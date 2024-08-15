/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Jacek Fedorynski
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <tusb.h>

#include "config.h"
#include "globals.h"
#include "our_descriptor.h"
#include "platform.h"
#include "remapper.h"

// Vendor ID and Product ID for the ZUOYA GMK87 keyboard
constexpr uint16_t kUsbVid = 0x320F;  // Vendor ID
constexpr uint16_t kUsbPid = 0x5055;  // Product ID

// Device descriptor structure for USB
static tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,  // USB version (2.0)
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = kUsbVid,
    .idProduct = kUsbPid,
    .bcdDevice = 0x0105,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x00,  // no serial number
    .bNumConfigurations = 0x01,
};

// Configuration descriptors for the USB device
static const uint8_t configuration_descriptor0[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN, 0, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD, our_descriptors[0].descriptor_length, 0x81, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_HID_DESCRIPTOR(1, 0, HID_ITF_PROTOCOL_NONE, config_report_descriptor_length, 0x83, CFG_TUD_HID_EP_BUFSIZE, 1),
};

static const uint8_t configuration_descriptor1[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN, 0, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD, our_descriptors[1].descriptor_length, 0x81, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_HID_DESCRIPTOR(1, 0, HID_ITF_PROTOCOL_NONE, config_report_descriptor_length, 0x83, CFG_TUD_HID_EP_BUFSIZE, 1),
};

static const uint8_t configuration_descriptor2[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_HID_DESC_LEN, 0, 100),
    TUD_HID_INOUT_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, our_descriptors[2].descriptor_length, 0x02, 0x81, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_HID_DESCRIPTOR(1, 0, HID_ITF_PROTOCOL_NONE, config_report_descriptor_length, 0x83, CFG_TUD_HID_EP_BUFSIZE, 1),
};

static const uint8_t configuration_descriptor3[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN, 0, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, our_descriptors[3].descriptor_length, 0x81, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_HID_DESCRIPTOR(1, 0, HID_ITF_PROTOCOL_NONE, config_report_descriptor_length, 0x83, CFG_TUD_HID_EP_BUFSIZE, 1),
};

static const uint8_t configuration_descriptor4[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_HID_DESC_LEN, 0, 100),
    TUD_HID_INOUT_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, our_descriptors[4].descriptor_length, 0x02, 0x81, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_HID_DESCRIPTOR(1, 0, HID_ITF_PROTOCOL_NONE, config_report_descriptor_length, 0x83, CFG_TUD_HID_EP_BUFSIZE, 1),
};

static const uint8_t configuration_descriptor5[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN, 0, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, our_descriptors[5].descriptor_length, 0x81, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_HID_DESCRIPTOR(1, 0, HID_ITF_PROTOCOL_NONE, config_report_descriptor_length, 0x83, CFG_TUD_HID_EP_BUFSIZE, 1),
};

// Array of configuration descriptors
static const uint8_t* configuration_descriptors[] = {
    configuration_descriptor0,
    configuration_descriptor1,
    configuration_descriptor2,
    configuration_descriptor3,
    configuration_descriptor4,
    configuration_descriptor5,
};

// String descriptor array containing language and device information.
static const char* string_desc_arr[] = {
    (const char[]){ 0x09, 0x04 },  // 0: Supported language is English (0x0409)
    "ZUOYA",                       // 1: Manufacturer
    "ZUOYA GMK87 XXXX",            // 2: Product
};

static uint16_t _desc_str[32];

static const char id_chars[33] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

// Invoked when received GET STRING DESCRIPTOR request
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else if (index == 0xEE) {
        return nullptr;  // Handle the 0xEE index string accordingly
    } else {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
            return nullptr;  // Return nullptr for out of bounds index
        }

        const char* str = string_desc_arr[index];
        chr_count = strlen(str);
        if (chr_count > 31) {
            chr_count = 31;  // Cap at max char
        }

        // Convert ASCII string into UTF-16 format.
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = static_cast<uint16_t>(str[i]);
        }

        // Append unique ID characters to the string descriptor for product string.
        if (index == 2) {
            uint64_t unique_id = get_unique_id();  // Get unique ID
            for (uint8_t i = 0; i < 4; i++) {
                _desc_str[1 + chr_count - 4 + i] = id_chars[(unique_id >> (i * 5)) & 0x1F];
            }
        }
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);  // Set descriptor length

    return _desc_str;  // Return pointer to the string descriptor
}

// Callback function invoked when the host requests to get a report from the HID interface.
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    return (itf == 0) ? handle_get_report0(report_id, buffer, reqlen) : handle_get_report1(report_id, buffer, reqlen);
}

// Callback function invoked when the host sends a report to the HID interface.
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    if (itf == 0) {
        if ((report_id == 0) && (report_type == 0) && (bufsize > 0)) {
            report_id = buffer[0];  // Extract report ID
            buffer++;               // Move buffer pointer
        }
        handle_set_report0(report_id, buffer, bufsize);  // Handle for interface 0
    } else {
        handle_set_report1(report_id, buffer, bufsize);  // Handle for interface 1
    }
}

// Callback function invoked when the host sets the protocol for the HID interface.
void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol) {
    printf("tud_hid_set_protocol_cb %d %d\n", instance, protocol);  // Log the protocol change
    boot_protocol_keyboard = (protocol == HID_PROTOCOL_BOOT);       // Set boot protocol flag
    boot_protocol_updated = true;                                   // Mark boot protocol as updated
}

// Callback function invoked when the USB device is mounted.
void tud_mount_cb() {
    reset_resolution_multiplier();
    if (boot_protocol_keyboard) {
        boot_protocol_keyboard = false;
        boot_protocol_updated = true;
    }
}
