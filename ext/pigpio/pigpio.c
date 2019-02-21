#include "ruby.h"
#include "pigpiod_if2.h"

#define TypedData_Get_Struct2(obj, type, data_type) ((type*)rb_check_typeddata((obj), (data_type)))

static VALUE cCallbackID;
static VALUE cNativeQueue;
static VALUE cCallbackError;

typedef struct{
   uint32_t tick;
  unsigned char level;
  unsigned char dummy;
  unsigned char gpio;
  unsigned char pi;
} callback_item_t;

#define PG_EXT_CALLBACK_BUF_SIZE (128)
typedef struct{
  volatile callback_item_t buf[ PG_EXT_CALLBACK_BUF_SIZE ];
  volatile unsigned short read;
  volatile unsigned short write;
  volatile unsigned short size;
  volatile char flag_overflow;
} callback_queue_t;



typedef struct{
  callback_queue_t*queue;
  VALUE callback;
  struct timeval t;
} callback_pqueue_t;
void pigpio_rbst_callback_pqueue_dmark(void* _self){
  callback_pqueue_t *self=(callback_pqueue_t *)_self;
  rb_gc_mark(self->callback);
}
void pigpio_rbst_callback_pqueue_dfree(void* _self){
  callback_pqueue_t *self=(callback_pqueue_t *)_self;
  if(self->queue!=NULL)free(self->queue);
  xfree(self);
  return;
}
size_t pigpio_rbst_callback_pqueue_dsize(const void *_self){
  return sizeof(callback_pqueue_t)+sizeof(callback_queue_t);
}
const rb_data_type_t callback_pqueue_type = { //https://gist.github.com/yugui/87ef6964d8a76794be6f
    "struct@callback_pqueue",{
      pigpio_rbst_callback_pqueue_dmark,
      pigpio_rbst_callback_pqueue_dfree,
      pigpio_rbst_callback_pqueue_dsize,{0,0}},0,NULL,0
};
/*
Constructor of Pigpio::NativeQueue class
*/
VALUE pigpio_rbst_callback_pqueue_make_inner(VALUE callback,time_t sec,long usec){
  VALUE obj;
  callback_pqueue_t *st;
  callback_queue_t *queue;
  obj = TypedData_Make_Struct(cNativeQueue, callback_pqueue_t, &callback_pqueue_type, st);
  st->t.tv_sec=sec;
  st->t.tv_usec=usec;
  st->callback=callback;
  st->queue=queue=malloc(sizeof(callback_queue_t));
  queue->read=0;
  queue->write=0;
  queue->flag_overflow=0;
 return obj;
}
/*
Set a data to FIFO Queue.
*/
void pigpio_rbbk_CBFuncEx(int pi, unsigned user_gpio, unsigned level, uint32_t tick, void *_queue){
  callback_queue_t *queue=(callback_queue_t *)_queue;
  unsigned short cur=queue->write;
  queue->buf[cur].tick=tick;
  queue->buf[cur].level=level;
  queue->buf[cur].gpio=user_gpio;
  queue->buf[cur].pi=pi;
  cur=(cur>=(PG_EXT_CALLBACK_BUF_SIZE-1)) ? 0 : (cur+1) ;
/*
  cur->tick=tick;
  cur->level=level;
  cur->gpio=user_gpio;
  cur->pi=pi;
  cur=(cur==queue->buf+(PG_EXT_CALLBACK_BUF_SIZE-1)) ? queue->buf : (cur+1) ;
*/
  if(cur==queue->read){
    queue->flag_overflow=1;
  }else{
    queue->write=cur;
  }
  return;
}
static VALUE callback_receive(void*_self){
  callback_pqueue_t *st=TypedData_Get_Struct2((VALUE)_self,callback_pqueue_t,&callback_pqueue_type);
  VALUE callee_proc      =st->callback;
  struct timeval  t      =st->t;
  callback_queue_t *queue=st->queue;
  unsigned short cur=queue->read;

  while(1){
    if(queue->flag_overflow!=0){
      rb_raise(cCallbackError,"Overflow NativeQueue.\n");
    }
    while(cur==queue->write){
      rb_thread_wait_for(t);
    }
    rb_funcall((VALUE)callee_proc, rb_intern("call"), 4,
      ULONG2NUM(queue->buf[cur].tick),CHR2FIX(queue->buf[cur].level),CHR2FIX(queue->buf[cur].gpio),CHR2FIX(queue->buf[cur].pi));
    queue->read=cur=(cur==(PG_EXT_CALLBACK_BUF_SIZE-1)) ? 0 : (cur+1) ;
  }
  return Qnil;
}



typedef int (*cancel_t)(unsigned);
typedef struct{
  int id;
  VALUE queue;
  VALUE thread;
  cancel_t cancel;
} callback_id_t;

void pigpio_rbst_callback_id_dmark(void* _self){
  callback_id_t *self=(callback_id_t *)_self;
  rb_gc_mark(self->queue);
  rb_gc_mark(self->thread);
}
void pigpio_rbst_callback_id_dfree(void* _self){
  callback_id_t *self=(callback_id_t *)_self;
  xfree(self);
  return;
}
size_t pigpio_rbst_callback_id_dsize(const void *_self){
  return sizeof(callback_id_t);
}
const rb_data_type_t callback_id_data_type = { //https://gist.github.com/yugui/87ef6964d8a76794be6f
    "struct@callback_id",{
      pigpio_rbst_callback_id_dmark,
      pigpio_rbst_callback_id_dfree,
      pigpio_rbst_callback_id_dsize,
      {0,0}
    },0,NULL,0
};
VALUE pigpio_rbst_callback_id_make_inner(int id,cancel_t cancel,VALUE queue,VALUE thread){
  VALUE obj;
  callback_id_t *st;
  obj = TypedData_Make_Struct(cCallbackID, callback_id_t, &callback_id_data_type, st);
  st->id=id;
  st->queue=queue;
  st->thread=thread;
  st->cancel=cancel;
  return obj;
}
/*
Get callback id.R eturn Integer.
*/
VALUE pigpio_rbst_callback_id_r_id(VALUE self){
  callback_id_t *st=TypedData_Get_Struct2(self,callback_id_t,&callback_id_data_type);
  return(INT2NUM(st->id));
}
/*
This function cancels a callback/event-callback.

The function returns 0 if OK, otherwise pigif_callback_not_found.

:call-seq:
 cancel() -> Integer

See also: {pigpio site event_callback_cancel}[http://abyz.me.uk/rpi/pigpio/pdif2.html#event_callback_cancel]
See also: {pigpio site callback_cancel}[http://abyz.me.uk/rpi/pigpio/pdif2.html#callback_cancel]
*/
VALUE pigpio_rbst_callback_id_cancel(VALUE self){
  int id;
  callback_id_t *st=TypedData_Get_Struct2(self,callback_id_t,&callback_id_data_type);
  id=st->id;
  if(id<0){return INT2NUM(pigif_callback_not_found);}
  id=(*(st->cancel))(st->id);
  if(st->thread!=Qnil){
    rb_funcall((VALUE)st->thread, rb_intern("kill"), 0);
  }
  st->id=-1;
  st->thread=Qnil;
  st->queue=Qnil;
  return INT2NUM(id);
}

void pigpio_rbbk_evtCBFuncEx(int pi, unsigned event, uint32_t tick, void *callee_proc){
  (rb_funcall((VALUE)callee_proc, rb_intern("call"), 2,ULONG2NUM(tick),UINT2NUM(event)));
  return;
}

const rb_data_type_t bsc_xfer_data_type = { //https://gist.github.com/yugui/87ef6964d8a76794be6f
    "struct@bsc_xfer",{NULL,(void*)-1,0,{0,0}},0,NULL,0
};
/*
Constructor of bsc_xfer_t as Pigpio::BscXfer class

  typedef struct
  {
    uint32_t control;          // Write
    int rxCnt;                 // Read only
    char rxBuf[BSC_FIFO_SIZE]; // Read only
    int txCnt;                 // Write
    char txBuf[BSC_FIFO_SIZE]; // Write
  } bsc_xfer_t;
*/
VALUE pigpio_rbst_bsc_xfer_make(VALUE self){
  VALUE obj;
  bsc_xfer_t *st;
  obj = TypedData_Make_Struct(self, bsc_xfer_t, &bsc_xfer_data_type, st);
  return obj;
}
/*
Setter
*/
VALUE pigpio_rbst_bsc_xfer_w_control(VALUE self,VALUE control){
  bsc_xfer_t *st=TypedData_Get_Struct2(self,bsc_xfer_t,&bsc_xfer_data_type);
  st->control=NUM2ULONG(control);
  return self;
}
/*
Setter
*/
VALUE pigpio_rbst_bsc_xfer_w_txBuf(VALUE self,VALUE txBuf){
  bsc_xfer_t *st=TypedData_Get_Struct2(self,bsc_xfer_t,&bsc_xfer_data_type);
  int len=RSTRING_LEN(txBuf);
  char *buf=StringValuePtr(txBuf);
  st->txCnt=(len<BSC_FIFO_SIZE)?len:BSC_FIFO_SIZE;
  for(int i=0;i<st->txCnt;i++){
    st->txBuf[i]=*buf++;
  }
  RB_GC_GUARD(txBuf);
  return self;
}
/*
Getter
*/
VALUE pigpio_rbst_bsc_xfer_r_rxBuf(VALUE self){
  bsc_xfer_t *st=TypedData_Get_Struct2(self,bsc_xfer_t,&bsc_xfer_data_type);
  VALUE rxBuf=rb_str_new("",st->rxCnt);
  char *buf=StringValuePtr(rxBuf);
  for(int i=0;i<st->rxCnt;i++){
    *buf++=st->rxBuf[i];
  }
  return rxBuf;
}

const rb_data_type_t gpioPulse_data_type = { //https://gist.github.com/yugui/87ef6964d8a76794be6f
    "struct@gpioPulse",{NULL,(void*)-1,0,{0,0}},0,NULL,0
};
/*
Constructor of gpioPulse_t as Pigpio::Pulse class
. .
 typedef struct
 {
    uint32_t gpioOn;
    uint32_t gpioOff;
    uint32_t usDelay;
 } gpioPulse_t;
. .
*/
VALUE pigpio_rbst_gpioPulse_make(VALUE self,VALUE gpioOn,VALUE gpioOff,VALUE usDelay){
  VALUE obj;
  gpioPulse_t *st;
  obj = TypedData_Make_Struct(self, gpioPulse_t, &gpioPulse_data_type, st);
  st->gpioOn =NUM2ULONG(gpioOn );
  st->gpioOff=NUM2ULONG(gpioOff);
  st->usDelay=NUM2ULONG(usDelay);
  return obj;
}

/*
Connect to the pigpio daemon.  Reserving command and
notification streams.

. .
 addrStr: specifies the host or IP address of the Pi running the
          pigpio daemon.  It may be NULL in which case localhost
          is used unless overridden by the PIGPIO_ADDR environment
          variable.

 portStr: specifies the port address used by the Pi running the
          pigpio daemon.  It may be NULL in which case "8888"
          is used unless overridden by the PIGPIO_PORT environment
          variable.
. .

Returns an integer value greater than or equal to zero if OK.

This value is passed to the GPIO routines to specify the Pi
to be operated on.

:call-seq:
 pigpio_start(String address,String port) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#pigpio_start]
*/
VALUE pigpio_rbfn_pigpio_start(int argc, VALUE *argv, VALUE self){
  int ret;
  VALUE addrStr; VALUE portStr;
  rb_scan_args(argc,argv,"02",&addrStr,&portStr);
  ret=pigpio_start(
    NIL_P(addrStr)? NULL : StringValueCStr(addrStr),
    NIL_P(portStr)? NULL : StringValueCStr(portStr));
  RB_GC_GUARD(addrStr);
  RB_GC_GUARD(portStr);
  return INT2NUM(ret);
}
/*
Terminates the connection to a pigpio daemon and releases
resources used by the library.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 pigpio_stop() -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#pigpio_stop]
*/
VALUE pigpio_rbfn_pigpio_stop(VALUE self,VALUE pi){
  pigpio_stop(NUM2INT(pi));
  return Qnil;
}

