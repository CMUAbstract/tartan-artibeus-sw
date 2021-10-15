// application.c
// Tartan Artibeus EXPT board flight 401 application implementation file
//
// Written by Bradley Denby
// Other contributors: Shize Che
//
// See the top-level LICENSE file for the license.

// Standard library
#include <math.h>                 // floor,round,pow,cos,sqrt,sin,fmod,atan,fabs
#include <stdint.h>               // uint8_t,uint16_t,uint32_t,int16_t,int32_t
#include <stdlib.h>               // atoi, atof

// libopencm3 library
#include <libopencm3/cm3/scb.h>     // SCB_VTOR
#include <libopencm3/stm32/flash.h> // used in init_clock
#include <libopencm3/stm32/gpio.h>  // used in init_gpio
#include <libopencm3/stm32/pwr.h>   // used in set_rtc
#include <libopencm3/stm32/rcc.h>   // used in init_clock, init_rtc
#include <libopencm3/stm32/rtc.h>   // used in rtc functions
#include <libopencm3/stm32/usart.h> // used in init_uart

// ta-expt library
#include <application.h>            // Header file
#include <taolst_protocol.h>        // TAOLST protocol macros, typedefs, fnctns

// Variables
int rtc_set = 0; // Boolean; Zero until RTC date and time have been set

// Initialization functions

void init_clock(void) {
  rcc_osc_on(RCC_HSI16);                    // 16 MHz internal RC oscillator
  rcc_wait_for_osc_ready(RCC_HSI16);        // Wait until oscillator is ready
  rcc_set_sysclk_source(RCC_CFGR_SW_HSI16); // Sets sysclk source for RTOS
  rcc_set_hpre(RCC_CFGR_HPRE_NODIV);        // AHB at 80 MHz (80 MHz max.)
  rcc_set_ppre1(RCC_CFGR_PPRE1_DIV2);       // APB1 at 40 MHz (80 MHz max.)
  rcc_set_ppre2(RCC_CFGR_PPRE2_NODIV);      // APB2 at 80 MHz (80 MHz max.)
  //flash_prefetch_enable();                  // Enable instr prefetch buffer
  flash_set_ws(FLASH_ACR_LATENCY_4WS);      // RM0351: 4 WS for 80 MHz
  //flash_dcache_enable();                    // Enable data cache
  //flash_icache_enable();                    // Enable instruction cache
  rcc_set_main_pll(                         // Setup 80 MHz clock
   RCC_PLLCFGR_PLLSRC_HSI16,                // PLL clock source
   4,                                       // PLL VCO division factor
   40,                                      // PLL VCO multiplication factor
   0,                                       // PLL P clk output division factor
   0,                                       // PLL Q clk output division factor
   RCC_PLLCFGR_PLLR_DIV2                    // PLL sysclk output division factor
  ); // 16MHz/4 = 4MHz; 4MHz*40 = 160MHz VCO; 160MHz/2 = 80MHz PLL
  rcc_osc_on(RCC_PLL);                      // 80 MHz PLL
  rcc_wait_for_osc_ready(RCC_PLL);          // Wait until PLL is ready
  rcc_set_sysclk_source(RCC_CFGR_SW_PLL);   // Sets sysclk source for RTOS
  rcc_wait_for_sysclk_status(RCC_PLL);
  rcc_ahb_frequency = 80000000;
  rcc_apb1_frequency = 40000000;
  rcc_apb2_frequency = 80000000;
}

void init_led(void) {
  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO10);
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);
  gpio_set(GPIOC, GPIO10);
  gpio_clear(GPIOC, GPIO12);
}

void init_uart(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);
  gpio_mode_setup(GPIOA,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO9|GPIO10);
  gpio_set_af(GPIOA,GPIO_AF7,GPIO9);  // USART1_TX is alternate function 7
  gpio_set_af(GPIOA,GPIO_AF7,GPIO10); // USART1_RX is alternate function 7
  usart_set_baudrate(USART1,115200);
  usart_set_databits(USART1,8);
  usart_set_stopbits(USART1,USART_STOPBITS_1);
  usart_set_mode(USART1,USART_MODE_TX_RX);
  usart_set_parity(USART1,USART_PARITY_NONE);
  usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);
  usart_enable(USART1);
}

void init_rtc(void) {
  rcc_osc_on(RCC_LSI);               // Low-speed internal oscillator
  rcc_wait_for_osc_ready(RCC_LSI);   // Wait until oscillator is ready
  rcc_periph_clock_enable(RCC_PWR);  // Enable power interface clock for the RTC
  pwr_disable_backup_domain_write_protect();
  rcc_set_rtc_clock_source(RCC_LSI); // Set RTC source
  rcc_enable_rtc_clock();            // Enable RTC
  pwr_enable_backup_domain_write_protect();
  rtc_set = 0;                       // RTC date and time has not yet been set
}

