#include "pigpiod_if2.h"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

#define numAry(ary) (sizeof( ary )/sizeof( ary [0]))
static FILE *ret_values=NULL;
static void file_close(){
  if(ret_values!=NULL)
    fclose(ret_values);
}
static const char *flie_lines(){
  static char ret_buf[256];
  char *ret;
  if (ret_values==NULL){
    if ((ret_values = fopen(VFILENAME, "r")) == NULL){
      fprintf(stderr, "Can't load : %s.\n", VFILENAME);
      exit(-1);
    }
  }
  ret=fgets(ret_buf,numAry(ret_buf),ret_values);
  if(feof(ret_values)!=0 || ret==NULL){
    fclose(ret_values);
    ret_values=NULL;
    if(ret==NULL){
      return flie_lines();
    }
  }
  return ret;
}
static void pargs(const char *format, ...) {
  va_list va;
  FILE *ret_args=NULL;
  if ((ret_args = fopen(AFILENAME, "a")) == NULL){
    fprintf(stderr, "Can't open : %s.\n", AFILENAME);
    exit(-1);
  }
  va_start(va, format);
  vfprintf(ret_args,format, va);
  va_end(va);
  fclose(ret_args);
  return;
}
//Root
double time_time(void){
  file_close();
  return 12.3;
}
//void time_sleep(double seconds){return;}
char *pigpio_error(int errnum){
  static char retc[50];
  sscanf(flie_lines(),"%s",retc);
  pargs("pigpio_error : %d\n",errnum);
  return retc;
}
unsigned pigpiod_if_version(void){
  unsigned reti;
  sscanf(flie_lines(),"%u",&reti);
  return reti;
}
//pthread_t *start_thread(gpioThreadFunc_t thread_func, void *userdata);
//void stop_thread(pthread_t *pth);
int pigpio_start(char *addrStr, char *portStr){
  int reti;
  pargs("pigpio_start : %s,%s\n",addrStr,addrStr);
  sscanf(flie_lines(),"%d",&reti);
  return reti;
}
void pigpio_stop(int pi){
  pargs("pigpio_stop : %d\n",pi);
  return ;
}
uint32_t get_current_tick(int pi){
  uint32_t reti;
  pargs("get_current_tick : %d\n",pi);
  sscanf(flie_lines(),"%lu",&reti);
  return reti;
}
uint32_t get_hardware_revision(int pi){
  uint32_t reti;
  pargs("get_hardware_revision : %d\n",pi);
  sscanf(flie_lines(),"%lu",&reti);
  return reti;
}
uint32_t get_pigpio_version(int pi){
  uint32_t reti;
  pargs("get_pigpio_version : %d\n",pi);
  sscanf(flie_lines(),"%lu",&reti);
  return reti;
}

//GPIO
int set_mode(int pi, unsigned gpio, unsigned mode){
  int reti;
  pargs("set_mode : %d,%u,%u\n",pi,gpio,mode);
  sscanf(flie_lines(),"%d",&reti);
  return reti;
}
int get_mode(int pi, unsigned gpio){
  int reti;
  pargs("getget_mode_hardware_revision : %d,%u,%u\n",pi,gpio);
  sscanf(flie_lines(),"%d",&reti);
  return reti;
}
int set_pull_up_down(int pi, unsigned gpio, unsigned pud){
  int reti;
  pargs("set_pull_up_down : %d,%u,%u\n",pi,gpio,pud);
  sscanf(flie_lines(),"%d",&reti);
  return reti;
}
int gpio_read(int pi, unsigned gpio){
  int reti;
  pargs("get_hardware_revision : %d,%u,%u\n",pi,gpio);
  sscanf(flie_lines(),"%d",&reti);
  return reti;
}
int gpio_write(int pi, unsigned gpio, unsigned level){
  int reti;
  pargs("get_hardware_revision : %d,%u,%u\n",pi,gpio,level);
  sscanf(flie_lines(),"%d",&reti);
  return reti;
}
int hardware_clock(int pi, unsigned gpio, unsigned clkfreq){
  int reti;
  pargs("get_hardware_revision : %d,%u,%u\n",pi,gpio,clkfreq);
  sscanf(flie_lines(),"%d",&reti);
  return reti;
}
int hardware_PWM(int pi, unsigned gpio, unsigned PWMfreq, uint32_t PWMduty){
  int reti;
  pargs("get_hardware_revision : %d,%u,%u,%lu\n",pi,gpio,PWMfreq,PWMduty);
  sscanf(flie_lines(),"%d",&reti);
  return reti;
}
//Bank
uint32_t read_bank_1(int pi){return 123;}
uint32_t read_bank_2(int pi){return 123;}
int clear_bank_1(int pi, uint32_t bits){return 123;}
int clear_bank_2(int pi, uint32_t bits){return 123;}
int set_bank_1(int pi, uint32_t bits){return 123;}
int set_bank_2(int pi, uint32_t bits){return 123;}
//UserGPIO
int set_watchdog(int pi, unsigned user_gpio, unsigned timeout){return 123;}
int set_glitch_filter(int pi, unsigned user_gpio, unsigned steady){return 123;}
int callback(int pi, unsigned user_gpio, unsigned edge, CBFunc_t f){return 123;}
int wait_for_edge(int pi, unsigned user_gpio, unsigned edge, double timeout){return 123;}
int set_noise_filter(int pi, unsigned user_gpio, unsigned steady, unsigned active){return 123;}
int gpio_trigger(int pi, unsigned user_gpio, unsigned pulseLen, unsigned level){return 123;}
//PWM
int set_PWM_dutycycle(int pi, unsigned user_gpio, unsigned dutycycle){return 123;}
int get_PWM_dutycycle(int pi, unsigned user_gpio){return 123;}
int set_PWM_range(int pi, unsigned user_gpio, unsigned range){return 123;}
int get_PWM_range(int pi, unsigned user_gpio){return 123;}
int get_PWM_real_range(int pi, unsigned user_gpio){return 123;}
int set_PWM_frequency(int pi, unsigned user_gpio, unsigned frequency){return 123;}
int get_PWM_frequency(int pi, unsigned user_gpio){return 123;}
int set_servo_pulsewidth(int pi, unsigned user_gpio, unsigned pulsewidth){return 123;}
int get_servo_pulsewidth(int pi, unsigned user_gpio){return 123;}
//Notify
int notify_open(int pi){return 123;}
int notify_begin(int pi, unsigned handle, uint32_t bits){return 123;}
int notify_pause(int pi, unsigned handle){return 123;}
int notify_close(int pi, unsigned handle){return 123;}