/*
Set the GPIO mode.

. .
   pi: >=0 (as returned by [*pigpio_start*]).
 gpio: 0-53.
 mode: PI_INPUT, PI_OUTPUT, PI_ALT0, PI_ALT1,
       PI_ALT2, PI_ALT3, PI_ALT4, PI_ALT5.
. .

Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_MODE,
or PI_NOT_PERMITTED.

:call-seq:
 set_mode(Integer pi,Integer gpio,Integer mode) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_mode]
*/
VALUE pigpio_rbfn_set_mode(VALUE self,VALUE pi, VALUE gpio, VALUE mode){
  return INT2NUM(set_mode(NUM2INT(pi), NUM2UINT(gpio), NUM2UINT(mode)));
}
/*
Get the GPIO mode.

. .
   pi: >=0 (as returned by [*pigpio_start*]).
 gpio: 0-53.
. .

Returns the GPIO mode if OK, otherwise PI_BAD_GPIO.

:call-seq:
 get_mode(Integer pi,Integer gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_mode]
*/
VALUE pigpio_rbfn_get_mode(VALUE self,VALUE pi, VALUE gpio){
  return INT2NUM(get_mode( NUM2INT(pi), NUM2UINT(gpio)));
}
/*
Set or clear the GPIO pull-up/down resistor. 

. .
   pi: >=0 (as returned by [*pigpio_start*]).
 gpio: 0-53.
  pud: PI_PUD_UP, PI_PUD_DOWN, PI_PUD_OFF.
. .

Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_PUD,
or PI_NOT_PERMITTED.

:call-seq:
 set_pull_up_down(Integer pi,Integer gpio,Integer pud) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_pull_up_down]
*/
VALUE pigpio_rbfn_set_pull_up_down(VALUE self,VALUE pi, VALUE gpio, VALUE pud){
  return INT2NUM(set_pull_up_down(NUM2INT(pi), NUM2UINT(gpio), NUM2UINT(pud)));
}
/*
Read the GPIO level. 

. .
   pi: >=0 (as returned by [*pigpio_start*]).
 gpio:0-53.
. .

Returns the GPIO level if OK, otherwise PI_BAD_GPIO.

:call-seq:
 gpio_read(Integer pi,Integer gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#gpio_read]
*/
VALUE pigpio_rbfn_gpio_read(VALUE self,VALUE pi, VALUE gpio){
  return INT2NUM(gpio_read(NUM2INT(pi), NUM2UINT(gpio)));
}
/*
Write the GPIO level.

. .
    pi: >=0 (as returned by [*pigpio_start*]).
  gpio: 0-53.
 level: 0, 1.
. .

Returns 0 if OK, otherwise PI_BAD_GPIO, PI_BAD_LEVEL,
or PI_NOT_PERMITTED.

Notes

If PWM or servo pulses are active on the GPIO they are switched off.

:call-seq:
 gpio_write(Integer pi,Integer gpio,Integer level) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#gpio_write]
*/
VALUE pigpio_rbfn_gpio_write(VALUE self,VALUE pi, VALUE gpio, VALUE level){
  return INT2NUM(gpio_write(NUM2INT(pi), NUM2UINT(gpio), NUM2UINT(level)));
}
/*
Start (non-zero dutycycle) or stop (0) PWM pulses on the GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
 dutycycle: 0-range (range defaults to 255).
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_DUTYCYCLE,
or PI_NOT_PERMITTED.
Notes

The [*set_PWM_range*] function may be used to change the
default range of 255.

:call-seq:
 set_PWM_dutycycle(Integer pi,Integer user_gpio, VALUE dutycycle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_PWM_dutycycle]
*/
VALUE pigpio_rbfn_set_PWM_dutycycle(VALUE self, VALUE pi, VALUE user_gpio, VALUE dutycycle){
  return INT2NUM( set_PWM_dutycycle(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(dutycycle)));
}
/*
Return the PWM dutycycle in use on a GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_NOT_PWM_GPIO.

For normal PWM the dutycycle will be out of the defined range
for the GPIO (see [*get_PWM_range*]).

If a hardware clock is active on the GPIO the reported dutycycle
will be 500000 (500k) out of 1000000 (1M).

If hardware PWM is active on the GPIO the reported dutycycle
will be out of a 1000000 (1M).

:call-seq:
 get_PWM_dutycycle(Integer pi,Integer user_gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_PWM_dutycycle]
*/
VALUE pigpio_rbfn_get_PWM_dutycycle(VALUE self, VALUE pi, VALUE user_gpio){
  return INT2NUM( get_PWM_dutycycle(NUM2INT(pi), NUM2UINT(user_gpio)));
}
/*
Set the range of PWM values to be used on the GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
     range: 25-40000.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_DUTYRANGE,
or PI_NOT_PERMITTED.

Notes

If PWM is currently active on the GPIO its dutycycle will be
scaled to reflect the new range.

The real range, the number of steps between fully off and fully on
for each of the 18 available GPIO frequencies is

. .
   25(#1),    50(#2),   100(#3),   125(#4),    200(#5),    250(#6),
  400(#7),   500(#8),   625(#9),   800(#10),  1000(#11),  1250(#12),
 2000(#13), 2500(#14), 4000(#15), 5000(#16), 10000(#17), 20000(#18)
. .

The real value set by set_PWM_range is (dutycycle * real range) / range.


:call-seq:
 set_PWM_range(Integer pi,Integer user_gpio, Integer range) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_PWM_range]
*/
VALUE pigpio_rbfn_set_PWM_range(VALUE self, VALUE pi, VALUE user_gpio, VALUE range){
  return INT2NUM( set_PWM_range(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(range)));
}
/*
Get the range of PWM values being used on the GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
. .

Returns the dutycycle range used for the GPIO if OK,
otherwise PI_BAD_USER_GPIO.

If a hardware clock or hardware PWM is active on the GPIO the
reported range will be 1000000 (1M).

:call-seq:
 get_PWM_range(Integer pi,Integer user_gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_PWM_range]
*/
VALUE pigpio_rbfn_get_PWM_range(VALUE self, VALUE pi, VALUE user_gpio){
  return INT2NUM( get_PWM_range(NUM2INT(pi), NUM2UINT(user_gpio)));
}
/*
Get the real underlying range of PWM values being used on the GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
. .

Returns the real range used for the GPIO if OK,
otherwise PI_BAD_USER_GPIO.

If a hardware clock is active on the GPIO the reported
real range will be 1000000 (1M).

If hardware PWM is active on the GPIO the reported real range
will be approximately 250M divided by the set PWM frequency.

:call-seq:
 get_PWM_real_range(Integer pi,Integer user_gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_PWM_real_range]
*/
VALUE pigpio_rbfn_get_PWM_real_range(VALUE self, VALUE pi, VALUE user_gpio){
  return INT2NUM( get_PWM_real_range(NUM2INT(pi), NUM2UINT(user_gpio)));
}
/*
Set the frequency (in Hz) of the PWM to be used on the GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
 frequency: >=0 (Hz).
. .

Returns the numerically closest frequency if OK, otherwise
PI_BAD_USER_GPIO or PI_NOT_PERMITTED.

If PWM is currently active on the GPIO it will be switched
off and then back on at the new frequency.

Each GPIO can be independently set to one of 18 different
PWM frequencies.

The selectable frequencies depend upon the sample rate which
may be 1, 2, 4, 5, 8, or 10 microseconds (default 5).  The
sample rate is set when the pigpio daemon is started.

The frequencies for each sample rate are:

. .
                        Hertz

        1: 40000 20000 10000 8000 5000 4000 2500 2000 1600
            1250  1000   800  500  400  250  200  100   50

        2: 20000 10000  5000 4000 2500 2000 1250 1000  800
             625   500   400  250  200  125  100   50   25

        4: 10000  5000  2500 2000 1250 1000  625  500  400
             313   250   200  125  100   63   50   25   13
 sample
  rate
  (us)  5:  8000  4000  2000 1600 1000  800  500  400  320
             250   200   160  100   80   50   40   20   10
 
        8:  5000  2500  1250 1000  625  500  313  250  200
             156   125   100   63   50   31   25   13    6

       10:  4000  2000  1000  800  500  400  250  200  160
             125   100    80   50   40   25   20   10    5
. .

:call-seq:
 set_PWM_frequency(Integer pi,Integer user_gpio, Integer frequency) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_PWM_frequency]
*/
VALUE pigpio_rbfn_set_PWM_frequency(VALUE self, VALUE pi, VALUE user_gpio, VALUE frequency){
  return INT2NUM( set_PWM_frequency(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(frequency)));
}
/*
Get the frequency of PWM being used on the GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
. .

For normal PWM the frequency will be that defined for the GPIO by
[*set_PWM_frequency*].

If a hardware clock is active on the GPIO the reported frequency
will be that set by [*hardware_clock*].

If hardware PWM is active on the GPIO the reported frequency
will be that set by [*hardware_PWM*].

Returns the frequency (in hertz) used for the GPIO if OK,
otherwise PI_BAD_USER_GPIO.

:call-seq:
 get_PWM_frequency(Integer pi,Integer user_gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_PWM_frequency]
*/
VALUE pigpio_rbfn_get_PWM_frequency(VALUE self, VALUE pi, VALUE user_gpio){
  return INT2NUM( get_PWM_frequency(NUM2INT(pi), NUM2UINT(user_gpio)));
}
/*
Start (500-2500) or stop (0) servo pulses on the GPIO.

. .
         pi: >=0 (as returned by [*pigpio_start*]).
  user_gpio: 0-31.
 pulsewidth: 0 (off), 500 (anti-clockwise) - 2500 (clockwise).
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_PULSEWIDTH or
PI_NOT_PERMITTED.

The selected pulsewidth will continue to be transmitted until
changed by a subsequent call to set_servo_pulsewidth.

The pulsewidths supported by servos varies and should probably be
determined by experiment. A value of 1500 should always be safe and
represents the mid-point of rotation.

You can DAMAGE a servo if you command it to move beyond its limits.

OTHER UPDATE RATES:

This function updates servos at 50Hz.  If you wish to use a different
update frequency you will have to use the PWM functions.

. .
 Update Rate (Hz)     50   100  200  400  500
 1E6/Hz            20000 10000 5000 2500 2000
. .

Firstly set the desired PWM frequency using [*set_PWM_frequency*].

Then set the PWM range using [*set_PWM_range*] to 1E6/Hz.
Doing this allows you to use units of microseconds when setting
the servo pulsewidth.

E.g. If you want to update a servo connected to GPIO 25 at 400Hz

. .
 set_PWM_frequency(25, 400);
 set_PWM_range(25, 2500);
. .

Thereafter use the [*set_PWM_dutycycle*] function to move the servo,
e.g. set_PWM_dutycycle(25, 1500) will set a 1500 us pulse. 

:call-seq:
 set_servo_pulsewidth(Integer pi,Integer user_gpio, Integer pulsewidth) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_servo_pulsewidth]
*/
VALUE pigpio_rbfn_set_servo_pulsewidth(VALUE self, VALUE pi, VALUE user_gpio, VALUE pulsewidth){
  return INT2NUM( set_servo_pulsewidth(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(pulsewidth)));
}
/*
Return the servo pulsewidth in use on a GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO or PI_NOT_SERVO_GPIO.

:call-seq:
 get_servo_pulsewidth(Integer pi,Integer user_gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_servo_pulsewidth]
*/
VALUE pigpio_rbfn_get_servo_pulsewidth(VALUE self, VALUE pi, VALUE user_gpio){
  return INT2NUM( get_servo_pulsewidth(NUM2INT(pi), NUM2UINT(user_gpio)));
}
/*
Get a free notification handle.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

Returns a handle greater than or equal to zero if OK,
otherwise PI_NO_HANDLE.

A notification is a method for being notified of GPIO state
changes via a pipe.

Pipes are only accessible from the local machine so this function
serves no purpose if you are using the library from a remote machine.
The in-built (socket) notifications provided by [*callback*]
should be used instead.

Notifications for handle x will be available at the pipe
named /dev/pigpiox (where x is the handle number).
E.g. if the function returns 15 then the notifications must be
read from /dev/pigpio15.

:call-seq:
 notify_open(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#notify_open]
*/
VALUE pigpio_rbfn_notify_open(VALUE self, VALUE pi){
  return INT2NUM( notify_open(NUM2INT(pi)));
}
/*
Start notifications on a previously opened handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: 0-31 (as returned by [*notify_open*])
   bits: a mask indicating the GPIO to be notified.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.

The notification sends state changes for each GPIO whose
corresponding bit in bits is set.

Each notification occupies 12 bytes in the fifo as follows:

. .
typedef struct
 {
   uint16_t seqno;
   uint16_t flags;
   uint32_t tick;
   uint32_t level;
 } gpioReport_t;
. .

seqno: starts at 0 each time the handle is opened and then increments
by one for each report.

flags: three flags are defined, PI_NTFY_FLAGS_WDOG,
PI_NTFY_FLAGS_ALIVE, and PI_NTFY_FLAGS_EVENT.

If bit 5 is set (PI_NTFY_FLAGS_WDOG) then bits 0-4 of the flags
indicate a GPIO which has had a watchdog timeout.

If bit 6 is set (PI_NTFY_FLAGS_ALIVE) this indicates a keep alive
signal on the pipe/socket and is sent once a minute in the absence
of other notification activity.

If bit 7 is set (PI_NTFY_FLAGS_EVENT) then bits 0-4 of the flags
indicate an event which has been triggered.

tick: the number of microseconds since system boot.  It wraps around
after 1h12m.

level: indicates the level of each GPIO.  If bit 1<<x is set then
GPIO x is high.


:call-seq:
 notify_begin(Integer pi,Integer handle, Integer bits) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#notify_begin]
*/
VALUE pigpio_rbfn_notify_begin(VALUE self, VALUE pi, VALUE handle, VALUE bits){
  return INT2NUM( notify_begin(NUM2INT(pi), NUM2UINT(handle), NUM2ULONG(bits)));
}
/*
Pause notifications on a previously opened handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: 0-31 (as returned by [*notify_open*])
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.

Notifications for the handle are suspended until
[*notify_begin*] is called again.

:call-seq:
 notify_pause(Integer pi,Integer handle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#notify_pause]
*/
VALUE pigpio_rbfn_notify_pause(VALUE self, VALUE pi, VALUE handle){
  return INT2NUM( notify_pause(NUM2INT(pi), NUM2UINT(handle)));
}
/*
Stop notifications on a previously opened handle and
release the handle for reuse.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: 0-31 (as returned by [*notify_open*])
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.

:call-seq:
 notify_close(Integer pi,Integer handle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#notify_close]
*/
VALUE pigpio_rbfn_notify_close(VALUE self, VALUE pi, VALUE handle){
  return INT2NUM( notify_close(NUM2INT(pi), NUM2UINT(handle)));
}
/*
Sets a watchdog for a GPIO.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
   timeout: 0-60000.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO
or PI_BAD_WDOG_TIMEOUT.

The watchdog is nominally in milliseconds.

Only one watchdog may be registered per GPIO.

The watchdog may be cancelled by setting timeout to 0.

Once a watchdog has been started callbacks for the GPIO will be
triggered every timeout interval after the last GPIO activity.

The callback will receive the special level PI_TIMEOUT.

:call-seq:
 set_watchdog(Integer pi,Integer user_gpio,Integer timeout) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_watchdog]
*/
VALUE pigpio_rbfn_set_watchdog(VALUE self,VALUE pi, VALUE user_gpio, VALUE timeout){
  return INT2NUM(set_watchdog(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(timeout)));
}
/*
Sets a glitch filter on a GPIO.

Level changes on the GPIO are not reported unless the level
has been stable for at least [*steady*] microseconds.  The
level is then reported.  Level changes of less than
[*steady*] microseconds are ignored.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31
    steady: 0-300000
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_BAD_FILTER.

This filter affects the GPIO samples returned to callbacks set up
with [*callback*], [*callback_ex*] and [*wait_for_edge*].

It does not affect levels read by [*gpio_read*],
[*read_bank_1*], or [*read_bank_2*].

Each (stable) edge will be timestamped [*steady*] microseconds
after it was first detected.


:call-seq:
 set_glitch_filter(Integer pi,Integer user_gpio,Integer steady) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_glitch_filter]
*/
VALUE pigpio_rbfn_set_glitch_filter(VALUE self,VALUE pi, VALUE user_gpio, VALUE steady){
  return INT2NUM(set_glitch_filter(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(steady)));
}
/*
Sets a noise filter on a GPIO.

Level changes on the GPIO are ignored until a level which has
been stable for [*steady*] microseconds is detected.  Level changes
on the GPIO are then reported for [*active*] microseconds after
which the process repeats.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31
    steady: 0-300000
    active: 0-1000000
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_BAD_FILTER.

This filter affects the GPIO samples returned to callbacks set up
with [*callback*], [*callback_ex*] and [*wait_for_edge*].

It does not affect levels read by [*gpio_read*],
[*read_bank_1*], or [*read_bank_2*].

Level changes before and after the active period may
be reported.  Your software must be designed to cope with
such reports.


:call-seq:
 set_noise_filter(Integer pi,Integer user_gpio,Integer steady,Integer active) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_noise_filter]
*/
VALUE pigpio_rbfn_set_noise_filter(VALUE self,VALUE pi, VALUE user_gpio, VALUE steady, VALUE active){
  return INT2NUM(set_noise_filter(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(steady), NUM2UINT(active)));
}