// Utility functions

int set_rtc(const uint32_t sec, const uint32_t ns) {
  // sec and ns represent time since J2000
  //   J2000 UTC: 2000-01-01 11:58:55.816
  // modify sec and ns to indicate time since 2000-01-01 11:58:56
  //   this modification makes the later math simpler
  uint32_t nanosecond  = ns;
  uint32_t sec_rounded = sec;
  if(nanosecond>=184000000) {
    nanosecond -= 184000000;
  } else {
    nanosecond = 1000000000-(184000000-nanosecond);
    if(sec==0) {
      return 0; // refuse to handle this case
    } else {
      sec_rounded -= 1;
    }
  }
  // sec_rounded represents seconds since 2000-01-01 11:58:56
  //   J2000 Julian date: 2451545
  // modify sec_rounded to indicate seconds since 2000-01-01 00:00:00
  //   (Julian date ~2451544.5)
  //   this modification makes the later math simpler
  uint32_t sec_since_y2k = sec_rounded+43136;
  // calculate whole days since Julian date ~2451544.5
  int32_t day_since_y2k = (int32_t)(sec_since_y2k/86400);
  // track the leftover seconds
  uint32_t remaining_sec = sec_since_y2k%86400;
  // calculate the Julian date omitting any remaining_sec
  //   if day_since_y2k==0, jd should be 2451545 b/c remaining_sec<86400
  int32_t jd = 2451545+day_since_y2k;
  // convert jd into year, month, and day (see fliegel1968letters)
  int32_t l = jd+68596;
  int32_t n = 4*l/146097;
  l = l-(146097*n+3)/4;
  int32_t i = 4000*(l+1)/1461001;
  l = l-1461*i/4+31;
  int32_t j = 80*l/2447;
  int32_t k = l-2447*j/80;
  l = j/11;
  j = j+2-12*l;
  i = 100*(n-49)+i+l;
  // convert into uint8_t forms expected by RTC
  uint8_t year = (uint8_t)(i-2000);
  uint8_t month = (uint8_t)(j);
  uint8_t day = (uint8_t)(k);
  // convert remaining_sec into hour, minute, second
  uint8_t hour = (uint8_t)(remaining_sec/3600);
  uint8_t minute = (uint8_t)((remaining_sec%3600)/60);
  uint8_t second = (uint8_t)((remaining_sec%3600)%60);
  // set the RTC
  pwr_disable_backup_domain_write_protect();
  rtc_wait_for_synchro();
  rtc_unlock();
  rtc_set_init_flag();
  rtc_wait_for_init_ready();
  rtc_set_prescaler((uint32_t)249,(uint32_t)127);
  rtc_enable_bypass_shadow_register();
  rtc_calendar_set_year(year);
  rtc_calendar_set_month(month);
  rtc_calendar_set_day(day);
  rtc_set_am_format();
  rtc_time_set_time(hour,minute,second,1);
  rtc_clear_init_flag();
  rtc_lock();
  pwr_enable_backup_domain_write_protect();
  // record and return success
  rtc_set = 1;
  return rtc_set;
}

int get_rtc(uint32_t* sec, uint32_t* ns) {
  if(rtc_set) {
    // Read all values "atomically"
    int32_t year = (int32_t)(((RTC_DR>>20)*10)+((RTC_DR>>16)&0xf)+2000);
    int32_t month = (int32_t)((((RTC_DR>>12)&0x1)*10)+((RTC_DR>>8)&0xf));
    int32_t day = (int32_t)((((RTC_DR>>4)&0x3)*10)+(RTC_DR&0xf));
    int32_t hour = (int32_t)((((RTC_TR>>20)&0x3)*10)+((RTC_TR>>16)&0xf));
    int32_t minute = (int32_t)((((RTC_TR>>12)&0x7)*10)+((RTC_TR>>8)&0xf));
    int32_t second = (int32_t)((((RTC_TR>>4)&0x7)*10)+(RTC_TR&0xf));
    // Calculate JD from year, month, day
    int32_t jd =
     day-32075+1461*(year+4800+(month-14)/12)/4
     +367*(month-2-(month-14)/12*12)/12-3
     *((year+4900+(month-14)/12)/100)/4;
    // Convert into seconds since 2000-01-01 11:58:56
    *sec = (uint32_t)(86400*(jd-2451545)+60*(60*hour+minute)+second-43136);
    // Write nanoseconds
    *ns = (uint32_t)(184000000);
  }
  return rtc_set;
}