//Wave
int wave_clear(int pi){return 123;}
int wave_add_new(int pi){return 123;}
int wave_add_generic(int pi, unsigned numPulses, gpioPulse_t *pulses){return 123;}
int wave_add_serial(int pi, unsigned user_gpio, unsigned baud, unsigned data_bits, unsigned stop_bits, unsigned offset, unsigned numBytes, char *str){return 123;}
int wave_create(int pi){return 123;}
int wave_delete(int pi, unsigned wave_id){return 123;}
int wave_send_once(int pi, unsigned wave_id){return 123;}
int wave_send_repeat(int pi, unsigned wave_id){return 123;}
int wave_send_using_mode(int pi, unsigned wave_id, unsigned mode){return 123;}
int wave_chain(int pi, char *buf, unsigned bufSize){return 123;}
int wave_tx_at(int pi){return 123;}
int wave_tx_busy(int pi){return 123;}
int wave_tx_stop(int pi){return 123;}
int wave_get_micros(int pi){return 123;}
int wave_get_high_micros(int pi){return 123;}
int wave_get_max_micros(int pi){return 123;}
int wave_get_pulses(int pi){return 123;}
int wave_get_high_pulses(int pi){return 123;}
int wave_get_max_pulses(int pi){return 123;}
int wave_get_cbs(int pi){return 123;}
int wave_get_high_cbs(int pi){return 123;}
int wave_get_max_cbs(int pi){return 123;}

int store_script(int pi, char *script){return 123;}
int run_script(int pi, unsigned script_id, unsigned numPar, uint32_t *param){return 123;}
int script_status(int pi, unsigned script_id, uint32_t *param){return 123;}
int stop_script(int pi, unsigned script_id){return 123;}
int delete_script(int pi, unsigned script_id){return 123;}

int bb_serial_read_open(int pi, unsigned user_gpio, unsigned baud, unsigned data_bits){return 123;}
int bb_serial_read(int pi, unsigned user_gpio, void *buf, size_t bufSize){return 123;}
int bb_serial_read_close(int pi, unsigned user_gpio){return 123;}
int bb_serial_invert(int pi, unsigned user_gpio, unsigned invert){return 123;}

int i2c_open(int pi, unsigned i2c_bus, unsigned i2c_addr, unsigned i2c_flags){return 123;}
int i2c_close(int pi, unsigned handle){return 123;}
int i2c_write_quick(int pi, unsigned handle, unsigned bit){return 123;}
int i2c_write_byte(int pi, unsigned handle, unsigned bVal){return 123;}
int i2c_read_byte(int pi, unsigned handle){return 123;}
int i2c_read_byte_data(int pi, unsigned handle, unsigned i2c_reg){return 123;}
int i2c_read_word_data(int pi, unsigned handle, unsigned i2c_reg){return 123;}
int i2c_process_call(int pi, unsigned handle, unsigned i2c_reg, unsigned wVal){return 123;}
int i2c_read_block_data(int pi, unsigned handle, unsigned i2c_reg, char *buf){return 123;}
int i2c_read_device(int pi, unsigned handle, char *buf, unsigned count){return 123;}
int i2c_write_device(int pi, unsigned handle, char *buf, unsigned count){return 123;}