/*
Return the current time in seconds since the Epoch.


:call-seq:
 time_time() -> Float

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#time_time]
*/
VALUE pigpio_rbfn_time_time(VALUE self){
  return DBL2NUM(time_time());
}
/*
Return a text description for an error code.

. .
 errnum: the error code.
. .


:call-seq:
 pigpio_error(Integer errnum) -> String

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#pigpio_error]
*/
VALUE pigpio_rbfn_pigpio_error(VALUE self,VALUE errnum){
  return rb_utf8_str_new_cstr(pigpio_error(NUM2INT(errnum)));
}
/*
Return the pigpiod_if2 version.

:call-seq:
 pigpiod_if_version() -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#pigpiod_if_version]
*/
VALUE pigpio_rbfn_pigpiod_if_version(VALUE self){
  return UINT2NUM(pigpiod_if_version());
}
/*
Gets the current system tick.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

Tick is the number of microseconds since system boot.

As tick is an unsigned 32 bit quantity it wraps around after
2**32 microseconds, which is approximately 1 hour 12 minutes.

:call-seq:
 get_current_tick(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_current_tick]
*/
VALUE pigpio_rbfn_get_current_tick(VALUE self,VALUE pi){
  return ULONG2NUM(get_current_tick(NUM2INT(pi)));
}
/*
Get the Pi's hardware revision number.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

The hardware revision is the last few characters on the Revision line
of /proc/cpuinfo.

If the hardware revision can not be found or is not a valid
hexadecimal number the function returns 0.

The revision number can be used to determine the assignment of GPIO
to pins (see [*gpio*]).

There are at least three types of board.

Type 1 boards have hardware revision numbers of 2 and 3.

Type 2 boards have hardware revision numbers of 4, 5, 6, and 15.

Type 3 boards have hardware revision numbers of 16 or greater.

:call-seq:
 get_hardware_revision(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_hardware_revision]
*/
VALUE pigpio_rbfn_get_hardware_revision(VALUE self,VALUE pi){
  return ULONG2NUM(get_hardware_revision(NUM2INT(pi)));
}
/*
Returns the pigpio version.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 get_pigpio_version(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_pigpio_version]
*/
VALUE pigpio_rbfn_get_pigpio_version(VALUE self,VALUE pi){
  return ULONG2NUM(get_pigpio_version(NUM2INT(pi)));
}
/*
This function clears all waveforms and any data added by calls to the
[*wave_add_**] functions.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

Returns 0 if OK.

:call-seq:
 wave_clear(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_clear]
*/
VALUE pigpio_rbfn_wave_clear(VALUE self,VALUE pi){
  return INT2NUM(wave_clear(NUM2INT(pi)));
}
/*
This function starts a new empty waveform.  You wouldn't normally need
to call this function as it is automatically called after a waveform is
created with the [*wave_create*] function.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

Returns 0 if OK.


:call-seq:
 wave_add_new(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_add_new]
*/
VALUE pigpio_rbfn_wave_add_new(VALUE self,VALUE pi){
  return INT2NUM(wave_add_new(NUM2INT(pi)));
}
/*
This function adds a number of pulses to the current waveform.

. .
       pi: >=0 (as returned by [*pigpio_start*]).
numPulses: the number of pulses.
   pulses: an array of pulses.
. .

Returns the new total number of pulses in the current waveform if OK,
otherwise PI_TOO_MANY_PULSES.

The pulses are interleaved in time order within the existing waveform
(if any).

Merging allows the waveform to be built in parts, that is the settings
for GPIO#1 can be added, and then GPIO#2 etc.

If the added waveform is intended to start after or within the existing
waveform then the first pulse should consist solely of a delay.

:call-seq:
 wave_add_generic(Integer pi,[PIGPIO::Pulse] pulses) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_add_generic]
*/
VALUE pigpio_rbfn_wave_add_generic(VALUE self,VALUE pi, VALUE pulses){
  unsigned numPulses=rb_array_len(pulses);
  gpioPulse_t buf[numPulses];
  gpioPulse_t *st;
  unsigned i;
  for(i=0;i<numPulses;i++){
    TypedData_Get_Struct(rb_ary_entry(pulses,i),gpioPulse_t,&gpioPulse_data_type,st);
    buf[i]=*st;
  }
  return INT2NUM( wave_add_generic(NUM2INT(pi),numPulses, buf));
}
/*
This function adds a waveform representing serial data to the
existing waveform (if any).  The serial data starts offset
microseconds from the start of the waveform.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
      baud: 50-1000000
 data_bits: number of data bits (1-32)
 stop_bits: number of stop half bits (2-8)
    offset: >=0
  numBytes: >=1
       str: an array of chars.
. .

Returns the new total number of pulses in the current waveform if OK,
otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD, PI_BAD_DATABITS,
PI_BAD_STOP_BITS, PI_TOO_MANY_CHARS, PI_BAD_SER_OFFSET,
or PI_TOO_MANY_PULSES.

NOTES:

The serial data is formatted as one start bit, [*data_bits*] data bits,
and [*stop_bits*]/2 stop bits.

It is legal to add serial data streams with different baud rates to
the same waveform.

[*numBytes*] is the number of bytes of data in str.

The bytes required for each character depend upon [*data_bits*].

For [*data_bits*] 1-8 there will be one byte per character. 
For [*data_bits*] 9-16 there will be two bytes per character. 
For [*data_bits*] 17-32 there will be four bytes per character.

:call-seq:
 wave_add_serial(Integer pi, Integer user_gpio, Integer baud, Integer data_bits, Integer stop_bits, Integer offset, String str) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_add_serial]
*/
VALUE pigpio_rbfn_wave_add_serial(VALUE self,VALUE pi, VALUE user_gpio, VALUE baud, VALUE data_bits, VALUE stop_bits, VALUE offset, VALUE str){
  int ret=wave_add_serial(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(baud), NUM2UINT(data_bits), NUM2UINT(stop_bits), NUM2UINT(offset),RSTRING_LEN(str),StringValuePtr(str));
  RB_GC_GUARD(str);
  return INT2NUM(ret);
}
/*
This function creates a waveform from the data provided by the prior
calls to the [*wave_add_**] functions.  Upon success a wave id
greater than or equal to 0 is returned, otherwise PI_EMPTY_WAVEFORM,
PI_TOO_MANY_CBS, PI_TOO_MANY_OOL, or PI_NO_WAVEFORM_ID.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

The data provided by the [*wave_add_**] functions is consumed by this
function.

As many waveforms may be created as there is space available.  The
wave id is passed to [*wave_send_**] to specify the waveform to transmit.

Normal usage would be

Step 1. [*wave_clear*] to clear all waveforms and added data.

Step 2. [*wave_add_**] calls to supply the waveform data.

Step 3. [*wave_create*] to create the waveform and get a unique id

Repeat steps 2 and 3 as needed.

Step 4. [*wave_send_**] with the id of the waveform to transmit.

A waveform comprises one or more pulses.  Each pulse consists of a
PIGPIO::Pulse class.

The fields specify

1) the GPIO to be switched on at the start of the pulse. 
2) the GPIO to be switched off at the start of the pulse. 
3) the delay in microseconds before the next pulse. 

Any or all the fields can be zero.  It doesn't make any sense to
set all the fields to zero (the pulse will be ignored).

When a waveform is started each pulse is executed in order with the
specified delay between the pulse and the next.

Returns the new waveform id if OK, otherwise PI_EMPTY_WAVEFORM,
PI_NO_WAVEFORM_ID, PI_TOO_MANY_CBS, or PI_TOO_MANY_OOL.

:call-seq:
 wave_create(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_create]
*/
VALUE pigpio_rbfn_wave_create(VALUE self,VALUE pi){
  return INT2NUM(wave_create(NUM2INT(pi)));
}
/*
This function deletes the waveform with id wave_id.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
 wave_id: >=0, as returned by [*wave_create*].
. .

Wave ids are allocated in order, 0, 1, 2, etc.

Returns 0 if OK, otherwise PI_BAD_WAVE_ID.

:call-seq:
 wave_delete(Integer pi,Integer wave_id) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_delete]
*/
VALUE pigpio_rbfn_wave_delete(VALUE self,VALUE pi, VALUE wave_id){
  return INT2NUM(wave_delete(NUM2INT(pi), NUM2UINT(wave_id)));
}
/*
This function transmits the waveform with id wave_id.  The waveform
is sent once.

NOTE: Any hardware PWM started by [*hardware_PWM*] will be cancelled.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
 wave_id: >=0, as returned by [*wave_create*].
. .

Returns the number of DMA control blocks in the waveform if OK,
otherwise PI_BAD_WAVE_ID, or PI_BAD_WAVE_MODE.

:call-seq:
 wave_send_once(Integer pi,Integer wave_id) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_send_once]
*/
VALUE pigpio_rbfn_wave_send_once(VALUE self,VALUE pi, VALUE wave_id){
  return INT2NUM(wave_send_once(NUM2INT(pi),NUM2UINT(wave_id)));
}
/*
This function transmits the waveform with id wave_id.  The waveform
cycles until cancelled (either by the sending of a new waveform or
by [*wave_tx_stop*]).

NOTE: Any hardware PWM started by [*hardware_PWM*] will be cancelled.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
wave_id: >=0, as returned by [*wave_create*].
. .

Returns the number of DMA control blocks in the waveform if OK,
otherwise PI_BAD_WAVE_ID, or PI_BAD_WAVE_MODE.

:call-seq:
 wave_send_repeat(Integer pi,Integer wave_id) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_send_repeat]
*/
VALUE pigpio_rbfn_wave_send_repeat(VALUE self,VALUE pi, VALUE wave_id){
  return INT2NUM(wave_send_repeat(NUM2INT(pi),NUM2UINT(wave_id)));
}
/*
Transmits the waveform with id wave_id using mode mode.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
 wave_id: >=0, as returned by [*wave_create*].
    mode: PI_WAVE_MODE_ONE_SHOT, PI_WAVE_MODE_REPEAT,
          PI_WAVE_MODE_ONE_SHOT_SYNC, or PI_WAVE_MODE_REPEAT_SYNC.
. .

PI_WAVE_MODE_ONE_SHOT: same as [*wave_send_once*].

PI_WAVE_MODE_REPEAT same as [*wave_send_repeat*].

PI_WAVE_MODE_ONE_SHOT_SYNC same as [*wave_send_once*] but tries
to sync with the previous waveform.

PI_WAVE_MODE_REPEAT_SYNC same as [*wave_send_repeat*] but tries
to sync with the previous waveform.

WARNING: bad things may happen if you delete the previous
waveform before it has been synced to the new waveform.

NOTE: Any hardware PWM started by [*hardware_PWM*] will be cancelled.

Returns the number of DMA control blocks in the waveform if OK,
otherwise PI_BAD_WAVE_ID, or PI_BAD_WAVE_MODE.


:call-seq:
 wave_send_using_mode(Integer pi,Integer wave_id) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_send_using_mode]
*/
VALUE pigpio_rbfn_wave_send_using_mode(VALUE self,VALUE pi, VALUE wave_id, VALUE mode){
  return INT2NUM(wave_send_using_mode(NUM2INT(pi),NUM2UINT(wave_id),NUM2UINT(mode)));
}
/*
This function transmits a chain of waveforms.

NOTE: Any hardware PWM started by [*hardware_PWM*] will be cancelled.

The waves to be transmitted are specified by the contents of buf
which contains an ordered list of [*wave_id*]s and optional command
codes and related data.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
     buf: pointer to the wave_ids and optional command codes
 bufSize: the number of bytes in buf
. .

Returns 0 if OK, otherwise PI_CHAIN_NESTING, PI_CHAIN_LOOP_CNT, PI_BAD_CHAIN_LOOP, PI_BAD_CHAIN_CMD, PI_CHAIN_COUNTER,
PI_BAD_CHAIN_DELAY, PI_CHAIN_TOO_BIG, or PI_BAD_WAVE_ID.

Each wave is transmitted in the order specified.  A wave may
occur multiple times per chain.

A blocks of waves may be transmitted multiple times by using
the loop commands. The block is bracketed by loop start and
end commands.  Loops may be nested.

Delays between waves may be added with the delay command.

The following command codes are supported:

 Name         @ Cmd & Data @ Meaning
 Loop Start   @ 255 0      @ Identify start of a wave block
 Loop Repeat  @ 255 1 x y  @ loop x + y*256 times
 Delay        @ 255 2 x y  @ delay x + y*256 microseconds
 Loop Forever @ 255 3      @ loop forever

If present Loop Forever must be the last entry in the chain.

The code is currently dimensioned to support a chain with roughly
600 entries and 20 loop counters.


:call-seq:
 wave_chain(Integer pi, [Integer] buf) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_chain]
*/
VALUE pigpio_rbfn_wave_chain(VALUE self,VALUE pi, VALUE buf){
  unsigned bufSize=rb_array_len(buf);
  char bufc[bufSize];
  unsigned i;
  for(i=0;i<bufSize;i++){
    bufc[i]=(char)FIX2INT(rb_ary_entry(buf,i));
  }
  return INT2NUM( wave_chain(NUM2INT(pi),bufc, bufSize));
}
/*
This function returns the id of the waveform currently being
transmitted.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

Returns the waveform id or one of the following special values:

PI_WAVE_NOT_FOUND (9998) - transmitted wave not found. 
PI_NO_TX_WAVE (9999) - no wave being transmitted.

:call-seq:
 wave_tx_at(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_tx_at]
*/
VALUE pigpio_rbfn_wave_tx_at(VALUE self,VALUE pi){
  return INT2NUM(wave_tx_at(NUM2INT(pi)));
}
/*
This function checks to see if a waveform is currently being
transmitted.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

Returns 1 if a waveform is currently being transmitted, otherwise 0.

:call-seq:
 wave_tx_busy(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_tx_busy]
*/
VALUE pigpio_rbfn_wave_tx_busy(VALUE self,VALUE pi){
  return INT2NUM(wave_tx_busy(NUM2INT(pi)));
}
/*
This function stops the transmission of the current waveform.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

Returns 0 if OK.

This function is intended to stop a waveform started with the repeat mode.

:call-seq:
 wave_tx_stop(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_tx_stop]
*/
VALUE pigpio_rbfn_wave_tx_stop(VALUE self,VALUE pi){
  return INT2NUM(wave_tx_stop(NUM2INT(pi)));
}
/*
This function returns the length in microseconds of the current
waveform.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_micros(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_micros]
*/
VALUE pigpio_rbfn_wave_get_micros(VALUE self,VALUE pi){
  return INT2NUM(wave_get_micros(NUM2INT(pi)));
}
/*
This function returns the length in microseconds of the longest waveform
created since the pigpio daemon was started.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_high_micros(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_high_micros]
*/
VALUE pigpio_rbfn_wave_get_high_micros(VALUE self,VALUE pi){
  return INT2NUM(wave_get_high_micros(NUM2INT(pi)));
}
/*
This function returns the maximum possible size of a waveform in 
microseconds.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_max_micros(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_max_micros]
*/
VALUE pigpio_rbfn_wave_get_max_micros(VALUE self,VALUE pi){
  return INT2NUM(wave_get_max_micros(NUM2INT(pi)));
}
/*
This function returns the length in pulses of the current waveform.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_pulses(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_pulses]
*/
VALUE pigpio_rbfn_wave_get_pulses(VALUE self,VALUE pi){
  return INT2NUM(wave_get_pulses(NUM2INT(pi)));
}
/*
This function returns the length in pulses of the longest waveform
created since the pigpio daemon was started.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_high_pulses(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_high_pulses]
*/
VALUE pigpio_rbfn_wave_get_high_pulses(VALUE self,VALUE pi){
  return INT2NUM(wave_get_high_pulses(NUM2INT(pi)));
}
/*
This function returns the maximum possible size of a waveform in pulses.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_max_pulses(Integer pi,Integer gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_max_pulses]
*/
VALUE pigpio_rbfn_wave_get_max_pulses(VALUE self,VALUE pi){
  return INT2NUM(wave_get_max_pulses(NUM2INT(pi)));
}
/*
This function returns the length in DMA control blocks of the current
waveform.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_cbs(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_cbs]
*/
VALUE pigpio_rbfn_wave_get_cbs(VALUE self,VALUE pi){
  return INT2NUM(wave_get_cbs(NUM2INT(pi)));
}
/*
This function returns the length in DMA control blocks of the longest
waveform created since the pigpio daemon was started.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_high_cbs(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_high_cbs]
*/
VALUE pigpio_rbfn_wave_get_high_cbs(VALUE self,VALUE pi){
  return INT2NUM(wave_get_high_cbs(NUM2INT(pi)));
}
/*
This function returns the maximum possible size of a waveform in DMA
control blocks.

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

:call-seq:
 wave_get_max_cbs(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wave_get_max_cbs]
*/
VALUE pigpio_rbfn_wave_get_max_cbs(VALUE self,VALUE pi){
  return INT2NUM(wave_get_max_cbs(NUM2INT(pi)));
}
/*
This function sends a trigger pulse to a GPIO.  The GPIO is set to
level for pulseLen microseconds and then reset to not level.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
  pulseLen: 1-100.
     level: 0,1.
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_LEVEL,
PI_BAD_PULSELEN, or PI_NOT_PERMITTED.

:call-seq:
 gpio_trigger(Integer pi,Integer user_gpio,Integer pulseLen,Integer level) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#gpio_trigger]
*/
VALUE pigpio_rbfn_gpio_trigger(VALUE self,VALUE pi, VALUE user_gpio, VALUE pulseLen, VALUE level){
  return INT2NUM(gpio_trigger(NUM2INT(pi),NUM2UINT(user_gpio),NUM2UINT(pulseLen),NUM2UINT(level)));
}
/*
This function stores a script for later execution.

See [[http://abyz.co.uk/rpi/pigpio/pigs.html#Scripts]] for details.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 script: the text of the script.
. .

The function returns a script id if the script is valid,
otherwise PI_BAD_SCRIPT.

:call-seq:
 store_script(Integer pi,String script) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#store_script]
*/
VALUE pigpio_rbfn_store_script(VALUE self,VALUE pi, VALUE script){
  //rb_gc_disable();
  int ret=store_script(NUM2INT(pi), StringValueCStr(script));
  //rb_gc_enable();
  RB_GC_GUARD(script);
  return INT2NUM(ret);
}
/*
This function runs a stored script.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 script_id: >=0, as returned by [*store_script*].
    numPar: 0-10, the number of parameters.
     param: an array of parameters.
. .

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID, or
PI_TOO_MANY_PARAM

param is an array of up to 10 parameters which may be referenced in
the script as p0 to p9.

:call-seq:
 rb_array_len(Integer pi,Integer script_id,[Integer] param) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#rb_array_len]
*/
VALUE pigpio_rbfn_run_script(VALUE self,VALUE pi, VALUE script_id, VALUE param){
  unsigned numPar=rb_array_len(param);
  uint32_t paramc[10];
  int ret;
  for(int i=(numPar<10)?numPar:10;i>=0;i--){
    paramc[i]=rb_ary_entry(param,i);
  }
  ret=run_script(NUM2INT(pi), NUM2UINT(script_id), numPar, paramc);
  RB_GC_GUARD(param);
  return INT2NUM(ret);
}
/*
This function returns the run status of a stored script as well
as the current values of parameters 0 to 9.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 script_id: >=0, as returned by [*store_script*].
     param: an array to hold the returned 10 parameters.
. .

The function returns greater than or equal to 0 if OK,
otherwise PI_BAD_SCRIPT_ID.

The run status may be

. .
 PI_SCRIPT_INITING
 PI_SCRIPT_HALTED
 PI_SCRIPT_RUNNING
 PI_SCRIPT_WAITING
 PI_SCRIPT_FAILED
. .

The current value of script parameters 0 to 9 are returned in param.

:call-seq:
 script_status(Integer pi,Integer script_id) -> [Integer,[Integer]param]

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#script_status]
*/
VALUE pigpio_rbfn_script_status(VALUE self,VALUE pi, VALUE script_id){
  uint32_t paramc[10];
  int ret=script_status(NUM2INT(pi), NUM2UINT(script_id), paramc);
  VALUE param=rb_ary_new_capa(10);
  for(int i=0;i<10;i++){
    rb_ary_store(param,i,ULONG2NUM(paramc[i]));
  }
  return rb_ary_new_from_args(2,param,INT2NUM(ret));
}
/*
This function stops a running script.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 script_id: >=0, as returned by [*store_script*].
. .

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.

:call-seq:
 stop_script(Integer pi,Integer script_id) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#stop_script]
*/
VALUE pigpio_rbfn_stop_script(VALUE self,VALUE pi, VALUE script_id){
  return INT2NUM( stop_script(NUM2INT(pi), NUM2UINT(script_id)));
}
/*
This function deletes a stored script.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 script_id: >=0, as returned by [*store_script*].
. .

The function returns 0 if OK, otherwise PI_BAD_SCRIPT_ID.

:call-seq:
 delete_script(Integer pi,Integer script_id) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#delete_script]
*/
VALUE pigpio_rbfn_delete_script(VALUE self,VALUE pi, VALUE script_id){
  return INT2NUM( delete_script(NUM2INT(pi), NUM2UINT(script_id)));
}
/*
Read the levels of the bank 1 GPIO (GPIO 0-31).

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

The returned 32 bit integer has a bit set if the corresponding
GPIO is logic 1.  GPIO n has bit value (1<<n).

:call-seq:
 read_bank_1(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#read_bank_1]
*/
VALUE pigpio_rbfn_read_bank_1(VALUE self, VALUE pi){
  return ULONG2NUM( read_bank_1(NUM2INT(pi)));
}
/*
Read the levels of the bank 2 GPIO (GPIO 32-53).

. .
 pi: >=0 (as returned by [*pigpio_start*]).
. .

The returned 32 bit integer has a bit set if the corresponding
GPIO is logic 1.  GPIO n has bit value (1<<(n-32)).

:call-seq:
 read_bank_2(Integer pi) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#read_bank_2]
*/
VALUE pigpio_rbfn_read_bank_2(VALUE self, VALUE pi){
  return ULONG2NUM( read_bank_2(NUM2INT(pi)));
}
/*
Clears GPIO 0-31 if the corresponding bit in bits is set.

. .
   pi: >=0 (as returned by [*pigpio_start*]).
 bits: a bit mask with 1 set if the corresponding GPIO is
       to be cleared.
. .

Returns 0 if OK, otherwise PI_SOME_PERMITTED.

A status of PI_SOME_PERMITTED indicates that the user is not
allowed to write to one or more of the GPIO.

:call-seq:
 clear_bank_1(Integer pi,Integer bits) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#clear_bank_1]
*/
VALUE pigpio_rbfn_clear_bank_1(VALUE self, VALUE pi, VALUE bits){
  return INT2NUM( clear_bank_1(NUM2INT(pi), NUM2ULONG(bits)));
}
/*
Clears GPIO 32-53 if the corresponding bit (0-21) in bits is set.

. .
   pi: >=0 (as returned by [*pigpio_start*]).
 bits: a bit mask with 1 set if the corresponding GPIO is
       to be cleared.
. .

Returns 0 if OK, otherwise PI_SOME_PERMITTED.

A status of PI_SOME_PERMITTED indicates that the user is not
allowed to write to one or more of the GPIO.

:call-seq:
 clear_bank_2(Integer pi,Integer bits) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#clear_bank_2]
*/
VALUE pigpio_rbfn_clear_bank_2(VALUE self, VALUE pi, VALUE bits){
  return INT2NUM( clear_bank_2(NUM2INT(pi), NUM2ULONG(bits)));
}
/*
Sets GPIO 0-31 if the corresponding bit in bits is set.

. .
   pi: >=0 (as returned by [*pigpio_start*]).
 bits: a bit mask with 1 set if the corresponding GPIO is
       to be set.
. .

Returns 0 if OK, otherwise PI_SOME_PERMITTED.

A status of PI_SOME_PERMITTED indicates that the user is not
allowed to write to one or more of the GPIO.

:call-seq:
 set_bank_1(Integer pi,Integer bits) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_bank_1]
*/
VALUE pigpio_rbfn_set_bank_1(VALUE self, VALUE pi, VALUE bits){
  return INT2NUM( set_bank_1(NUM2INT(pi), NUM2ULONG(bits)));
}
/*
Sets GPIO 32-53 if the corresponding bit (0-21) in bits is set.

. .
   pi: >=0 (as returned by [*pigpio_start*]).
 bits: a bit mask with 1 set if the corresponding GPIO is
       to be set.
. .

Returns 0 if OK, otherwise PI_SOME_PERMITTED.

A status of PI_SOME_PERMITTED indicates that the user is not
allowed to write to one or more of the GPIO.

:call-seq:
 set_bank_2(Integer pi,Integer bits) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_bank_2]
*/
VALUE pigpio_rbfn_set_bank_2(VALUE self, VALUE pi, VALUE bits){
  return INT2NUM( set_bank_2(NUM2INT(pi), NUM2ULONG(bits)));
}
/*
Starts a hardware clock on a GPIO at the specified frequency.
Frequencies above 30MHz are unlikely to work.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
      gpio: see description
 frequency: 0 (off) or 4689-250000000 (250M)
. .

Returns 0 if OK, otherwise PI_NOT_PERMITTED, PI_BAD_GPIO,
PI_NOT_HCLK_GPIO, PI_BAD_HCLK_FREQ,or PI_BAD_HCLK_PASS.

The same clock is available on multiple GPIO.  The latest
frequency setting will be used by all GPIO which share a clock.

The GPIO must be one of the following.

. .
 4   clock 0  All models
 5   clock 1  All models but A and B (reserved for system use)
 6   clock 2  All models but A and B
 20  clock 0  All models but A and B
 21  clock 1  All models but A and Rev.2 B (reserved for system use)
 
 32  clock 0  Compute module only
 34  clock 0  Compute module only
 42  clock 1  Compute module only (reserved for system use)
 43  clock 2  Compute module only
 44  clock 1  Compute module only (reserved for system use)
. .

Access to clock 1 is protected by a password as its use will likely
crash the Pi.  The password is given by or'ing 0x5A000000 with the
GPIO number.

:call-seq:
 hardware_clock(Integer pi,Integer gpio, Integer clkfreq) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#hardware_clock]
*/
VALUE pigpio_rbfn_hardware_clock(VALUE self, VALUE pi, VALUE gpio, VALUE clkfreq){
  return INT2NUM( hardware_clock(NUM2INT(pi), NUM2UINT(gpio), NUM2UINT(clkfreq)));
}
/*
Starts hardware PWM on a GPIO at the specified frequency and dutycycle.
Frequencies above 30MHz are unlikely to work.

NOTE: Any waveform started by [*wave_send_**] or [*wave_chain*]
will be cancelled.

This function is only valid if the pigpio main clock is PCM.  The
main clock defaults to PCM but may be overridden when the pigpio
daemon is started (option -t).

. .
      pi: >=0 (as returned by [*pigpio_start*]).
    gpio: see descripton
 PWMfreq: 0 (off) or 1-125000000 (125M)
 PWMduty: 0 (off) to 1000000 (1M)(fully on)
. .

Returns 0 if OK, otherwise PI_NOT_PERMITTED, PI_BAD_GPIO,
PI_NOT_HPWM_GPIO, PI_BAD_HPWM_DUTY, PI_BAD_HPWM_FREQ,
or PI_HPWM_ILLEGAL.

The same PWM channel is available on multiple GPIO.  The latest
frequency and dutycycle setting will be used by all GPIO which
share a PWM channel.

The GPIO must be one of the following.

. .
 12  PWM channel 0  All models but A and B
 13  PWM channel 1  All models but A and B
 18  PWM channel 0  All models
 19  PWM channel 1  All models but A and B
 
 40  PWM channel 0  Compute module only
 41  PWM channel 1  Compute module only
 45  PWM channel 1  Compute module only
 52  PWM channel 0  Compute module only
 53  PWM channel 1  Compute module only
. .

The actual number of steps beween off and fully on is the
integral part of 250 million divided by PWMfreq.

The actual frequency set is 250 million / steps.

There will only be a million steps for a PWMfreq of 250.
Lower frequencies will have more steps and higher
frequencies will have fewer steps.  PWMduty is
automatically scaled to take this into account.

:call-seq:
 hardware_PWM(Integer pi,Integer gpio, Integer PWMfreq, Integer PWMduty) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#hardware_PWM]
*/
VALUE pigpio_rbfn_hardware_PWM(VALUE self, VALUE pi, VALUE gpio, VALUE PWMfreq, VALUE PWMduty){
  return INT2NUM( hardware_PWM(NUM2INT(pi), NUM2UINT(gpio), NUM2UINT(PWMfreq), NUM2ULONG(PWMduty)));
}