date_time_t get_date_time_rtc(void) {
  date_time_t now = {
   .year       = 0,
   .month      = 0,
   .day        = 0,
   .hour       = 0,
   .minute     = 0,
   .second     = 0,
   .nanosecond = 0
  };
  now.year = (int16_t)(((RTC_DR>>20)*10)+((RTC_DR>>16)&0xf)+2000);
  now.month = (uint8_t)((((RTC_DR>>12)&0x1)*10)+((RTC_DR>>8)&0xf));
  now.day = (uint8_t)((((RTC_DR>>4)&0x3)*10)+(RTC_DR&0xf));
  now.hour = (uint8_t)((((RTC_TR>>20)&0x3)*10)+((RTC_TR>>16)&0xf));
  now.minute = (uint8_t)((((RTC_TR>>12)&0x7)*10)+((RTC_TR>>8)&0xf));
  now.second = (uint8_t)((((RTC_TR>>4)&0x7)*10)+(RTC_TR&0xf));
  now.nanosecond = (uint32_t)(184000000);
  return now;
}

// Application functions

tle_t parse_tle(char* start) {
  // initialize output struct
  tle_t tle = {
   .epoch_year     = 0,
   .epoch_day      = 0.0f,
   .bstar          = 0.0f,
   .inclination    = 0.0f,
   .raan           = 0.0f,
   .eccentricity   = 0.0f,
   .arg_of_perigee = 0.0f,
   .mean_anomaly   = 0.0f,
   .mean_motion    = 0.0f
  };
  // epoch year
  char yy_buff[3] = {
   *(start+42),
   *(start+43),
   '\0'
  };
  int16_t yy = (int16_t)(atoi(yy_buff));
  int16_t year = 2000+yy;
  if(yy>=57) {
    year = 1900+yy;
  }
  tle.epoch_year = year;
  // epoch day
  char ddd_buff[13] = {
   *(start+44),
   *(start+45),
   *(start+46),
   *(start+47),
   *(start+48),
   *(start+49),
   *(start+50),
   *(start+51),
   *(start+52),
   *(start+53),
   *(start+54),
   *(start+55),
   '\0'
  };
  tle.epoch_day = (float)(atof(ddd_buff));
  // bstar
  char bstar_buff[12] = {
   *(start+77),
   '0',
   '.',
   *(start+78),
   *(start+79),
   *(start+80),
   *(start+81),
   *(start+82),
   'e',
   *(start+83),
   *(start+84),
   '\0'
  };
  tle.bstar = (float)(atof(bstar_buff));
  // inclination
  char inclination_buff[9] = {
   *(start+101),
   *(start+102),
   *(start+103),
   *(start+104),
   *(start+105),
   *(start+106),
   *(start+107),
   *(start+108),
   '\0'
  };
  tle.inclination = (float)(atof(inclination_buff));
  // raan
  char raan_buff[9] = {
   *(start+110),
   *(start+111),
   *(start+112),
   *(start+113),
   *(start+114),
   *(start+115),
   *(start+116),
   *(start+117),
   '\0'
  };
  tle.raan = (float)(atof(raan_buff));
  // eccentricity
  char eccentricity_buff[10] = {
   '0',
   '.',
   *(start+119),
   *(start+120),
   *(start+121),
   *(start+122),
   *(start+123),
   *(start+124),
   *(start+125),
   '\0'
  };
  tle.eccentricity = (float)(atof(eccentricity_buff));
  // arg of perigee
  char arg_of_perigee_buff[9] = {
   *(start+127),
   *(start+128),
   *(start+129),
   *(start+130),
   *(start+131),
   *(start+132),
   *(start+133),
   *(start+134),
   '\0'
  };
  tle.arg_of_perigee = (float)(atof(arg_of_perigee_buff));
  // mean anomaly
  char mean_anomaly_buff[9] = {
   *(start+136),
   *(start+137),
   *(start+138),
   *(start+139),
   *(start+140),
   *(start+141),
   *(start+142),
   *(start+143),
   '\0'
  };
  tle.mean_anomaly = (float)(atof(mean_anomaly_buff));
  // mean motion
  char mean_motion_buff[12] = {
   *(start+145),
   *(start+146),
   *(start+147),
   *(start+148),
   *(start+149),
   *(start+150),
   *(start+151),
   *(start+152),
   *(start+153),
   *(start+154),
   *(start+155),
   '\0'
  };
  tle.mean_motion = (float)(atof(mean_motion_buff));
  // return result
  return tle;
}

int is_leap_year(const int16_t year) {
  return ((year%400==0) || (year%100!=0 && year%4==0));
}

