// application.h
// Tartan Artibeus EXPT board flight 401 application header file
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

#ifndef APPLICATION_H
#define APPLICATION_H

// ta-expt library
#include <taolst_protocol.h> // rx_cmd_buff_t, tx_cmd_buff_t

// Macros

//// Application start address
#define APP_ADDR   ((uint32_t)0x08008000U)

//// SRAM1 start address
#define SRAM1_BASE ((uint32_t)0x20000000U)

//// SRAM1 size
#define SRAM1_SIZE ((uint32_t)0x00040000U)

//// Constants
#define HOUR_PER_DAY         ((uint8_t)24)            // hours per day
#define MIN_PER_HOUR         ((uint8_t)60)            // minutes per hour
#define SEC_PER_MIN          ((uint8_t)60)            // seconds per minute
#define NS_PER_SEC           ((uint32_t)1000000000)   // nanoseconds per sec
#define MIN_PER_DAY          ((uint16_t)1440)         // minutes per day
#define STR3_KE              ((float)0.743669161e-1f) // sqrt(GM) (Earth rad/min)^1.5
#define STR3_TWO_THIRDS      ((float)0.66666667f)     // two thirds
#define STR3_K2              ((float)5.413080e-4f)    // 0.5*J2*(eq. Earth rad.)^2
#define STR3_DU_PER_ER       ((float)1.0f)            // distance units / Earth radii
#define STR3_KM_PER_ER       ((float)6378.135f)       // kilometers per Earth radii
#define STR3_Q0              ((float)120.0f)          // density function parameter
#define STR3_S0              ((float)78.0f)           // density function parameter
#define STR3_A30             ((float)0.253881e-5f)    // -J3*(eq. Earth rad.)^3
#define STR3_K4              ((float)0.62098875e-6f)  // -0.375*J4*(eq. Earth rad.)^4
#define STR3_TWO_PI          ((float)6.2831853f)      // two pi
#define STR3_HALF_PI         ((float)1.57079633f)     // pi / 2
#define STR3_THREE_HALVES_PI ((float)4.71238898f)     // 3*pi/2
#define STR3_PI              ((float)3.14159265f)     // pi

// structs

//// tle struct
typedef struct tle {
  int16_t epoch_year;     // TLE epoch year
  float   epoch_day;      // TLE epoch day of year and fraction of day
  float   bstar;          // inverse Earth radians
  float   inclination;    // radians
  float   raan;           // right ascension of node in radians
  float   eccentricity;   // unitless
  float   arg_of_perigee; // radians
  float   mean_anomaly;   // radians
  float   mean_motion;    // radians per minute
} tle_t;

//// date and time struct
typedef struct date_time {
  int16_t  year;
  uint8_t  month;
  uint8_t  day;
  uint8_t  hour;
  uint8_t  minute;
  uint8_t  second;
  uint32_t nanosecond;
} date_time_t;

//// ECI coordinate frame (x,y,z) position in kilometers
typedef struct eci_posn {
  float x;
  float y;
  float z;
} eci_posn_t;

// Initialization functions

void init_clock(void);
void init_led(void);
void init_uart(void);
void init_rtc(void);

// Utility functions

/*  int set_rtc(const int32_t sec, const int32_t ns)
 *    sec: seconds since J2000
 *    ns:  any additional nanoseconds
 *  Return:
 *    0 to indicate failure
 *    Non-zero to indicate success
 */
int set_rtc(const uint32_t sec, const uint32_t ns);

/*  int get_rtc(int32_t* sec, int32_t* ns)
 *    sec: Upon return, contains RTC seconds since J2000
 *    ns:  Upon return, contains any additional nanoseconds
 *  Return:
 *    0 to indicate failure (e.g. RTC has not been set)
 *    Non-zero to indicate success (sec and ns contain valid values)
 */
int get_rtc(uint32_t* sec, uint32_t* ns);

/*  date_time_t get_date_time_rtc(void)
 *    void: no function parameters
 *  Return:
 *    date_time_t struct
 */
date_time_t get_date_time_rtc(void);

// Application functions

/*  tle_t parse_tle(char* start)
 *    start: pointer to first char of title line of TLE
 *  Return:
 *    tle_t struct with values parsed from the input characters
 */
tle_t parse_tle(char* start);

/*  int is_leap_year(const int16_t year)
 *    year: year to examine
 *  Return:
 *    0 to indicate NOT a leap year
 *    Non-zero to indicate IS a leap year
 */
int is_leap_year(const int16_t year);

/*  date_time_t get_tle_epoch(const tle_t* tle)
 *    tle: tle_t struct
 *  Return:
 *    date_time_t struct representing the TLE epoch
 */
date_time_t get_tle_epoch(const tle_t* tle);

/* uint32_t calc_julian_day_from_ymd(
 *  const int16_t year, const uint8_t month, const uint8_t day
 * )
 *    year:  Gregorian year
 *    month: Gregorian month
 *    day:   Gregorian day
 *  Return:
 *    uint32_t representing the Julian day
 */
uint32_t calc_julian_day_from_ymd(
 const int16_t year, const uint8_t month, const uint8_t day
);

/*  float calc_tdiff_minute(const date_time_t* event, const date_time_t* epoch)
 *    event: date_time_t struct representing the event of interest
 *    epoch: date_time_t struct representing the comparison point
 *  Return:
 *    float representing the minutes from epoch to event
 */
float calc_tdiff_minute(const date_time_t* event, const date_time_t* epoch);

/* eci_posn_t sgp4(
 *  const float bstar, const float i0, const float o0, const float e0,
 *  const float w0, const float m0, const float n0, const float tsince
 * )
 *    bstar:  inverse Earth radians
 *    i0:     inclination in radians
 *    o0:     right ascension of node in radians
 *    e0:     eccentricity (unitless)
 *    w0:     argument of perigee in radians
 *    m0:     mean anomaly in radians
 *    n0:     mean_motion in radians per minute
 *    tsince: minutes since TLE epoch
 *  Return:
 *    eci_posn_t struct representing the ECI position of the satellite
 */
eci_posn_t sgp4(
 const float bstar, const float i0, const float o0, const float e0,
 const float w0, const float m0, const float n0, const float tsince
);

// Task-like functions

void rx_usart1(rx_cmd_buff_t* rx_cmd_buff_o);
void reply(rx_cmd_buff_t* rx_cmd_buff_o, tx_cmd_buff_t* tx_cmd_buff_o);
void tx_usart1(tx_cmd_buff_t* tx_cmd_buff_o);

#endif