/*
This function initialises a new callback.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
      edge: RISING_EDGE, FALLING_EDGE, or EITHER_EDGE.
. .

The function returns a callback id if OK, otherwise pigif_bad_malloc,
pigif_duplicate_callback, or pigif_bad_callback.

The callback is called with the GPIO, edge, and tick, whenever the
GPIO has the identified edge.

. .
 Parameter   Value    Meaning
 
 GPIO        0-31     The GPIO which has changed state
 
 edge        0-2      0 = change to low (a falling edge)
                      1 = change to high (a rising edge)
                      2 = no level change (a watchdog timeout)
 
 tick        32 bit   The number of microseconds since boot
                      WARNING: this wraps around from
                      4294967295 to 0 roughly every 72 minutes
. .

:call-seq:
 callback(Integer pi,Integer user_gpio, Integer edge){|tick,level,gpio,pi| ... } -> Pigpio::Callback

If you call this method without a block, this method raises an Pigpio::CallbackError exception.

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#callback_ex]
*/
VALUE pigpio_rbfn_callback(int argc, VALUE *argv, VALUE self){
  int id;
  VALUE queue;
  VALUE thread;
  callback_pqueue_t *st;
  VALUE pi; VALUE user_gpio; VALUE edge; VALUE block;
  rb_scan_args(argc,argv,"3&",&pi,&user_gpio,&edge,&block);
  if(block==Qnil){
    return Qnil;
  }
  queue=pigpio_rbst_callback_pqueue_make_inner(block,0,100000);
  thread=rb_thread_create(callback_receive,(void*)queue);
  st=TypedData_Get_Struct2(queue,callback_pqueue_t,&callback_pqueue_type);

  //if(NIL_P(callee_proc)){rb_raise(cCallbackError,"No callback block.\n");}
  id=callback_ex(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(edge), pigpio_rbbk_CBFuncEx, (void*)st->queue);
  return pigpio_rbst_callback_id_make_inner(id,callback_cancel,queue,thread);
}
/*
This function cancels a callback identified by its id.

. .
 callback_id: >=0, as returned by a call to [*callback*] or [*callback_ex*].
. .

The function returns 0 if OK, otherwise pigif_callback_not_found.

:call-seq:
 callback_cancel(Integer callback_id) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#callback_cancel]
*/
VALUE pigpio_rbfn_callback_cancel(VALUE self,VALUE callback){
  return INT2NUM( callback_cancel(NUM2UINT(callback)));
}
/*
This function waits for an edge on the GPIO for up to timeout
seconds.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
      edge: RISING_EDGE, FALLING_EDGE, or EITHER_EDGE.
   timeout: >=0.
. .

The function returns when the edge occurs or after the timeout.

Do not use this function for precise timing purposes,
the edge is only checked 20 times a second. Whenever
you need to know the accurate time of GPIO events use
a [*callback*] function.

The function returns 1 if the edge occurred, otherwise 0.

:call-seq:
 wait_for_edge(Integer pi,Integer user_gpio, Integer edge, Float timeout) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wait_for_edge]
*/
VALUE pigpio_rbfn_wait_for_edge(VALUE self,VALUE pi, VALUE user_gpio, VALUE edge, VALUE timeout){
  return INT2NUM( wait_for_edge(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(edge), NUM2DBL(timeout)));
}

/*
This function initialises an event callback.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
   event: 0-31.
. .

The function returns a callback id if OK, otherwise pigif_bad_malloc,
pigif_duplicate_callback, or pigif_bad_callback.

The callback is called with the event id, and tick, whenever the
event occurs.

:call-seq:
 event_callback(Integer pi,Integer event){|tick,event| } -> Pigpio::Callback

If you call this method without a block, this method raises an Pigpio::CallbackError exception.

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#event_callback_ex]
*/
VALUE pigpio_rbfn_event_callback(int argc, VALUE *argv, VALUE self){
  int id;
  VALUE pi; VALUE event; VALUE queue; VALUE thread;
  rb_scan_args(argc,argv,"4",&pi,&event,&queue,&thread);
  id=event_callback_ex(NUM2INT(pi), NUM2UINT(event), pigpio_rbbk_evtCBFuncEx, (void *)queue);
  return pigpio_rbst_callback_id_make_inner(id,event_callback_cancel,queue,thread);
}
/*
This function cancels an event callback identified by its id.

. .
 callback_id: >=0, as returned by a call to [*event_callback*] or [*event_callback_ex*].
. .

The function returns 0 if OK, otherwise pigif_callback_not_found.

:call-seq:
 event_callback_cancel(Integer callback_id) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#event_callback_cancel]
*/
VALUE pigpio_rbfn_event_callback_cancel(VALUE self,VALUE callback_id){
  return INT2NUM( event_callback_cancel(NUM2UINT(callback_id) ));
}
/*
This function waits for an event for up to timeout seconds.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
   event: 0-31.
 timeout: >=0.
. .

The function returns when the event occurs or after the timeout.

The function returns 1 if the event occurred, otherwise 0.

:call-seq:
 wait_for_event(Integer pi,Integer event, Float timeout) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#wait_for_event]
*/
VALUE pigpio_rbfn_wait_for_event(VALUE self,VALUE pi, VALUE event, VALUE timeout){
  return INT2NUM( wait_for_event(NUM2INT(pi), NUM2UINT(event), NUM2DBL(timeout)));
}
/*
This function signals the occurrence of an event.

. .
    pi: >=0 (as returned by [*pigpio_start*]).
 event: 0-31.
. .

Returns 0 if OK, otherwise PI_BAD_EVENT_ID.

An event is a signal used to inform one or more consumers
to start an action.  Each consumer which has registered an interest
in the event (e.g. by calling [*event_callback*]) will be informed by
a callback.

One event, PI_EVENT_BSC (31) is predefined.  This event is
auto generated on BSC slave activity.

The meaning of other events is arbitrary.

Note that other than its id and its tick there is no data associated
with an event.

:call-seq:
 event_trigger(Integer pi,Integer event) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#event_trigger]
*/
VALUE pigpio_rbfn_event_trigger(VALUE self,VALUE pi, VALUE event){
  return INT2NUM( event_trigger(NUM2INT(pi), NUM2UINT(event)));
}