date_time_t get_tle_epoch(const tle_t* tle) {
  // initialize output struct
  date_time_t tle_epoch = {
   .year       = 0,
   .month      = 0,
   .day        = 0,
   .hour       = 0,
   .minute     = 0,
   .second     = 0,
   .nanosecond = 0
  };
  // year
  tle_epoch.year = tle->epoch_year;
  // month
  uint16_t day_per_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if(is_leap_year(tle_epoch.year)) {
    day_per_month[1]+=1;
  }
  float ddd = tle->epoch_day;
  uint16_t doy = (uint16_t)(floor(ddd)); // day of year
  uint8_t month = 1;
  uint16_t excess = 0;
  uint16_t thresh = day_per_month[0];
  for(size_t i=1; thresh<doy && i<12; i++) {
    month+=1;
    excess = thresh;
    thresh+=day_per_month[i];
  }
  tle_epoch.month = month;
  // day
  uint16_t day16 = doy;
  if(month!=1) {
    day16-=excess;
  }
  uint8_t day = (uint8_t)(day16);
  tle_epoch.day = day;
  // hour
  float hh = ddd-(float)(doy);
  uint8_t hour = (uint8_t)(floor(hh*((float)(HOUR_PER_DAY))));
  tle_epoch.hour = hour;
  // minute
  float mm = hh*((float)(HOUR_PER_DAY))-floor(hh*((float)(HOUR_PER_DAY)));
  uint8_t minute = (uint8_t)(floor(mm*((float)(MIN_PER_HOUR))));
  tle_epoch.minute = minute;
  // second
  double ss = mm*((float)(MIN_PER_HOUR))-floor(mm*((float)(MIN_PER_HOUR)));
  uint8_t second = (uint8_t)(floor(ss*((float)(SEC_PER_MIN))));
  tle_epoch.second = second;
  // nanosecond
  float ns = ss*((float)(SEC_PER_MIN))-floor(ss*((float)(SEC_PER_MIN)));
  uint32_t nanosecond = (uint32_t)(round(ns*((float)(NS_PER_SEC))));
  tle_epoch.nanosecond = nanosecond;
  // return result
  return tle_epoch;
}

uint32_t calc_julian_day_from_ymd(
 const int16_t year, const uint8_t month, const uint8_t day
) {
  const int32_t JD =
   (int32_t)(day)-32075+1461*(
    (int32_t)(year)+4800+((int32_t)(month)-14)/12
   )/4+367*(
    (int32_t)(month)-2-((int32_t)(month)-14)/12*12
   )/12-3*(
    ((int32_t)(year)+4900+((int32_t)(month)-14)/12)/100
   )/4;
  return JD;
}

float calc_tdiff_minute(const date_time_t* event, const date_time_t* epoch) {
  uint32_t event_jd = calc_julian_day_from_ymd(
   event->year, event->month, event->day
  );
  uint32_t event_sc =
   (uint32_t)(event->second)+
   (uint32_t)(SEC_PER_MIN)*(
    (uint32_t)(event->minute)+
    (uint32_t)(MIN_PER_HOUR)*(uint32_t)(event->hour)
   );
  uint32_t event_ns = event->nanosecond;
  uint32_t epoch_jd = calc_julian_day_from_ymd(
   epoch->year, epoch->month, epoch->day
  );
  uint32_t epoch_sc =
   (uint32_t)(epoch->second)+
   (uint32_t)(SEC_PER_MIN)*(
    (uint32_t)(epoch->minute)+
    (uint32_t)(MIN_PER_HOUR)*(uint32_t)(epoch->hour)
   );
  uint32_t epoch_ns = epoch->nanosecond;
  return
   (event_jd-epoch_jd)*(float)(MIN_PER_DAY)+
   (
    (float)(event_sc)-(float)(epoch_sc)
   )/(float)(SEC_PER_MIN)+
   (
    (float)(event_ns)-(float)(epoch_ns)
   )/(
    (float)(NS_PER_SEC)*
    (float)(SEC_PER_MIN)
   );
}

