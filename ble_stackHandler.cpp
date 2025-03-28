#include "ble_stackHandler.h"

#include "ble_globalValues.h"
#include "ble_initialization.h"
#include "ble_dataHandler.h"
#include "ts_globalValues.h"

/**************************************************************************/ /**
 * Bluetooth stack event handler
 * Called when an event happens on BLE the stack
 *
 * @param[in] evt Event coming from the Bluetooth stack
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt) {
    switch (SL_BT_MSG_ID(evt->header)) {
      // -------------------------------
      // This event indicates the device has started and the radio is ready.
      // Do not call any stack command before receiving this boot event!
      case sl_bt_evt_system_boot_id:
        {
          Serial.println("BLE stack booted");
  
          // Initialize the application specific GATT table
          ble_initialize_gatt_db();
  
          // Start advertising
          ble_start_advertising();
          Serial.println("BLE advertisement started");
        }
        break;
  
      // -------------------------------
      // This event indicates that a new connection was opened
      case sl_bt_evt_connection_opened_id:
        Serial.println("BLE connection opened");
        // Store the connection handle which will be needed for sending indications
        connection_handle = evt->data.evt_connection_opened.connection;
        break;
  
      // -------------------------------
      // This event indicates that a connection was closed
      case sl_bt_evt_connection_closed_id:
        connection_handle = 0u;
        indication_enabled = false;
        Serial.println("BLE connection closed");
        // delete[] txBuffer;
        // Restart the advertisement
        ble_start_advertising();
        Serial.println("BLE advertisement restarted");
        break;
  
      // -------------------------------
      // This event is received when a GATT characteristic status changes
      case sl_bt_evt_gatt_server_characteristic_status_id:
        // If the 'Data' characteristic has been changed
        if (evt->data.evt_gatt_server_characteristic_status.characteristic == ringdownData_characteristic_handle) {
          uint16_t client_config_flags = evt->data.evt_gatt_server_characteristic_status.client_config_flags;
          uint8_t status_flags = evt->data.evt_gatt_server_characteristic_status.status_flags;
          if ((client_config_flags == 0x02) && (status_flags == 0x01)) {
            // If indication was enabled (0x02) in the client config flags, and the status flag shows that it's a change
            Serial.println("Data indication enabled");
            indication_enabled = true;
          } else if ((client_config_flags == 0x00) && (status_flags == 0x01)) {
            // If indication was disabled (0x00) in the client config flags, and the status flag shows that it's a change
            Serial.println("Data indication disabled");
            indication_enabled = false;
          } else if (status_flags == 0x02) {  // Wait for client to ack indication received then send next indication
            Serial.println("Ack received");
            clientReadyForNextIndication = true;
            indicationIndex++;
            if (indicationIndex < numPoints) {
              handle_data_indication();
            } else {
              Serial.println("Indications sent!");
              indicationIndex = 0;
            }
          }
        }
        break;
  
      // -------------------------------
      // Default event handler
      default:
        break;
    }
}