/*
This function opens a serial device at a specified baud rate
with specified flags.  The device name must start with
/dev/tty or /dev/serial.


. .
        pi: >=0 (as returned by [*pigpio_start*]).
   ser_tty: the serial device to open.
      baud: the baud rate in bits per second, see below.
 ser_flags: 0.
. .

Returns a handle (>=0) if OK, otherwise PI_NO_HANDLE, or
PI_SER_OPEN_FAILED.

The baud rate must be one of 50, 75, 110, 134, 150,
200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200,
38400, 57600, 115200, or 230400.

No flags are currently defined.  This parameter should be set to zero.

:call-seq:
 serial_open(Integer pi,String ser_tty, Integer baud, Integer ser_flags) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#serial_open]
*/
VALUE pigpio_rbfn_serial_open(VALUE self, VALUE pi, VALUE ser_tty, VALUE baud, VALUE ser_flags){
  int ret=serial_open(NUM2INT(pi), StringValueCStr(ser_tty), NUM2UINT(baud), NUM2UINT(ser_flags));
  RB_GC_GUARD(ser_tty);
  return INT2NUM(ret);
}
/*
This function closes the serial device associated with handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*serial_open*].
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.

:call-seq:
 serial_close(Integer pi,Integer handle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#serial_close]
*/
VALUE pigpio_rbfn_serial_close(VALUE self, VALUE pi, VALUE handle){
  return INT2NUM( serial_close(NUM2INT(pi), NUM2UINT(handle)));
}
/*
This function writes bVal to the serial port associated with handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*serial_open*].
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_SER_WRITE_FAILED.

:call-seq:
 serial_write_byte(Integer pi,Integer handle, Integer bVal) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#serial_write_byte]
*/
VALUE pigpio_rbfn_serial_write_byte(VALUE self, VALUE pi, VALUE handle, VALUE bVal){
  return INT2NUM( serial_write_byte(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(bVal)));
}
/*
This function reads a byte from the serial port associated with handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*serial_open*].
. .

Returns the read byte (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_SER_READ_NO_DATA, or PI_SER_READ_FAILED.

If no data is ready PI_SER_READ_NO_DATA is returned.

:call-seq:
 serial_read_byte(Integer pi,Integer handle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#serial_read_byte]
*/
VALUE pigpio_rbfn_serial_read_byte(VALUE self, VALUE pi, VALUE handle){
  return INT2NUM( serial_read_byte(NUM2INT(pi), NUM2UINT(handle)));
}
/*
This function writes count bytes from buf to the the serial port
associated with handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*serial_open*].
    buf: the array of bytes to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_SER_WRITE_FAILED.

:call-seq:
 serial_write(Integer pi,Integer handle, String buf) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#serial_write]
*/
VALUE pigpio_rbfn_serial_write(VALUE self, VALUE pi, VALUE handle, VALUE buf){
  int ret=serial_write(NUM2INT(pi), NUM2UINT(handle), StringValuePtr(buf), RSTRING_LEN(buf));
  RB_GC_GUARD(buf);
  return INT2NUM(ret);
}
/*
This function reads up to count bytes from the the serial port
associated with handle and writes them to buf.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*serial_open*].
    buf: an array to receive the read data.
  count: the maximum number of bytes to read.
. .

Returns the number of bytes read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, PI_SER_READ_NO_DATA, or PI_SER_WRITE_FAILED.

If no data is ready zero is returned.

:call-seq:
 serial_read(Integer pi,Integer handle, Integer count) -> Integer (When onError)
 serial_read(Integer pi,Integer handle, Integer count) -> String buf (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#serial_read]
*/
VALUE pigpio_rbfn_serial_read(VALUE self, VALUE pi, VALUE handle, VALUE count){
  unsigned countc=NUM2UINT(count);
  VALUE buf=rb_str_new("",countc);//#<Encoding:ASCII-8BIT>;
  int ret=serial_read(NUM2INT(pi), NUM2UINT(handle), (void*)StringValuePtr(buf), countc);
  return (ret < 0)?INT2NUM(ret):rb_str_resize(buf,ret);
  //if(ret < 0){buf=Qnil;}
  //return rb_ary_new_from_args(2,buf,INT2NUM(ret));
}
/*
Returns the number of bytes available to be read from the
device associated with handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*serial_open*].
. .

Returns the number of bytes of data available (>=0) if OK,
otherwise PI_BAD_HANDLE.

:call-seq:
 serial_data_available(Integer pi,Integer handle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#serial_data_available]
*/
VALUE pigpio_rbfn_serial_data_available(VALUE self, VALUE pi, VALUE handle){
  return INT2NUM( serial_data_available(NUM2INT(pi), NUM2UINT(handle)));
}
/*
This function opens a GPIO for bit bang reading of serial data.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31.
      baud: 50-250000
 data_bits: 1-32
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_WAVE_BAUD,
or PI_GPIO_IN_USE.

The serial data is returned in a cyclic buffer and is read using
bb_serial_read.

It is the caller's responsibility to read data from the cyclic buffer
in a timely fashion.

:call-seq:
 bb_serial_read_open(Integer pi,Integer user_gpio, Integer baud, Integer data_bits) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_serial_read_open]
*/
VALUE pigpio_rbfn_bb_serial_read_open(VALUE self, VALUE pi, VALUE user_gpio, VALUE baud, VALUE data_bits){
  return INT2NUM( bb_serial_read_open(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(baud), NUM2UINT(data_bits)));
}
/*
This function copies up to bufSize bytes of data read from the
bit bang serial cyclic buffer to the buffer starting at buf.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31, previously opened with [*bb_serial_read_open*].
       buf: an array to receive the read bytes.
   bufSize: >=0
. .

Returns the number of bytes copied if OK, otherwise PI_BAD_USER_GPIO
or PI_NOT_SERIAL_GPIO.

The bytes returned for each character depend upon the number of
data bits [*data_bits*] specified in the [*bb_serial_read_open*] command.

* For [*data_bits*] 1-8 there will be one byte per character. 
* For [*data_bits*] 9-16 there will be two bytes per character. 
* For [*data_bits*] 17-32 there will be four bytes per character.

:call-seq:
 bb_serial_read(Integer pi,Integer user_gpio, Integer bufSize) -> Integer (When onError)
 bb_serial_read(Integer pi,Integer user_gpio, Integer bufSize) -> String  (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_serial_read]
*/
VALUE pigpio_rbfn_bb_serial_read(VALUE self, VALUE pi, VALUE user_gpio, VALUE bufSize){
  size_t countc=NUM2SIZET(bufSize);
  VALUE buf=rb_str_new("",countc);//#<Encoding:ASCII-8BIT>;
  int ret=bb_serial_read(NUM2INT(pi), NUM2UINT(user_gpio), (void*)StringValuePtr(buf), countc);
  return (ret < 0)?INT2NUM(ret):rb_str_resize(buf,ret);
  //if(ret < 0){buf=Qnil;}
  //return rb_ary_new_from_args(2,buf,INT2NUM(ret));
}
/*
This function closes a GPIO for bit bang reading of serial data.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31, previously opened with [*bb_serial_read_open*].
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_SERIAL_GPIO.

:call-seq:
 bb_serial_read_close(Integer pi,Integer user_gpio) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_serial_read_close]
*/
VALUE pigpio_rbfn_bb_serial_read_close(VALUE self, VALUE pi, VALUE user_gpio){
  return INT2NUM( bb_serial_read_close(NUM2INT(pi), NUM2UINT(user_gpio)));
}
/*
This function inverts serial logic for big bang serial reads.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
 user_gpio: 0-31, previously opened with [*bb_serial_read_open*].
    invert: 0-1, 1 invert, 0 normal.
. .

Returns 0 if OK, otherwise PI_NOT_SERIAL_GPIO or PI_BAD_SER_INVERT.

:call-seq:
 bb_serial_invert(Integer pi,Integer user_gpio, Integer invert) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_serial_invert]
*/
VALUE pigpio_rbfn_bb_serial_invert(VALUE self, VALUE pi, VALUE user_gpio, VALUE invert){
  return INT2NUM( bb_serial_invert(NUM2INT(pi), NUM2UINT(user_gpio), NUM2UINT(invert)));
}