eci_posn_t sgp4(
 const float bstar, const float i0, const float o0, const float e0,
 const float w0, const float m0, const float n0, const float tsince
) {
  // Recover mean motion and semimajor axis     // line001-line013 boilerplate
  const float a1 =                                             // eq01,line014
   pow(STR3_KE/n0,STR3_TWO_THIRDS);
  const float cosi0 = cos(i0);                                 //      line015
  const float thetar2 = cosi0*cosi0;                           // eq10,line016
  const float thetar2t3m1 = thetar2*3.0f-1.0f;       // X3THM1 //      line017
                                                               // omit line018
  const float beta0r2 = 1.0f-e0*e0;                            //      line019
  const float beta0 = sqrt(beta0r2);                           // eq12,line020
  const float delta1 =                                         // eq02,line021
   1.5f*STR3_K2*thetar2t3m1/(a1*a1*beta0*beta0r2);
  const float a0 =                                             // eq03,line022
   a1*(
    1.0f-0.5f*STR3_TWO_THIRDS*delta1-delta1*delta1-
    (134.0f/81.0f)*delta1*delta1*delta1
   );
  const float delta0 =                                         // eq04,line023
   1.5f*STR3_K2*thetar2t3m1/(a0*a0*beta0*beta0r2);
  const float n0pp = n0/(1.0f+delta0);                         // eq05,line024
  const float a0pp = a0/(1.0f-delta0);                         // eq06,line025
  // Check if perigee height is less than 220 km   // line026-line033 comments
  bool isimp = false;                                          //      line034
  // a0pp*(1.0f-e0)/ae and 220.0f/kmper+ae are distances to Earth center
  if(
   a0pp*(1.0f-e0)/STR3_DU_PER_ER <
   (220.0f/STR3_KM_PER_ER+STR3_DU_PER_ER)
  ) {
    isimp = true;                                              //      line035
  }                                                // line036-line039 comments
  // Set constants based on perigee height
  float q0msr4temp =                                 // QOMS2T // dr26,line041
   pow(
    (STR3_Q0-STR3_S0)*STR3_DU_PER_ER/STR3_KM_PER_ER,
    4.0f
   );
  float stemp =                                           // S // dr27,line040
   STR3_DU_PER_ER*(1.0f+STR3_S0/STR3_KM_PER_ER);
  const float perigee =                                        //      line042
   (a0pp*(1.0f-e0)-STR3_DU_PER_ER)*STR3_KM_PER_ER;
  if(perigee <= 98.0f) {                                       //      line045
    stemp = 20.0f;                                             //      line046
    q0msr4temp =                                               //      line047
     pow(
      (STR3_Q0-stemp)*STR3_DU_PER_ER/STR3_KM_PER_ER,
      4.0f
     );
    stemp = stemp/STR3_KM_PER_ER+STR3_DU_PER_ER;               // eq08,line048
  } else if(perigee < 156.0f) {                                //      line043
    stemp = perigee-STR3_S0;                                   //      line044
    q0msr4temp =                                               //      line047
     pow(
      (STR3_Q0-stemp)*STR3_DU_PER_ER/STR3_KM_PER_ER,
      4.0f
     );
    stemp = stemp/STR3_KM_PER_ER+STR3_DU_PER_ER;               // eq07,line048
  }
  const float q0msr4 = q0msr4temp;                   // QOMS24 // eq09,line041
  const float s = stemp;                                 // S4 //      line040
  const float xi = 1.0f/(a0pp-s);                       // TSI // eq11,line050
  const float eta = a0pp*e0*xi;                                // eq13,line051
  const float etar2 = eta*eta;                                 //      line052
  const float e0eta = e0*eta;                          // EETA //      line053
  const float psi = fabs(1.0f-etar2);                 // PSISQ //      line054
  const float q0msr4txir4 = q0msr4*pow(xi,4.0f);       // COEF //      line055
  const float q0msr4txir4dpsir3p5 =                   // COEF1 //      line056
   q0msr4txir4/pow(psi,3.5f);
  const float c2 =                                             // eq14,line057
   q0msr4txir4dpsir3p5*n0pp*(                                  // thru line058
    a0pp*(1.0f+1.5f*etar2+4.0f*e0eta+e0eta*etar2)+
    0.75f*STR3_K2*xi/psi*thetar2t3m1*(8.0f+24.0f*etar2+3.0f*etar2*etar2)
   );
  const float c1 = bstar*c2;                                   // eq15,line059
  const float sini0 = sin(i0);                                 //      line060
  const float a30dk2 =                               // A3OVK2 //omit? line061
   STR3_A30/STR3_K2*pow(STR3_DU_PER_ER,3.0f);
  const float c3 =                                             // eq16,line062
   q0msr4txir4*xi*a30dk2*n0pp*STR3_DU_PER_ER*sini0/e0;
  const float nthetar2a1 = -1.0f*thetar2+1.0f;       // X1MTH2 //      line063
  const float c4 =                                             // eq17,line064
   2.0f*n0pp*q0msr4txir4dpsir3p5*a0pp*beta0r2*(                // thru line068
    (2.0f*eta*(1.0f+e0eta)+0.5f*e0+0.5f*eta*etar2)-
    (2.0f*STR3_K2*xi/(a0pp*psi))*
    (
     -3.0f*thetar2t3m1*(1.0f+1.5f*etar2-2.0f*e0eta-0.5f*e0eta*etar2)+
     0.75f*nthetar2a1*(2.0f*etar2-e0eta-e0eta*etar2)*cos(2.0f*w0)
    )
   );
  const float thetar4 = thetar2*thetar2;                       //      line070
  const float pinvsq = 1.0f/(a0pp*a0pp*beta0r2*beta0r2);       //      line049
  const float k2m3pinvsqn0pp = STR3_K2*3.0f*pinvsq*n0pp;       //TEMP1 line071
  const float k2r2m3pinvsqr2n0pp =                    // TEMP2 //      line072
   k2m3pinvsqn0pp*STR3_K2*pinvsq;
  const float k4m1p25pinvsqr2n0pp =                   // TEMP3 //      line073
   STR3_K4*1.25f*pinvsq*pinvsq*n0pp;
  const float mdt =                                            //      line074
   n0pp+k2m3pinvsqn0pp*thetar2t3m1*beta0*0.5f+                 // thru line075
   k2r2m3pinvsqr2n0pp*(13.0f-78.0f*thetar2+137.0f*thetar4)*beta0*0.0625f;
  const float n5thetar2a1 = -5.0f*thetar2+1.0f;      // X1M5TH //      line076
  const float wdt =                                            //      line077
   k2m3pinvsqn0pp*n5thetar2a1*-0.5f+                           // thru line078
   k2r2m3pinvsqr2n0pp*(7.0f-114.0f*thetar2+395.0f*thetar4)*0.0625f+
   k4m1p25pinvsqr2n0pp*(3.0f-36.0f*thetar2+49.0f*thetar4);
                                                               // omit line079
  const float odt =                                            //      line080
   -1.0f*k2m3pinvsqn0pp*cosi0+                                 // thru line081
   k2r2m3pinvsqr2n0pp*(4.0f*cosi0-19.0f*cosi0*thetar2)*0.5f+
   k4m1p25pinvsqr2n0pp*cosi0*(3.0f-7.0f*thetar2)*2.0f;
  const float mdf = m0+mdt*tsince;                             // eq22,line105
  const float wdf = w0+wdt*tsince;                             // eq23,line106
  const float odf = o0+odt*tsince;                             // eq24,line107
                                                               // omit line082
                                                               // omit line083
  const float uo = -3.5f*beta0r2*k2m3pinvsqn0pp*cosi0*c1;      // l79+ line084
                                                               // omit line085
                                                               // omit line088
                                                               // omit line089
                                                               // omit line090
  const float tsincer2 = tsince*tsince;                        //      line110
  float mptemp = mdf;                                          //      line109
  float wtemp = wdf;                                           //      line108
  float uatemp = 1.0f-c1*tsince;                      // TEMPA //      line112
  float uetemp = bstar*c4*tsince;                     // TEMPE //      line113
  float ultemp = 1.5f*c1*tsincer2;                    // TEMPL // l85+ line114
  if(!isimp) {                                              // line091,line115
    const float c1r2 = c1*c1;                                  //      line092
    const float c5 =                                           // eq18,line069
     2.0f*q0msr4txir4dpsir3p5*a0pp*beta0r2*(
      1.0f+2.75f*(etar2+e0eta)+e0eta*etar2
     );
    const float d2 = 4.0f*a0pp*xi*c1r2;                        // eq19,line093
                                                               // omit line094
    const float d3 = d2*xi*c1*(17.0f*a0pp+s)/3.0f;             // eq20,line095
    const float d4 =                                           // eq21,line096
     0.5f*d2*xi*xi*c1r2*a0pp*(221.0f*a0pp+31.0f*s)/3.0f;
    const float deltaw =                                       // eq25,line116
     bstar*c3*cos(w0)*tsince;                                  // l82+
    const float deltam =                                       // eq26,line117
     -1.0f*STR3_TWO_THIRDS*q0msr4txir4*bstar*STR3_DU_PER_ER/e0eta*
     (pow(1.0f+eta*cos(mdf),3.0f)-                             // l83+
      pow(1.0f+eta*cos(m0),3.0f));                             // l88+
                                                               // omit line118
    mptemp = mptemp+deltaw+deltam;                             //      line119
    wtemp = wtemp-deltaw-deltam;                               //      line120
                                                               // omit line121
                                                               // omit line122
    uatemp =                                                   //      line123
     uatemp-d2*tsincer2-d3*tsincer2*tsince-d4*tsince*tsincer2*tsince;
    uetemp =                                                   //      line124
     uetemp+bstar*c5*(sin(mptemp)-sin(m0));
    ultemp =                                                   //      line125
     ultemp+                                                   // thru line126
     (d2+2.0f*c1r2)*tsincer2*tsince+                           // l97+
     0.25f*(3.0f*d3+12.0f*c1*d2+10.0f*c1r2*c1)*tsince*tsincer2*tsince+
     0.2f*(                                                    // l98+
      3.0f*d4+12.0f*c1*d3+6.0f*d2*d2+30.0f*c1r2*d2+15.0f*c1r2*c1r2
     )*tsince*tsincer2*tsince*tsince;                          // l99+ l100+
  }                                                // line102-line104 comments
                                                               // omit line097
                                                               // omit line098
                                                               // omit line099
                                                               // omit line100
                                                               // omit line101
  const float mp = mptemp;                                     // eq27,line109
  const float w = wtemp;                                       // eq28,line108
  const float o = odf+uo*tsincer2;                             // eq29,line111
  const float a = a0pp*pow(uatemp,2.0f);                       // eq31,line127
  const float e = e0-uetemp;                                   // eq30,line128
  const float l = mp+w+o+n0pp*ultemp;                          // eq32,line129
  const float beta = sqrt(1.0f-e*e);                           // eq33,line130
  const float n = STR3_KE/pow(a,1.5f);                         // eq34,line131
  // Long period periodics                         // line132-line134 comments
  const float axn = e*cos(w);                                  // eq35,line135
  const float ull =                                   // XLCOF //      line086
   0.125f*a30dk2*sini0*(3.0f+5.0f*cosi0)/(1.0f+cosi0);         // omit line136
  const float ll = axn*ull/(a*beta*beta);                      // eq36,line137
  const float uaynl = 0.25f*a30dk2*sini0;             // AYCOF //      line087
  const float aynl = uaynl/(a*beta*beta);                      // eq37,line138
  const float lt = l+ll;                                       // eq38,line139
  const float ayn = e*sin(w)+aynl;                             // eq39,line140
  // Solve Kepler's equation                       // line141-line143 comments
  // FMOD /////////////////////////////////////////////////////// // FUNC FMOD
  float utemp = fmod(lt-o,STR3_TWO_PI);                        // eq40,line144
  if(utemp<0.0) {
    utemp+=STR3_TWO_PI;
  }
  const float u = utemp;
  // FMOD /////////////////////////////////////////////////////// // END  FMOD
  float eawprev = u;                                           // eq43,line145
  for(size_t i=0; i<10; i++) {                                 //      line146
    float eawcurr =                                            // eq41,line153
     eawprev+                                                  // eq42
     (u-ayn*cos(eawprev)+axn*sin(eawprev)-eawprev)/            // omit line149
     (1.0f-ayn*sin(eawprev)-axn*cos(eawprev));                 // omit line150
    if(fabs(eawcurr-eawprev) <= 1.0e-6f) {                     //      line154
      break;                                                   // omit line151
    }                                                          // omit line152
    eawprev = eawcurr;                                         //      line155
  }                                                // line156-line158 comments
  const float eaw = eawprev;
  // Short period periodics
  const float sineaw = sin(eaw);                               //      line147
  const float coseaw = cos(eaw);                               //      line148
  const float ecose = axn*coseaw+ayn*sineaw;                   // eq44,line159
  const float esine = axn*sineaw-ayn*coseaw;                   // eq45,line160
  const float elr2 = axn*axn+ayn*ayn;                          // eq46,line161
                                                               // omit line162
  const float pl = a*(1.0f-elr2);                              // eq47,line163
  const float r = a*(1.0f-ecose);                              // eq48,line164
                                                               // omit line165
  const float rdt = STR3_KE*sqrt(a)*esine/r;                   // eq49,line166
  const float rfdt = STR3_KE*sqrt(pl)/r;                       // eq50,line167
                                                               // omit line168
                                                               // omit line169
                                                               // omit line170
  const float cosu =                                           // eq51,line171
   a*(coseaw-axn+ayn*esine/(1.0f+sqrt(1.0f-elr2)))/r;
  const float sinu =                                           // eq52,line172
   a*(sineaw-ayn-axn*esine/(1.0f+sqrt(1.0f-elr2)))/r;
  // ACTAN /////////////////////////////////////////////////// // FUNC ACTAN
  float lowerutemp = 0.0f;
  if(cosu==0.0f) {
    if(sinu==0.0f) {
      lowerutemp = 0.0f;
    } else if(sinu>0.0f) {
      lowerutemp = STR3_HALF_PI;
    } else {
      lowerutemp = STR3_THREE_HALVES_PI;
    }
  } else if(cosu>0.0f) {
    if(sinu==0.0f) {
      lowerutemp = 0.0f;
    } else if(sinu>0.0f) {
      lowerutemp = atan(sinu/cosu);
    } else {
      lowerutemp = STR3_TWO_PI+atan(sinu/cosu);
    }
  } else {
    lowerutemp = STR3_PI+atan(sinu/cosu);
  }
  // ACTAN /////////////////////////////////////////////////// // END  ACTAN
  const float loweru = lowerutemp;                             // eq53,line173
  const float sin2u = 2.0f*sinu*cosu;                          //      line174
  const float cos2u = 2.0f*cosu*cosu-1.0f;                     //      line175
                                                               // omit line176
                                                               // omit line177
                                                               // omit line178
                                                   // line179-line181 comments
  const float deltar = 0.5f*STR3_K2*nthetar2a1*cos2u/pl;       // eq54fline182
  const float deltau =                                         // eq55fline183
   -0.25f*STR3_K2*(7.0f*thetar2-1.0f)*sin2u/(pl*pl);
  const float deltao = 1.5f*STR3_K2*cosi0*sin2u/(pl*pl);       // eq56fline184
  const float deltai =                                         // eq57fline185
   1.5f*STR3_K2*cosi0*sini0*cos2u/(pl*pl);
  const float deltardt =                                       // eq58fline186
   -1.0f*STR3_K2*n*nthetar2a1*sin2u/pl;
  const float deltarfdt =                                      // eq59fline187
   STR3_K2*n*(nthetar2a1*cos2u+1.5*thetar2t3m1)/pl;
  const float rk =                                             // eq60,line182
   r*(1.0f-1.5f*STR3_K2*sqrt(1.0f-elr2)*thetar2t3m1/(pl*pl))+
   deltar;
  const float uk = u+deltau;                                   // eq61,line183
  const float ok = o+deltao;                                   // eq62,line184
  const float ik = i0+deltai;                                  // eq63,line185
  const float rkdt = rdt+deltardt;                             // eq64,line186
  const float rfkdt = rfdt+deltarfdt;                          // eq65,line187
  // Unit orientation vectors                      // line188-line190 comments
  const float sinuk = sin(uk);                                 //      line191
  const float cosuk = cos(uk);                                 //      line192
  const float sinik = sin(ik);                                 //      line193
  const float cosik = cos(ik);                                 //      line194
  const float sinok = sin(ok);                                 //      line195
  const float cosok = cos(ok);                                 //      line196
  const float mx = -1.0f*sinok*cosik;                          // eq68,line197
  const float my = cosok*cosik;                                // eq69,line198
  const float mz = sinik;                                      // eq70
  const float nx = cosok;                                      // eq71
  const float ny = sinok;                                      // eq72
  const float nz = 0.0f;                                       // eq73
  const float ux = mx*sinuk+nx*cosuk;                          // eq66,line199
  const float uy = my*sinuk+ny*cosuk;                          // |   ,line200
  const float uz = mz*sinuk+nz*cosuk;                          // -   ,line201
  const float vx = mx*cosuk-nx*sinuk;                          // eq67,line202
  const float vy = my*cosuk-ny*sinuk;                          // |   ,line203
  const float vz = mz*cosuk-nz*sinuk;                          // -   ,line204
  // Position and velocity                         // line205-line207 comments
  const float px = rk*ux;                                      // eq74,line208
  const float py = rk*uy;                                      // |   ,line209
  const float pz = rk*uz;                                      // -   ,line210
  const float sx = rkdt*ux+rfkdt*vx;                           // eq75,line211
  const float sy = rkdt*uy+rfkdt*vy;                           // |   ,line212
  const float sz = rkdt*uz+rfkdt*vz;                           // -   ,line213
  // Return ECI position
  eci_posn_t eci_posn = {.x=0.0f, .y=0.0f, .z=0.0f};
  eci_posn.x = px*STR3_KM_PER_ER/STR3_DU_PER_ER;
  eci_posn.y = py*STR3_KM_PER_ER/STR3_DU_PER_ER;
  eci_posn.z = pz*STR3_KM_PER_ER/STR3_DU_PER_ER;
  return eci_posn;
}

