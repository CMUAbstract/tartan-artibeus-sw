// application.h
// Tartan Artibeus EXPT board application header file
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

#ifndef APPLICATION_H
#define APPLICATION_H

// Standard library
#include <stdint.h>          // uint8_t, uint32_t

// ta-expt library
#include <taolst_protocol.h> // rx_cmd_buff_t

// Macros

//// Byte counts
#define BYTES_PER_WORD ((uint32_t)4)
#define BYTES_PER_CMD  ((uint32_t)128)
#define BYTES_PER_PAGE ((uint32_t)2048)

// Variables

extern const uint8_t SUBPAGE_00[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_01[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_02[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_03[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_04[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_05[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_06[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_07[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_08[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_09[BYTES_PER_CMD];
extern const uint8_t SUBPAGE_0A[BYTES_PER_CMD];

// Functions

//// Fills rx_cmd_buff with bootloader_write_data cmd given page ID and data
void set_bootloader_write_data_cmd(
 rx_cmd_buff_t* rx_cmd_buff_o,
 const uint8_t subpage_id,
 const uint8_t* subpage_data
);

//// Calls flash write commands to write the data
int bootloader_write_data(rx_cmd_buff_t* rx_cmd_buff);

#endif