/*
This function selects a set of GPIO for bit banging SPI at a
specified baud rate.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
        CS: 0-31
      MISO: 0-31
      MOSI: 0-31
      SCLK: 0-31
      baud: 50-250000
 spi_flags: see below
. .

spi_flags consists of the least significant 22 bits.

. .
 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
  0  0  0  0  0  0  R  T  0  0  0  0  0  0  0  0  0  0  0  p  m  m
. .

mm defines the SPI mode, defaults to 0

. .
 Mode CPOL CPHA
  0    0    0
  1    0    1
  2    1    0
  3    1    1
. .

p is 0 if CS is active low (default) and 1 for active high.

T is 1 if the least significant bit is transmitted on MOSI first, the
default (0) shifts the most significant bit out first.

R is 1 if the least significant bit is received on MISO first, the
default (0) receives the most significant bit first.

The other bits in flags should be set to zero.

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_SPI_BAUD, or
PI_GPIO_IN_USE.

If more than one device is connected to the SPI bus (defined by
SCLK, MOSI, and MISO) each must have its own CS.

...
 bb_spi_open(pi,10, MISO, MOSI, SCLK, 10000, 0); // device 1
 bb_spi_open(pi,11, MISO, MOSI, SCLK, 20000, 3); // device 2
...

:call-seq:
 bb_spi_open(Integer pi,Integer CS, Integer MISO, Integer MOSI, Integer SCLK, Integer baud, Integer spi_flags) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_spi_open]
*/
VALUE pigpio_rbfn_bb_spi_open(VALUE self, VALUE pi, VALUE CS, VALUE MISO, VALUE MOSI, VALUE SCLK, VALUE baud, VALUE spi_flags){
  return INT2NUM( bb_spi_open(NUM2INT(pi), NUM2UINT(CS), NUM2UINT(MISO), NUM2UINT(MOSI), NUM2UINT(SCLK), NUM2UINT(baud), NUM2UINT(spi_flags)));
}
/*
This function stops bit banging SPI on a set of GPIO
opened with [*bbSPIOpen*].

. .
 pi: >=0 (as returned by [*pigpio_start*]).
 CS: 0-31, the CS GPIO used in a prior call to [*bb_spi_open*]
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_SPI_GPIO.

:call-seq:
 bb_spi_close(Integer pi,Integer CS) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_spi_close]
*/
VALUE pigpio_rbfn_bb_spi_close(VALUE self, VALUE pi, VALUE CS){
  return INT2NUM( bb_spi_close(NUM2INT(pi), NUM2UINT(CS)));
}
/*
This function executes a bit banged SPI transfer.

. .
    pi: >=0 (as returned by [*pigpio_start*]).
    CS: 0-31 (as used in a prior call to [*bb_spi_open*])
 txBuf: binary string buffer to hold data to be sent
 rxBuf: binary string buffer to hold returned data
. .

Returns >= 0 if OK (the number of bytes read), otherwise
PI_BAD_USER_GPIO, PI_NOT_SPI_GPIO or PI_BAD_POINTER.

:call-seq:
 bb_spi_xfer(Integer pi,Integer CS, String txBuf) -> Integer (When onError)
 bb_spi_xfer(Integer pi,Integer CS, String txBuf) -> String rxBuf (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_spi_xfer]
*/
VALUE pigpio_rbfn_bb_spi_xfer(VALUE self, VALUE pi, VALUE CS, VALUE txBuf){
  unsigned countc=RSTRING_LEN(txBuf);
  VALUE rxBuf=rb_str_new("",countc);//#<Encoding:ASCII-8BIT>;
  int ret=bb_spi_xfer(NUM2INT(pi), NUM2UINT(CS), StringValuePtr(txBuf), StringValuePtr(rxBuf), countc);
  RB_GC_GUARD(txBuf);
  return (ret < 0)?INT2NUM(ret):rb_str_resize(rxBuf,ret);
  //if(ret < 0){rxBuf=Qnil;}
  //return rb_ary_new_from_args(2,rxBuf,INT2NUM(ret));
}
/*
This function returns a handle for the SPI device on channel.
Data will be transferred at baud bits per second.  The flags may
be used to modify the default behaviour of 4-wire operation, mode 0,
active low chip select.

An auxiliary SPI device is available on all models but the
A and B and may be selected by setting the A bit in the
flags.  The auxiliary device has 3 chip selects and a
selectable word size in bits.

. .
          pi: >=0 (as returned by [*pigpio_start*]).
 spi_channel: 0-1 (0-2 for the auxiliary device).
        baud: 32K-125M (values above 30M are unlikely to work).
   spi_flags: see below.
. .

Returns a handle (>=0) if OK, otherwise PI_BAD_SPI_CHANNEL,
PI_BAD_SPI_SPEED, PI_BAD_FLAGS, PI_NO_AUX_SPI, or PI_SPI_OPEN_FAILED.

spi_flags consists of the least significant 22 bits.

. .
 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
  b  b  b  b  b  b  R  T  n  n  n  n  W  A u2 u1 u0 p2 p1 p0  m  m
. .

mm defines the SPI mode.

Warning: modes 1 and 3 do not appear to work on the auxiliary device.

. .
 Mode POL PHA
  0    0   0
  1    0   1
  2    1   0
  3    1   1
. .

px is 0 if CEx is active low (default) and 1 for active high.

ux is 0 if the CEx GPIO is reserved for SPI (default) and 1 otherwise.

A is 0 for the standard SPI device, 1 for the auxiliary SPI.

W is 0 if the device is not 3-wire, 1 if the device is 3-wire.  Standard
SPI device only.

nnnn defines the number of bytes (0-15) to write before switching
the MOSI line to MISO to read data.  This field is ignored
if W is not set.  Standard SPI device only.

T is 1 if the least significant bit is transmitted on MOSI first, the
default (0) shifts the most significant bit out first.  Auxiliary SPI
device only.

R is 1 if the least significant bit is received on MISO first, the
default (0) receives the most significant bit first.  Auxiliary SPI
device only.

bbbbbb defines the word size in bits (0-32).  The default (0)
sets 8 bits per word.  Auxiliary SPI device only.

The [*spi_read*], [*spi_write*], and [*spi_xfer*] functions
transfer data packed into 1, 2, or 4 bytes according to
the word size in bits.

For bits 1-8 there will be one byte per character. 
For bits 9-16 there will be two bytes per character. 
For bits 17-32 there will be four bytes per character.

Multi-byte transfers are made in least significant byte first order.

E.g. to transfer 32 11-bit words buf should contain 64 bytes
and count should be 64.

E.g. to transfer the 14 bit value 0x1ABC send the bytes 0xBC followed
by 0x1A.

The other bits in flags should be set to zero.

:call-seq:
 spi_open(Integer pi,Integer spi_channel, Integer baud, Integer spi_flags) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#spi_open]
*/
VALUE pigpio_rbfn_spi_open(VALUE self, VALUE pi, VALUE spi_channel, VALUE baud, VALUE spi_flags){
  return INT2NUM( spi_open(NUM2INT(pi), NUM2UINT(spi_channel), NUM2UINT(baud), NUM2UINT(spi_flags)));
}
/*
This functions closes the SPI device identified by the handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*spi_open*].
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.

:call-seq:
 spi_close(Integer pi,Integer handle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#spi_close]
*/
VALUE pigpio_rbfn_spi_close(VALUE self, VALUE pi, VALUE handle){
  return INT2NUM( spi_close(NUM2INT(pi), NUM2UINT(handle)));
}
/*
This function reads count bytes of data from the SPI
device associated with the handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*spi_open*].
    buf: an array to receive the read data bytes.
  count: the number of bytes to read.
. .

Returns the number of bytes transferred if OK, otherwise
PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or PI_SPI_XFER_FAILED.

:call-seq:
 spi_read(Integer pi,Integer handle, Integer count) -> Integer (When onError)
 spi_read(Integer pi,Integer handle, Integer count) -> String buf (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#spi_read]
*/
VALUE pigpio_rbfn_spi_read(VALUE self, VALUE pi, VALUE handle, VALUE count){
  unsigned countc=NUM2UINT(count);
  VALUE buf=rb_str_new("",countc);//#<Encoding:ASCII-8BIT>;
  int ret=spi_read(NUM2INT(pi), NUM2UINT(handle), StringValuePtr(buf), countc);
  return (ret < 0)?INT2NUM(ret):rb_str_resize(buf,ret);
  //if(ret < 0){buf=Qnil;}
  //return rb_ary_new_from_args(2,buf,INT2NUM(ret));
}
/*
This function writes count bytes of data from buf to the SPI
device associated with the handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*spi_open*].
    buf: the data bytes to write.
. .

Returns the number of bytes transferred if OK, otherwise
PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or PI_SPI_XFER_FAILED.

:call-seq:
 spi_write(Integer pi,Integer handle, String buf) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#spi_write]
*/
VALUE pigpio_rbfn_spi_write(VALUE self, VALUE pi, VALUE handle, VALUE buf){
  int ret= spi_write(NUM2INT(pi), NUM2UINT(handle), StringValuePtr(buf), RSTRING_LEN(buf));
  RB_GC_GUARD(buf);
  return INT2NUM(ret);
}
/*
This function transfers count bytes of data from txBuf to the SPI
device associated with the handle.  Simultaneously count bytes of
data are read from the device and placed in rxBuf.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*spi_open*].
  txBuf: the data bytes to write.
  rxBuf: the received data bytes.
. .

Returns the number of bytes transferred if OK, otherwise
PI_BAD_HANDLE, PI_BAD_SPI_COUNT, or PI_SPI_XFER_FAILED.

:call-seq:
 spi_xfer(Integer pi,Integer handle, String txBuf) -> Integer (When onError)
 spi_xfer(Integer pi,Integer handle, String txBuf) -> String rxBuf (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#spi_xfer]
*/
VALUE pigpio_rbfn_spi_xfer(VALUE self, VALUE pi, VALUE handle, VALUE txBuf){
  unsigned countc=RSTRING_LEN(txBuf);
  VALUE rxBuf=rb_str_new("",countc);//#<Encoding:ASCII-8BIT>;
  int ret=spi_xfer(NUM2INT(pi), NUM2UINT(handle), StringValuePtr(txBuf), StringValuePtr(rxBuf), countc);
  RB_GC_GUARD(txBuf);
  return (ret < 0)?INT2NUM(ret):rb_str_resize(rxBuf,ret);
}
/*
This returns a handle for the device at address i2c_addr on bus i2c_bus.

. .
        pi: >=0 (as returned by [*pigpio_start*]).
   i2c_bus: >=0.
  i2c_addr: 0-0x7F.
 i2c_flags: 0.
. .

No flags are currently defined.  This parameter should be set to zero.

Physically buses 0 and 1 are available on the Pi.  Higher numbered buses
will be available if a kernel supported bus multiplexor is being used.

Returns a handle (>=0) if OK, otherwise PI_BAD_I2C_BUS, PI_BAD_I2C_ADDR,
PI_BAD_FLAGS, PI_NO_HANDLE, or PI_I2C_OPEN_FAILED.

For the SMBus commands the low level transactions are shown at the end
of the function description.  The following abbreviations are used.

. .
 S       (1 bit) : Start bit
 P       (1 bit) : Stop bit
 Rd/Wr   (1 bit) : Read/Write bit. Rd equals 1, Wr equals 0.
 A, NA   (1 bit) : Accept and not accept bit. 
 Addr    (7 bits): I2C 7 bit address.
 i2c_reg (8 bits): A byte which often selects a register.
 Data    (8 bits): A data byte.
 Count   (8 bits): A byte defining the length of a block operation.
 
 [..]: Data sent by the device.
. .

:call-seq:
 i2c_open(Integer pi,Integer i2c_bus, Integer i2c_addr, Integer i2c_flags) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_open]
*/
VALUE pigpio_rbfn_i2c_open(VALUE self, VALUE pi, VALUE i2c_bus, VALUE i2c_addr, VALUE i2c_flags){
  return INT2NUM( i2c_open(NUM2INT(pi), NUM2UINT(i2c_bus), NUM2UINT(i2c_addr), NUM2UINT(i2c_flags)));
}
/*
This closes the I2C device associated with the handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*i2c_open*].
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE.

:call-seq:
 i2c_close(Integer pi,Integer handle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_close]
*/
VALUE pigpio_rbfn_i2c_close(VALUE self, VALUE pi, VALUE handle){
  return INT2NUM( i2c_close(NUM2INT(pi), NUM2UINT(handle)));
}
/*
This sends a single bit (in the Rd/Wr bit) to the device associated
with handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*i2c_open*].
    bit: 0-1, the value to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Quick command. SMBus 2.0 5.5.1
. .
 S Addr bit [A] P
. .

:call-seq:
 i2c_write_quick(Integer pi,Integer handle, Integer handle, Integer bit) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_write_quick]
*/
VALUE pigpio_rbfn_i2c_write_quick(VALUE self, VALUE pi, VALUE handle, VALUE bit){
  return INT2NUM( i2c_write_quick(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(bit)));
}
/*
This sends a single byte to the device associated with handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*i2c_open*].
   bVal: 0-0xFF, the value to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Send byte. SMBus 2.0 5.5.2
. .
 S Addr Wr [A] bVal [A] P
. .

:call-seq:
 i2c_write_byte(Integer pi,Integer handle, Integer bVal) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_write_byte]
*/
VALUE pigpio_rbfn_i2c_write_byte(VALUE self, VALUE pi, VALUE handle, VALUE bVal){
  return INT2NUM( i2c_write_byte(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(bVal)));
}
/*
This reads a single byte from the device associated with handle.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*i2c_open*].
. .

Returns the byte read (>=0) if OK, otherwise PI_BAD_HANDLE,
or PI_I2C_READ_FAILED.

Receive byte. SMBus 2.0 5.5.3
. .
 S Addr Rd [A] [Data] NA P
. .

:call-seq:
 i2c_read_byte(Integer pi,Integer handle) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_read_byte]
*/
VALUE pigpio_rbfn_i2c_read_byte(VALUE self, VALUE pi, VALUE handle){
  return INT2NUM( i2c_read_byte(NUM2INT(pi), NUM2UINT(handle)));
}
/*
This writes a single byte to the specified register of the device
associated with handle.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to write.
    bVal: 0-0xFF, the value to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Write byte. SMBus 2.0 5.5.4
. .
 S Addr Wr [A] i2c_reg [A] bVal [A] P
. .

:call-seq:
 i2c_write_byte_data(Integer pi,Integer handle, Integer i2c_reg, Integer bVal) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_write_byte_data]
*/
VALUE pigpio_rbfn_i2c_write_byte_data(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg, VALUE bVal){
  return INT2NUM( i2c_write_byte_data(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg), NUM2UINT(bVal)));
}
/*
This writes a single 16 bit word to the specified register of the device
associated with handle.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to write.
    wVal: 0-0xFFFF, the value to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Write word. SMBus 2.0 5.5.4
. .
 S Addr Wr [A] i2c_reg [A] wval_Low [A] wVal_High [A] P
. .

:call-seq:
 i2c_write_word_data(Integer pi,Integer handle, Integer i2c_reg, Integer wVal) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_write_word_data]
*/
VALUE pigpio_rbfn_i2c_write_word_data(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg, VALUE wVal){
  return INT2NUM( i2c_write_word_data(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg), NUM2UINT(wVal)));
}
/*
This reads a single byte from the specified register of the device
associated with handle.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to read.
. .

Returns the byte read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

Read byte. SMBus 2.0 5.5.5
. .
 S Addr Wr [A] i2c_reg [A] S Addr Rd [A] [Data] NA P
. .

:call-seq:
 i2c_read_byte_data(Integer pi,Integer handle, Integer i2c_reg) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_read_byte_data]
*/
VALUE pigpio_rbfn_i2c_read_byte_data(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg){
  return INT2NUM( i2c_read_byte_data(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg)));
}
/*
This reads a single 16 bit word from the specified register of the device
associated with handle.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to read.
. .

Returns the word read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

Read word. SMBus 2.0 5.5.5
. .
 S Addr Wr [A] i2c_reg [A]
    S Addr Rd [A] [DataLow] A [DataHigh] NA P
. .

:call-seq:
 i2c_read_word_data(Integer pi,Integer handle, Integer i2c_reg) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_read_word_data]
*/
VALUE pigpio_rbfn_i2c_read_word_data(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg){
  return INT2NUM( i2c_read_word_data(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg)));
}
/*
This writes 16 bits of data to the specified register of the device
associated with handle and and reads 16 bits of data in return.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to write/read.
    wVal: 0-0xFFFF, the value to write.
. .

Returns the word read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

Process call. SMBus 2.0 5.5.6
. .
 S Addr Wr [A] i2c_reg [A] wVal_Low [A] wVal_High [A]
    S Addr Rd [A] [DataLow] A [DataHigh] NA P
. .

:call-seq:
 i2c_process_call(Integer pi,Integer handle, Integer i2c_reg, Integer wVal) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_process_call]
*/
VALUE pigpio_rbfn_i2c_process_call(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg, VALUE wVal){
  return INT2NUM( i2c_process_call(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg), NUM2UINT(wVal)));
}
/*
This reads a block of up to 32 bytes from the specified register of
the device associated with handle.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to read.
     buf: an binary string to receive the read data.
. .

The amount of returned data is set by the device.

Returns the number of bytes read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

Block read. SMBus 2.0 5.5.7
. .
 S Addr Wr [A] i2c_reg [A]
    S Addr Rd [A] [Count] A [buf0] A [buf1] A ... A [bufn] NA P
. .

:call-seq:
 i2c_read_block_data(Integer pi,Integer handle, Integer i2c_reg) -> Integer (When onError)
 i2c_read_block_data(Integer pi,Integer handle, Integer i2c_reg) -> String buf (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_read_block_data]
*/
VALUE pigpio_rbfn_i2c_read_block_data(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg){
  char buf[32];
  int ret=i2c_read_block_data(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg), buf);
  return (ret<0)?INT2NUM(ret):rb_str_new(buf,ret);
}
/*
This reads count bytes from the specified register of the device
associated with handle .  The count may be 1-32.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to read.
     buf: an array to receive the read data.
   count: 1-32, the number of bytes to read.
. .

Returns the number of bytes read (>0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

. .
 S Addr Wr [A] i2c_reg [A]
    S Addr Rd [A] [buf0] A [buf1] A ... A [bufn] NA P
. .

:call-seq:
 i2c_read_i2c_block_data(Integer pi,Integer handle, Integer i2c_reg,Integer buf) -> Integer (When onError)
 i2c_read_i2c_block_data(Integer pi,Integer handle, Integer i2c_reg,Integer buf) -> String  (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_read_i2c_block_data]
*/
VALUE pigpio_rbfn_i2c_read_i2c_block_data(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg, VALUE count){
  char buf[32];
  int ret=i2c_read_i2c_block_data(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg), buf, NUM2UINT(count));
  return (ret<0)?INT2NUM(ret):rb_str_new(buf,ret);
}
/*
This writes up to 32 bytes to the specified register of the device
associated with handle.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to write.
     buf: an binary string with the data to send.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

Block write. SMBus 2.0 5.5.7
. .
 S Addr Wr [A] i2c_reg [A] count [A] buf0 [A] buf1 [A] ...
    [A] bufn [A] P
. .

:call-seq:
 i2c_write_block_data(Integer pi,Integer handle, Integer i2c_reg,String buf) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_write_block_data]
*/
VALUE pigpio_rbfn_i2c_write_block_data(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg, VALUE buf){
  int ret=i2c_write_block_data(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg),StringValuePtr(buf), RSTRING_LEN(buf));
  RB_GC_GUARD(buf);
  return INT2NUM(ret);
}
/*
This writes 1 to 32 bytes to the specified register of the device
associated with handle.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to write.
     buf: the data to write.
   count: 1-32, the number of bytes to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

. .
 S Addr Wr [A] i2c_reg [A] buf0 [A] buf1 [A] ... [A] bufn [A] P
. .

:call-seq:
 i2c_write_i2c_block_data(Integer pi,Integer handle, Integer i2c_reg,String buf) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_write_i2c_block_data]
*/
VALUE pigpio_rbfn_i2c_write_i2c_block_data(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg, VALUE buf){
  int ret=i2c_write_i2c_block_data(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg),StringValuePtr(buf), RSTRING_LEN(buf));
  RB_GC_GUARD(buf);
  return INT2NUM(ret);
}
/*
This writes data bytes to the specified register of the device
associated with handle and reads a device specified number
of bytes of data in return.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
  handle: >=0, as returned by a call to [*i2c_open*].
 i2c_reg: 0-255, the register to write/read.
     buf: an binary string with the data to send and to receive the read data.
. .


Returns the number of bytes read (>=0) if OK, otherwise PI_BAD_HANDLE,
PI_BAD_PARAM, or PI_I2C_READ_FAILED.

The smbus 2.0 documentation states that a minimum of 1 byte may be
sent and a minimum of 1 byte may be received.  The total number of
bytes sent/received must be 32 or less.

Block write-block read. SMBus 2.0 5.5.8
. .
 S Addr Wr [A] i2c_reg [A] count [A] buf0 [A] ...
    S Addr Rd [A] [Count] A [Data] ... A P
. .

:call-seq:
 i2c_block_process_call(Integer pi,Integer handle, Integer i2c_reg,String buf) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_block_process_call]
*/
VALUE pigpio_rbfn_i2c_block_process_call(VALUE self, VALUE pi, VALUE handle, VALUE i2c_reg, VALUE buf){
  int ret=i2c_block_process_call(NUM2INT(pi), NUM2UINT(handle), NUM2UINT(i2c_reg),StringValuePtr(buf), RSTRING_LEN(buf));
  RB_GC_GUARD(buf);
  return INT2NUM(ret);
}
/*
This reads count bytes from the raw device into buf.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*i2c_open*].
    buf: an binary string to receive the read data bytes.
  count: >0, the number of bytes to read.
. .

Returns count (>0) if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_READ_FAILED.

. .
 S Addr Rd [A] [buf0] A [buf1] A ... A [bufn] NA P
. .

:call-seq:
 i2c_read_device(Integer pi,Integer handle, Integer count) -> Integer (When onError)
 i2c_read_device(Integer pi,Integer handle, Integer count) -> String buf (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_read_device]
*/
VALUE pigpio_rbfn_i2c_read_device(VALUE self, VALUE pi, VALUE handle, VALUE count){
  unsigned countc=NUM2UINT(count);
  VALUE buf=rb_str_new("",countc);//#<Encoding:ASCII-8BIT>;
  int ret=i2c_read_device(NUM2INT(pi), NUM2UINT(handle), StringValuePtr(buf), countc);
  return (ret<0)?INT2NUM(ret):rb_str_resize(buf,ret);
}
/*
This writes count bytes from buf to the raw device.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*i2c_open*].
    buf: an array containing the data bytes to write.
  count: >0, the number of bytes to write.
. .

Returns 0 if OK, otherwise PI_BAD_HANDLE, PI_BAD_PARAM, or
PI_I2C_WRITE_FAILED.

. .
 S Addr Wr [A] buf0 [A] buf1 [A] ... [A] bufn [A] P
. .

:call-seq:
 i2c_write_device(Integer pi,Integer handle, String buf) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_write_device]
*/
VALUE pigpio_rbfn_i2c_write_device(VALUE self, VALUE pi, VALUE handle, VALUE buf){
  int ret=i2c_write_device(NUM2INT(pi), NUM2UINT(handle), StringValuePtr(buf), RSTRING_LEN(buf));
  RB_GC_GUARD(buf);
  return INT2NUM(ret);
}
/*
This function executes a sequence of I2C operations.  The
operations to be performed are specified by the contents of inBuf
which contains the concatenated command codes and associated data.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
 handle: >=0, as returned by a call to [*i2cOpen*]
  inBuf: the concatenated I2C commands, see below
 outBuf: buffer to hold returned data
 outLen: size of output buffer
. .

Returns >= 0 if OK (the number of bytes read), otherwise
PI_BAD_HANDLE, PI_BAD_POINTER, PI_BAD_I2C_CMD, PI_BAD_I2C_RLEN.
PI_BAD_I2C_WLEN, or PI_BAD_I2C_SEG.

The following command codes are supported:

 Name    @ Cmd & Data @ Meaning
 End     @ 0          @ No more commands
 Escape  @ 1          @ Next P is two bytes
 On      @ 2          @ Switch combined flag on
 Off     @ 3          @ Switch combined flag off
 Address @ 4 P        @ Set I2C address to P
 Flags   @ 5 lsb msb  @ Set I2C flags to lsb + (msb << 8)
 Read    @ 6 P        @ Read P bytes of data
 Write   @ 7 P ...    @ Write P bytes of data

The address, read, and write commands take a parameter P.
Normally P is one byte (0-255).  If the command is preceded by
the Escape command then P is two bytes (0-65535, least significant
byte first).

The address defaults to that associated with the handle.
The flags default to 0.  The address and flags maintain their
previous value until updated.

The returned I2C data is stored in consecutive locations of outBuf.

...
 Set address 0x53, write 0x32, read 6 bytes
 Set address 0x1E, write 0x03, read 6 bytes
 Set address 0x68, write 0x1B, read 8 bytes
 End
 
 0x04 0x53   0x07 0x01 0x32   0x06 0x06
 0x04 0x1E   0x07 0x01 0x03   0x06 0x06
 0x04 0x68   0x07 0x01 0x1B   0x06 0x08
 0x00
...


:call-seq:
 i2c_zip(Integer pi,Integer handle, String inBuf, Integer outLen) -> Integer (When onError)
 i2c_zip(Integer pi,Integer handle, String inBuf, Integer outLen) -> String outBuf (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#i2c_zip]
*/
VALUE pigpio_rbfn_i2c_zip(VALUE self, VALUE pi, VALUE handle, VALUE inBuf, VALUE outLen_v){
  unsigned inLen=RSTRING_LEN(inBuf);
  unsigned outLen=NUM2UINT(outLen_v);
  VALUE outBuf=rb_str_new("",outLen);//#<Encoding:ASCII-8BIT>;
  int ret=i2c_zip(NUM2INT(pi), NUM2UINT(handle), StringValuePtr(inBuf), inLen, StringValuePtr(outBuf), outLen);
  RB_GC_GUARD(inBuf);
  return (ret < 0)?INT2NUM(ret):rb_str_resize(outBuf,ret);
  //if(ret < 0){outBuf=Qnil;}
  //return rb_ary_new_from_args(2,outBuf,INT2NUM(ret));
}
/*
This function selects a pair of GPIO for bit banging I2C at a
specified baud rate.

Bit banging I2C allows for certain operations which are not possible
with the standard I2C driver.

* baud rates as low as 50 
* repeated starts 
* clock stretching 
* I2C on any pair of spare GPIO

. .
   pi: >=0 (as returned by [*pigpio_start*]).
  SDA: 0-31
  SCL: 0-31
 baud: 50-500000
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_I2C_BAUD, or
PI_GPIO_IN_USE.

NOTE:

The GPIO used for SDA and SCL must have pull-ups to 3V3 connected.  As
a guide the hardware pull-ups on pins 3 and 5 are 1k8 in value.

:call-seq:
 bb_i2c_open(Integer pi,Integer SDA, Integer SCL, Integer baud) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_i2c_open]
*/
VALUE pigpio_rbfn_bb_i2c_open(VALUE self, VALUE pi, VALUE SDA, VALUE SCL, VALUE baud){
  return INT2NUM( bb_i2c_open(NUM2INT(pi), NUM2UINT(SDA), NUM2UINT(SCL), NUM2UINT(baud)));
}
/*
This function stops bit banging I2C on a pair of GPIO previously
opened with [*bb_i2c_open*].

. .
  pi: >=0 (as returned by [*pigpio_start*]).
 SDA: 0-31, the SDA GPIO used in a prior call to [*bb_i2c_open*]
. .

Returns 0 if OK, otherwise PI_BAD_USER_GPIO, or PI_NOT_I2C_GPIO.

:call-seq:
 bb_i2c_close(Integer pi,Integer SDA) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_i2c_close]
*/
VALUE pigpio_rbfn_bb_i2c_close(VALUE self, VALUE pi, VALUE SDA){
  return INT2NUM( bb_i2c_close(NUM2INT(pi), NUM2UINT(SDA)));
}
/*
This function executes a sequence of bit banged I2C operations.  The
operations to be performed are specified by the contents of inBuf
which contains the concatenated command codes and associated data.

. .
     pi: >=0 (as returned by [*pigpio_start*]).
    SDA: 0-31 (as used in a prior call to [*bb_i2c_open*])
  inBuf: the concatenated I2C commands, see below
 outBuf: buffer to hold returned data
 outLen: size of output buffer
. .

Returns >= 0 if OK (the number of bytes read), otherwise
PI_BAD_USER_GPIO, PI_NOT_I2C_GPIO, PI_BAD_POINTER,
PI_BAD_I2C_CMD, PI_BAD_I2C_RLEN, PI_BAD_I2C_WLEN,
PI_I2C_READ_FAILED, or PI_I2C_WRITE_FAILED.

The following command codes are supported:

 Name    @ Cmd & Data   @ Meaning
 End     @ 0            @ No more commands
 Escape  @ 1            @ Next P is two bytes
 Start   @ 2            @ Start condition
 Stop    @ 3            @ Stop condition
 Address @ 4 P          @ Set I2C address to P
 Flags   @ 5 lsb msb    @ Set I2C flags to lsb + (msb << 8)
 Read    @ 6 P          @ Read P bytes of data
 Write   @ 7 P ...      @ Write P bytes of data

The address, read, and write commands take a parameter P.
Normally P is one byte (0-255).  If the command is preceded by
the Escape command then P is two bytes (0-65535, least significant
byte first).

The address and flags default to 0.  The address and flags maintain
their previous value until updated.

No flags are currently defined.

The returned I2C data is stored in consecutive locations of outBuf.

...
 Set address 0x53
 start, write 0x32, (re)start, read 6 bytes, stop
 Set address 0x1E
 start, write 0x03, (re)start, read 6 bytes, stop
 Set address 0x68
 start, write 0x1B, (re)start, read 8 bytes, stop
 End
 
 0x04 0x53
 0x02 0x07 0x01 0x32   0x02 0x06 0x06 0x03
 
 0x04 0x1E 
 0x02 0x07 0x01 0x03   0x02 0x06 0x06 0x03
 
 0x04 0x68
 0x02 0x07 0x01 0x1B   0x02 0x06 0x08 0x03
 
 0x00
...

:call-seq:
 bb_i2c_zip(Integer pi,Integer SDA, String inBuf, Integer outLen) -> Integer (When onError)
 bb_i2c_zip(Integer pi,Integer SDA, String inBuf, Integer outLen) -> String outBuf (When Success)

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bb_i2c_zip]
*/
VALUE pigpio_rbfn_bb_i2c_zip(VALUE self, VALUE pi, VALUE SDA, VALUE inBuf, VALUE outLen_v){
  unsigned outLen=NUM2UINT(outLen_v);
  VALUE outBuf=rb_str_new("",outLen);//#<Encoding:ASCII-8BIT>;
  int ret=bb_i2c_zip(NUM2INT(pi), NUM2UINT(SDA), StringValuePtr(inBuf), RSTRING_LEN(inBuf), StringValuePtr(outBuf), outLen);
  RB_GC_GUARD(inBuf);
  return (ret < 0)?INT2NUM(ret):rb_str_resize(outBuf,ret);
  //if(ret < 0){outBuf=Qnil;}
  //return rb_ary_new_from_args(2,outBuf,INT2NUM(ret));
}