// Task-like functions

void rx_usart1(rx_cmd_buff_t* rx_cmd_buff_o) {
  while(                                             // while
   usart_get_flag(USART1,USART_ISR_RXNE) &&          //  USART1 RX not empty AND
   rx_cmd_buff_o->state!=RX_CMD_BUFF_STATE_COMPLETE  //  Command not complete
  ) {                                                //
    uint8_t b = usart_recv(USART1);                  // Receive byte from RX pin
    push_rx_cmd_buff(rx_cmd_buff_o, b);              // Push byte to buffer
  }                                                  //
}

void reply(rx_cmd_buff_t* rx_cmd_buff_o, tx_cmd_buff_t* tx_cmd_buff_o) {
  if(                                                  // if
   rx_cmd_buff_o->state==RX_CMD_BUFF_STATE_COMPLETE && // rx_cmd is valid AND
   tx_cmd_buff_o->empty                                // tx_cmd is empty
  ) {                                                  //
    write_reply(rx_cmd_buff_o, tx_cmd_buff_o);         // execute cmd and reply
  }                                                    //
}

void tx_usart1(tx_cmd_buff_t* tx_cmd_buff_o) {
  while(                                             // while
   usart_get_flag(USART1,USART_ISR_TXE) &&           //  USART1 TX empty AND
   !(tx_cmd_buff_o->empty)                           //  TX buffer not empty
  ) {                                                //
    uint8_t b = pop_tx_cmd_buff(tx_cmd_buff_o);      // Pop byte from TX buffer
    usart_send(USART1,b);                            // Send byte to TX pin
  }                                                  //
}