int bb_i2c_open(int pi, unsigned SDA, unsigned SCL, unsigned baud){return 123;}
int bb_i2c_close(int pi, unsigned SDA){return 123;}
int bb_spi_close(int pi, unsigned CS){return 123;}

int spi_open(int pi, unsigned spi_channel, unsigned baud, unsigned spi_flags){return 123;}
int spi_close(int pi, unsigned handle){return 123;}
int spi_read(int pi, unsigned handle, char *buf, unsigned count){return 123;}
int spi_write(int pi, unsigned handle, char *buf, unsigned count){return 123;}

int serial_open(int pi, char *ser_tty, unsigned baud, unsigned ser_flags){return 123;}
int serial_close(int pi, unsigned handle){return 123;}
int serial_write_byte(int pi, unsigned handle, unsigned bVal){return 123;}
int serial_read_byte(int pi, unsigned handle){return 123;}
int serial_write(int pi, unsigned handle, char *buf, unsigned count){return 123;}
int serial_read(int pi, unsigned handle, char *buf, unsigned count){return 123;}
int serial_data_available(int pi, unsigned handle){return 123;}

int custom_1(int pi, unsigned arg1, unsigned arg2, char *argx, unsigned argc){return 123;}
int get_pad_strength(int pi, unsigned pad){return 123;}
int set_pad_strength(int pi, unsigned pad, unsigned padStrength){return 123;}
//int shell_(int pi, char *scriptName, char *scriptString){return 123;}
//int file_open(int pi, char *file, unsigned mode){return 123;}
//int file_close(int pi, unsigned handle){return 123;}
//int file_write(int pi, unsigned handle, char *buf, unsigned count){return 123;}
//int file_read(int pi, unsigned handle, char *buf, unsigned count){return 123;}
//int file_seek(int pi, unsigned handle, int32_t seekOffset, int seekFrom){return 123;}
//int file_list(int pi, char *fpat,  char *buf, unsigned count){return 123;}
int bsc_xfer(int pi, bsc_xfer_t *bscxfer){return 123;}
int bsc_i2c(int pi, int i2c_addr, bsc_xfer_t *bscxfer){return 123;}
int event_callback(int pi, unsigned event, evtCBFunc_t f){return 123;}
int event_callback_ex(int pi, unsigned event, evtCBFuncEx_t f, void *userdata){return 123;}
int event_callback_cancel(unsigned callback_id){return 123;}
int wait_for_event(int pi, unsigned event, double timeout){return 123;}
int event_trigger(int pi, unsigned event){return 123;}

int i2c_write_byte_data(int pi, unsigned handle, unsigned i2c_reg, unsigned bVal){return 123;}
int i2c_write_word_data(int pi, unsigned handle, unsigned i2c_reg, unsigned wVal){return 123;}
int i2c_write_block_data(int pi, unsigned handle, unsigned i2c_reg, char *buf, unsigned count){return 123;}
int i2c_block_process_call(int pi, unsigned handle, unsigned i2c_reg, char *buf, unsigned count){return 123;}
int i2c_read_i2c_block_data(int pi, unsigned handle, unsigned i2c_reg, char *buf, unsigned count){return 123;}
int i2c_write_i2c_block_data(int pi, unsigned handle, unsigned i2c_reg, char *buf, unsigned count){return 123;}
int i2c_zip(int pi, unsigned handle, char *inBuf, unsigned inLen, char *outBuf, unsigned outLen){return 123;}
int bb_i2c_zip(int pi, unsigned SDA, char *inBuf, unsigned inLen, char *outBuf, unsigned outLen){return 123;}
int bb_spi_open(int pi, unsigned CS, unsigned MISO, unsigned MOSI, unsigned SCLK, unsigned baud, unsigned spi_flags){return 123;}
int bb_spi_xfer(int pi, unsigned CS, char    *txBuf, char    *rxBuf, unsigned count){return 123;}
int spi_xfer(int pi, unsigned handle, char *txBuf, char *rxBuf, unsigned count){return 123;}


static void* volatile static_userdata;
void* threaded_func(void*data){
  int i;
  CBFuncEx_t f=(CBFuncEx_t)data;
  sleep(1);
  if(static_userdata!=NULL){(*f)(23,77,99,88,static_userdata);}
  sleep(1);
  if(static_userdata!=NULL){(*f)(23,76,98,87,static_userdata);}
  sleep(2);
  i=0;
  while(static_userdata!=NULL){
    (*f)(23,120,i,-i,static_userdata);
    usleep(1);
    i++;
  }
  printf("callback is stop\n");
}
int callback_cancel(unsigned callback_id){
  printf("callback is start\n");
  static_userdata=NULL;
  return 123;
}

int callback_ex(int pi, unsigned user_gpio, unsigned edge, CBFuncEx_t f, void *userdata){
  pthread_t thread;
  static_userdata=userdata;
  pthread_create( &thread, NULL, threaded_func, f );
  return 123;
}