/*
This function returns the pad drive strength in mA.

. .
  pi: >=0 (as returned by [*pigpio_start*]).
 pad: 0-2, the pad to get.
. .

Returns the pad drive strength if OK, otherwise PI_BAD_PAD.

 Pad @ GPIO
 0   @ 0-27
 1   @ 28-45
 2   @ 46-53

...
 strength = get_pad_strength(pi, 0); //  get pad 0 strength
...

:call-seq:
 get_pad_strength(Integer pi,Integer pad) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#get_pad_strength]
*/
VALUE pigpio_rbfn_get_pad_strength(VALUE self, VALUE pi, VALUE pad){
  return INT2NUM( get_pad_strength(NUM2INT(pi), NUM2UINT(pad)));
}
/*
This function sets the pad drive strength in mA.

. .
          pi: >=0 (as returned by [*pigpio_start*]).
         pad: 0-2, the pad to set.
 padStrength: 1-16 mA.
. .

Returns 0 if OK, otherwise PI_BAD_PAD, or PI_BAD_STRENGTH.

 Pad @ GPIO
 0   @ 0-27
 1   @ 28-45
 2   @ 46-53

...
 set_pad_strength(pi, 0, 10); // set pad 0 strength to 10 mA
...

:call-seq:
 set_pad_strength(Integer pi,Integer pad, Integer padStrength) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#set_pad_strength]
*/
VALUE pigpio_rbfn_set_pad_strength(VALUE self, VALUE pi, VALUE pad, VALUE padStrength){
  return INT2NUM( set_pad_strength(NUM2INT(pi), NUM2UINT(pad), NUM2UINT(padStrength)));
}

/*
This function provides a low-level interface to the
SPI/I2C Slave peripheral.  This peripheral allows the
Pi to act as a slave device on an I2C or SPI bus.

I can't get SPI to work properly.  I tried with a
control word of 0x303 and swapped MISO and MOSI.

The function sets the BSC mode, writes any data in
the transmit buffer to the BSC transmit FIFO, and
copies any data in the BSC receive FIFO to the
receive buffer.

. .
      pi: >=0 (as returned by [*pigpio_start*]).
 bscxfer: a structure defining the transfer.
 
 typedef struct
 {
    uint32_t control;          // Write
    int rxCnt;                 // Read only
    char rxBuf[BSC_FIFO_SIZE]; // Read only
    int txCnt;                 // Write
    char txBuf[BSC_FIFO_SIZE]; // Write
 } bsc_xfer_t;
. .

To start a transfer set control (see below) and copy the bytes to
be sent (if any) to txBuf and set the byte count in txCnt.

Upon return rxCnt will be set to the number of received bytes placed
in rxBuf.

The returned function value is the status of the transfer (see below).

If there was an error the status will be less than zero
(and will contain the error code).

The most significant word of the returned status contains the number
of bytes actually copied from txBuf to the BSC transmit FIFO (may be
less than requested if the FIFO already contained untransmitted data).

Note that the control word sets the BSC mode.  The BSC will stay in
that mode until a different control word is sent.

The BSC peripheral uses GPIO 18 (SDA) and 19 (SCL) in I2C mode
and GPIO 18 (MOSI), 19 (SCLK), 20 (MISO), and 21 (CE) in SPI mode.  You
need to swap MISO/MOSI between master and slave.

When a zero control word is received GPIO 18-21 will be reset
to INPUT mode.

control consists of the following bits.

. .
 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
  a  a  a  a  a  a  a  -  - IT HC TF IR RE TE BK EC ES PL PH I2 SP EN
. .

Bits 0-13 are copied unchanged to the BSC CR register.  See
pages 163-165 of the Broadcom peripherals document for full
details.

 aaaaaaa @ defines the I2C slave address (only relevant in I2C mode)
 IT      @ invert transmit status flags
 HC      @ enable host control
 TF      @ enable test FIFO
 IR      @ invert receive status flags
 RE      @ enable receive
 TE      @ enable transmit
 BK      @ abort operation and clear FIFOs
 EC      @ send control register as first I2C byte
 ES      @ send status register as first I2C byte
 PL      @ set SPI polarity high
 PH      @ set SPI phase high
 I2      @ enable I2C mode
 SP      @ enable SPI mode
 EN      @ enable BSC peripheral

The returned status has the following format

. .
 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
  S  S  S  S  S  R  R  R  R  R  T  T  T  T  T RB TE RF TF RE TB
. .

Bits 0-15 are copied unchanged from the BSC FR register.  See
pages 165-166 of the Broadcom peripherals document for full
details.

 SSSSS @ number of bytes successfully copied to transmit FIFO
 RRRRR @ number of bytes in receieve FIFO
 TTTTT @ number of bytes in transmit FIFO
 RB    @ receive busy
 TE    @ transmit FIFO empty
 RF    @ receive FIFO full
 TF    @ transmit FIFO full
 RE    @ receive FIFO empty
 TB    @ transmit busy

:call-seq:
 bsc_xfer(Integer pi, PIGPIO::BscXfer bscxfer) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bsc_xfer]
*/
VALUE pigpio_rbfn_bsc_xfer(VALUE self, VALUE pi, VALUE bscxfer){
  bsc_xfer_t *st=TypedData_Get_Struct2(bscxfer,bsc_xfer_t,&bsc_xfer_data_type);
  int ret=bsc_xfer(NUM2INT(pi), st);
  return INT2NUM(ret);
}
/*
This function allows the Pi to act as a slave I2C device.

The data bytes (if any) are written to the BSC transmit
FIFO and the bytes in the BSC receive FIFO are returned.

. .
       pi: >=0 (as returned by [*pigpio_start*]).
 i2c_addr: 0-0x7F.
  bscxfer: a structure defining the transfer.

 typedef struct
 {
    uint32_t control;          // N/A
    int rxCnt;                 // Read only
    char rxBuf[BSC_FIFO_SIZE]; // Read only
    int txCnt;                 // Write
    char txBuf[BSC_FIFO_SIZE]; // Write
 } bsc_xfer_t;
. .

txCnt is set to the number of bytes to be transmitted, possibly
zero. The data itself should be copied to txBuf.

Any received data will be written to rxBuf with rxCnt set.

See [*bsc_xfer*] for details of the returned status value.

If there was an error the status will be less than zero
(and will contain the error code).

Note that an i2c_address of 0 may be used to close
the BSC device and reassign the used GPIO (18/19)
as inputs.

:call-seq:
 bsc_i2c(Integer pi,Integer i2c_addr, PIGPIO::BscXfer bscxfer) -> Integer

See also: {pigpio site}[http://abyz.me.uk/rpi/pigpio/pdif2.html#bsc_i2c]
*/
VALUE pigpio_rbfn_bsc_i2c(VALUE self, VALUE pi, VALUE i2c_addr, VALUE bscxfer){
  bsc_xfer_t *st=TypedData_Get_Struct2(bscxfer,bsc_xfer_t,&bsc_xfer_data_type);
  int ret=bsc_i2c(NUM2INT(pi), NUM2INT(i2c_addr), st);
  return INT2NUM(ret);
}

/*
This class has some constances for pigpio library.
*/
void Init_pigpio(void){
  VALUE cPulse,cBscXfer,cNativeQueue;
/*
This class has some constances for pigpio library.
*/
  VALUE cPigpio = rb_define_class("Pigpio", rb_cObject);
  /*
  This module is a ruby binding to pigpio library.
  */
  VALUE cAPI = rb_define_module_under(cPigpio, "IF");
    rb_define_singleton_method(cAPI, "pigpio_start",      pigpio_rbfn_pigpio_start,       -1);
    rb_define_singleton_method(cAPI, "pigpio_stop",       pigpio_rbfn_pigpio_stop,        1);
    rb_define_singleton_method(cAPI, "set_mode",          pigpio_rbfn_set_mode,           3);
    rb_define_singleton_method(cAPI, "get_mode",          pigpio_rbfn_get_mode,           2);
    rb_define_singleton_method(cAPI, "set_pull_up_down",  pigpio_rbfn_set_pull_up_down,   3);
    rb_define_singleton_method(cAPI, "gpio_read",         pigpio_rbfn_gpio_read,          2);
    rb_define_singleton_method(cAPI, "gpio_write",        pigpio_rbfn_gpio_write,         3);
    rb_define_singleton_method(cAPI, "time_time",         pigpio_rbfn_time_time,          0);
    rb_define_singleton_method(cAPI, "pigpio_error",      pigpio_rbfn_pigpio_error,       1);
    rb_define_singleton_method(cAPI, "pigpiod_if_version",pigpio_rbfn_pigpiod_if_version, 0);
    rb_define_singleton_method(cAPI, "set_watchdog",         pigpio_rbfn_set_watchdog,          3);
    rb_define_singleton_method(cAPI, "set_glitch_filter",    pigpio_rbfn_set_glitch_filter,     3);
    rb_define_singleton_method(cAPI, "set_noise_filter",     pigpio_rbfn_set_noise_filter,      4);
    rb_define_singleton_method(cAPI, "get_current_tick",     pigpio_rbfn_get_current_tick,      1);
    rb_define_singleton_method(cAPI, "get_hardware_revision",pigpio_rbfn_get_hardware_revision, 1);
    rb_define_singleton_method(cAPI, "get_pigpio_version",   pigpio_rbfn_get_pigpio_version,    1);
    rb_define_singleton_method(cAPI, "wave_clear",           pigpio_rbfn_wave_clear,            1);
    rb_define_singleton_method(cAPI, "wave_add_new",         pigpio_rbfn_wave_add_new,          1);
    rb_define_singleton_method(cAPI, "wave_create",          pigpio_rbfn_wave_create,           1);
    rb_define_singleton_method(cAPI, "wave_delete",          pigpio_rbfn_wave_delete,           2);
    rb_define_singleton_method(cAPI, "wave_send_once",       pigpio_rbfn_wave_send_once,        2);
    rb_define_singleton_method(cAPI, "wave_send_repeat",     pigpio_rbfn_wave_send_repeat,      2);
    rb_define_singleton_method(cAPI, "wave_send_using_mode", pigpio_rbfn_wave_send_using_mode,  3);
    rb_define_singleton_method(cAPI, "wave_tx_at",           pigpio_rbfn_wave_tx_at,            1);
    rb_define_singleton_method(cAPI, "wave_tx_busy",         pigpio_rbfn_wave_tx_busy,          1);
    rb_define_singleton_method(cAPI, "wave_tx_stop",         pigpio_rbfn_wave_tx_stop,          1);
    rb_define_singleton_method(cAPI, "wave_get_micros",      pigpio_rbfn_wave_get_micros,       1);
    rb_define_singleton_method(cAPI, "wave_get_high_micros", pigpio_rbfn_wave_get_high_micros,  1);
    rb_define_singleton_method(cAPI, "wave_get_max_micros",  pigpio_rbfn_wave_get_max_micros,   1);
    rb_define_singleton_method(cAPI, "wave_get_pulses",      pigpio_rbfn_wave_get_pulses,       1);
    rb_define_singleton_method(cAPI, "wave_get_high_pulses", pigpio_rbfn_wave_get_high_pulses,  1);
    rb_define_singleton_method(cAPI, "wave_get_max_pulses",  pigpio_rbfn_wave_get_max_pulses,   1);
    rb_define_singleton_method(cAPI, "wave_get_cbs",         pigpio_rbfn_wave_get_cbs,          1);
    rb_define_singleton_method(cAPI, "wave_get_high_cbs",    pigpio_rbfn_wave_get_high_cbs,     1);
    rb_define_singleton_method(cAPI, "wave_get_max_cbs",     pigpio_rbfn_wave_get_max_cbs,      1);
    rb_define_singleton_method(cAPI, "gpio_trigger",         pigpio_rbfn_gpio_trigger,          4);
    rb_define_singleton_method(cAPI, "set_PWM_dutycycle",    pigpio_rbfn_set_PWM_dutycycle,     3);
    rb_define_singleton_method(cAPI, "get_PWM_dutycycle",    pigpio_rbfn_get_PWM_dutycycle,     2);
    rb_define_singleton_method(cAPI, "set_PWM_range",        pigpio_rbfn_set_PWM_range,         3);
    rb_define_singleton_method(cAPI, "get_PWM_range",        pigpio_rbfn_get_PWM_range,         2);
    rb_define_singleton_method(cAPI, "get_PWM_real_range",   pigpio_rbfn_get_PWM_real_range,    2);
    rb_define_singleton_method(cAPI, "set_PWM_frequency",    pigpio_rbfn_set_PWM_frequency,     3);
    rb_define_singleton_method(cAPI, "get_PWM_frequency",    pigpio_rbfn_get_PWM_frequency,     2);
    rb_define_singleton_method(cAPI, "set_servo_pulsewidth", pigpio_rbfn_set_servo_pulsewidth,  3);
    rb_define_singleton_method(cAPI, "get_servo_pulsewidth", pigpio_rbfn_get_servo_pulsewidth,  2);
    rb_define_singleton_method(cAPI, "notify_open",          pigpio_rbfn_notify_open,           1);
    rb_define_singleton_method(cAPI, "notify_begin",         pigpio_rbfn_notify_begin,          3);
    rb_define_singleton_method(cAPI, "notify_pause",         pigpio_rbfn_notify_pause,          2);
    rb_define_singleton_method(cAPI, "notify_close",         pigpio_rbfn_notify_close,          2);
    rb_define_singleton_method(cAPI, "read_bank_1",          pigpio_rbfn_read_bank_1,           1);
    rb_define_singleton_method(cAPI, "read_bank_2",          pigpio_rbfn_read_bank_2,           1);
    rb_define_singleton_method(cAPI, "clear_bank_1",         pigpio_rbfn_clear_bank_1,          2);
    rb_define_singleton_method(cAPI, "clear_bank_2",         pigpio_rbfn_clear_bank_2,          2);
    rb_define_singleton_method(cAPI, "set_bank_1",           pigpio_rbfn_set_bank_1,            2);
    rb_define_singleton_method(cAPI, "set_bank_2",           pigpio_rbfn_set_bank_2,            2);
    rb_define_singleton_method(cAPI, "hardware_clock",       pigpio_rbfn_hardware_clock,        3);
    rb_define_singleton_method(cAPI, "hardware_PWM",         pigpio_rbfn_hardware_PWM,          4);
    rb_define_singleton_method(cAPI, "wave_add_generic",        pigpio_rbfn_wave_add_generic,         2);
    rb_define_singleton_method(cAPI, "wave_add_serial",         pigpio_rbfn_wave_add_serial,          7);
    rb_define_singleton_method(cAPI, "wave_chain",              pigpio_rbfn_wave_chain,               2);
    rb_define_singleton_method(cAPI, "callback",                pigpio_rbfn_callback,                 -1);
    rb_define_singleton_method(cAPI, "callback_cancel",         pigpio_rbfn_callback_cancel,          1);
    rb_define_singleton_method(cAPI, "wait_for_edge",           pigpio_rbfn_wait_for_edge,            4);
    rb_define_singleton_method(cAPI, "event_callback",          pigpio_rbfn_event_callback,           -1);
    rb_define_singleton_method(cAPI, "event_callback_cancel",   pigpio_rbfn_event_callback_cancel,    1);
    rb_define_singleton_method(cAPI, "wait_for_event",          pigpio_rbfn_wait_for_event,           3);
    rb_define_singleton_method(cAPI, "event_trigger",           pigpio_rbfn_event_trigger,            2);
    rb_define_singleton_method(cAPI, "serial_open",             pigpio_rbfn_serial_open,              4);
    rb_define_singleton_method(cAPI, "serial_close",            pigpio_rbfn_serial_close,             2);
    rb_define_singleton_method(cAPI, "serial_write_byte",       pigpio_rbfn_serial_write_byte,        3);
    rb_define_singleton_method(cAPI, "serial_read_byte",        pigpio_rbfn_serial_read_byte,         2);
    rb_define_singleton_method(cAPI, "serial_write",            pigpio_rbfn_serial_write,             3);
    rb_define_singleton_method(cAPI, "serial_read",             pigpio_rbfn_serial_read,              3);
    rb_define_singleton_method(cAPI, "serial_data_available",   pigpio_rbfn_serial_data_available,    2);
    rb_define_singleton_method(cAPI, "bb_serial_read_open",     pigpio_rbfn_bb_serial_read_open,      4);
    rb_define_singleton_method(cAPI, "bb_serial_read",          pigpio_rbfn_bb_serial_read,           3);
    rb_define_singleton_method(cAPI, "bb_serial_read_close",    pigpio_rbfn_bb_serial_read_close,     2);
    rb_define_singleton_method(cAPI, "bb_serial_invert",        pigpio_rbfn_bb_serial_invert,         3);
    rb_define_singleton_method(cAPI, "bb_spi_open",             pigpio_rbfn_bb_spi_open,              7);
    rb_define_singleton_method(cAPI, "bb_spi_close",            pigpio_rbfn_bb_spi_close,             2);
    rb_define_singleton_method(cAPI, "bb_spi_xfer",             pigpio_rbfn_bb_spi_xfer,              3);
    rb_define_singleton_method(cAPI, "spi_open",                pigpio_rbfn_spi_open,                 4);
    rb_define_singleton_method(cAPI, "spi_close",               pigpio_rbfn_spi_close,                2);
    rb_define_singleton_method(cAPI, "spi_read",                pigpio_rbfn_spi_read,                 3);
    rb_define_singleton_method(cAPI, "spi_write",               pigpio_rbfn_spi_write,                3);
    rb_define_singleton_method(cAPI, "spi_xfer",                pigpio_rbfn_spi_xfer,                 3);
    rb_define_singleton_method(cAPI, "i2c_open",                pigpio_rbfn_i2c_open,                 4);
    rb_define_singleton_method(cAPI, "i2c_close",               pigpio_rbfn_i2c_close,                2);
    rb_define_singleton_method(cAPI, "i2c_write_quick",         pigpio_rbfn_i2c_write_quick,          3);
    rb_define_singleton_method(cAPI, "i2c_write_byte",          pigpio_rbfn_i2c_write_byte,           3);
    rb_define_singleton_method(cAPI, "i2c_read_byte",           pigpio_rbfn_i2c_read_byte,            2);
    rb_define_singleton_method(cAPI, "i2c_write_byte_data",     pigpio_rbfn_i2c_write_byte_data,      4);
    rb_define_singleton_method(cAPI, "i2c_write_word_data",     pigpio_rbfn_i2c_write_word_data,      4);
    rb_define_singleton_method(cAPI, "i2c_read_byte_data",      pigpio_rbfn_i2c_read_byte_data,       3);
    rb_define_singleton_method(cAPI, "i2c_read_word_data",      pigpio_rbfn_i2c_read_word_data,       3);
    rb_define_singleton_method(cAPI, "i2c_process_call",        pigpio_rbfn_i2c_process_call,         4);
    rb_define_singleton_method(cAPI, "i2c_read_block_data",     pigpio_rbfn_i2c_read_block_data,      3);
    rb_define_singleton_method(cAPI, "i2c_read_i2c_block_data", pigpio_rbfn_i2c_read_i2c_block_data,  4);
    rb_define_singleton_method(cAPI, "i2c_write_block_data",    pigpio_rbfn_i2c_write_block_data,     4);
    rb_define_singleton_method(cAPI, "i2c_write_i2c_block_data",pigpio_rbfn_i2c_write_i2c_block_data, 4);
    rb_define_singleton_method(cAPI, "i2c_block_process_call",  pigpio_rbfn_i2c_block_process_call,   4);
    rb_define_singleton_method(cAPI, "i2c_read_device",         pigpio_rbfn_i2c_read_device,          3);
    rb_define_singleton_method(cAPI, "i2c_write_device",        pigpio_rbfn_i2c_write_device,         3);
    rb_define_singleton_method(cAPI, "i2c_zip",                 pigpio_rbfn_i2c_zip,                  4);
    rb_define_singleton_method(cAPI, "bb_i2c_open",             pigpio_rbfn_bb_i2c_open,              4);
    rb_define_singleton_method(cAPI, "bb_i2c_close",            pigpio_rbfn_bb_i2c_close,             2);
    rb_define_singleton_method(cAPI, "bb_i2c_zip",              pigpio_rbfn_bb_i2c_zip,               4);
    rb_define_singleton_method(cAPI, "store_script",            pigpio_rbfn_store_script,             2);
    rb_define_singleton_method(cAPI, "run_script",              pigpio_rbfn_run_script,               3);
    rb_define_singleton_method(cAPI, "script_status",           pigpio_rbfn_script_status,            2);
    rb_define_singleton_method(cAPI, "stop_script",             pigpio_rbfn_stop_script,              2);
    rb_define_singleton_method(cAPI, "delete_script",           pigpio_rbfn_delete_script,            2);
    rb_define_singleton_method(cAPI, "bsc_xfer",                pigpio_rbfn_bsc_xfer,                 2);
    rb_define_singleton_method(cAPI, "bsc_i2c",                 pigpio_rbfn_bsc_i2c,                  3);
    rb_define_singleton_method(cAPI, "get_pad_strength",        pigpio_rbfn_get_pad_strength,         2);
    rb_define_singleton_method(cAPI, "set_pad_strength",        pigpio_rbfn_set_pad_strength,         3);

  /*
  This class wrap gpioPulse_t.
  */
  cPulse = rb_define_class_under(cPigpio,"Pulse", rb_cData);
    rb_define_singleton_method(cPulse, "make", pigpio_rbst_gpioPulse_make, 3);

  /*
  This class wrap bsc_xfer_t.
  */
  cBscXfer = rb_define_class_under(cPigpio,"BscXfer", rb_cData);
    rb_define_singleton_method(cBscXfer, "make", pigpio_rbst_bsc_xfer_make, 0);
    rb_define_method(cBscXfer, "control=", pigpio_rbst_bsc_xfer_w_control, 1);
    rb_define_method(cBscXfer, "txBuf=", pigpio_rbst_bsc_xfer_w_txBuf, 1);
    rb_define_method(cBscXfer, "rxBuf", pigpio_rbst_bsc_xfer_r_rxBuf, 0);

  /*
  The class of native queue.
  */
  cNativeQueue = rb_define_class_under(cAPI,"NativeQueue", rb_cData);
    rb_gc_register_address(&cNativeQueue);

  /*
  The class of callback.
  */
  cCallbackID = rb_define_class_under(cPigpio,"Callback", rb_cData);
    rb_define_method(cCallbackID, "id", pigpio_rbst_callback_id_r_id, 0);
    rb_define_method(cCallbackID, "cancel", pigpio_rbst_callback_id_cancel, 0);
    rb_gc_register_address(&cCallbackID);

  /*
  The class of callback error.
  */
  cCallbackError = rb_define_class_under(cPigpio,"CallbackError", rb_eException);
    rb_gc_register_address(&cCallbackError);
}
