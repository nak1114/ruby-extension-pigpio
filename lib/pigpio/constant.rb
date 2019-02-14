module Pigpio::Constant
  #/* gpio: 0-53 */

  PI_MIN_GPIO       =0
  PI_MAX_GPIO      =53

  #/* user_gpio: 0-31 */

  PI_MAX_USER_GPIO =31

  #/* level: 0-1 */

  PI_OFF   =0
  PI_ON    =1

  PI_CLEAR =0
  PI_SET   =1

  PI_LOW   =0
  PI_HIGH  =1

  #/* level: only reported for GPIO time-out, see gpioSetWatchdog */

  PI_TIMEOUT =2

  #/* mode: 0-7 */

  PI_INPUT  =0
  PI_OUTPUT =1
  PI_ALT0   =4
  PI_ALT1   =5
  PI_ALT2   =6
  PI_ALT3   =7
  PI_ALT4   =3
  PI_ALT5   =2

  #/* pud: 0-2 */

  PI_PUD_OFF  =0
  PI_PUD_DOWN =1
  PI_PUD_UP   =2

  #/* dutycycle: 0-range */

  PI_DEFAULT_DUTYCYCLE_RANGE   =255

  #/* range: 25-40000 */

  PI_MIN_DUTYCYCLE_RANGE        =25
  PI_MAX_DUTYCYCLE_RANGE     =40000

  #/* pulsewidth: 0, 500-2500 */

  PI_SERVO_OFF =0
  PI_MIN_SERVO_PULSEWIDTH =500
  PI_MAX_SERVO_PULSEWIDTH =2500

  #/* hardware PWM */

  PI_HW_PWM_MIN_FREQ =1
  PI_HW_PWM_MAX_FREQ =125000000
  PI_HW_PWM_RANGE =1000000

  #/* hardware clock */

  PI_HW_CLK_MIN_FREQ =4689
  PI_HW_CLK_MAX_FREQ =250000000

  PI_NOTIFY_SLOTS  =32

  PI_NTFY_FLAGS_EVENT    =(1 <<7)
  PI_NTFY_FLAGS_ALIVE    =(1 <<6)
  PI_NTFY_FLAGS_WDOG     =(1 <<5)
#  PI_NTFY_FLAGS_BIT(x) =(((x)<<0)&31)

  PI_WAVE_BLOCKS     =4
  PI_WAVE_MAX_PULSES =(PI_WAVE_BLOCKS * 3000)
  PI_WAVE_MAX_CHARS  =(PI_WAVE_BLOCKS *  300)

  PI_BB_I2C_MIN_BAUD     =50
  PI_BB_I2C_MAX_BAUD =500000

  PI_BB_SPI_MIN_BAUD     =50
  PI_BB_SPI_MAX_BAUD =250000

  PI_BB_SER_MIN_BAUD     =50
  PI_BB_SER_MAX_BAUD =250000

  PI_BB_SER_NORMAL =0
  PI_BB_SER_INVERT =1

  PI_WAVE_MIN_BAUD      =50
  PI_WAVE_MAX_BAUD =1000000

  PI_SPI_MIN_BAUD     =32000
  PI_SPI_MAX_BAUD =125000000

  PI_MIN_WAVE_DATABITS =1
  PI_MAX_WAVE_DATABITS =32

  PI_MIN_WAVE_HALFSTOPBITS =2
  PI_MAX_WAVE_HALFSTOPBITS =8

  PI_WAVE_MAX_MICROS =(30 * 60 * 1000000) #/* half an hour */

  PI_MAX_WAVES =250

  PI_MAX_WAVE_CYCLES =65535
  PI_MAX_WAVE_DELAY  =65535

  PI_WAVE_COUNT_PAGES =10

  #/* wave tx mode */

  PI_WAVE_MODE_ONE_SHOT      =0
  PI_WAVE_MODE_REPEAT        =1
  PI_WAVE_MODE_ONE_SHOT_SYNC =2
  PI_WAVE_MODE_REPEAT_SYNC   =3

  #/* special wave at return values */

  PI_WAVE_NOT_FOUND  =9998 #/* Transmitted wave not found. */
  PI_NO_TX_WAVE      =9999 #/* No wave being transmitted. */

  #/* Files, I2C, SPI, SER */

  PI_FILE_SLOTS =16
  PI_I2C_SLOTS  =64
  PI_SPI_SLOTS  =32
  PI_SER_SLOTS  =16

  PI_MAX_I2C_ADDR =0x7F

  PI_NUM_AUX_SPI_CHANNEL =3
  PI_NUM_STD_SPI_CHANNEL =2

  PI_MAX_I2C_DEVICE_COUNT =(1<<16)
  PI_MAX_SPI_DEVICE_COUNT =(1<<16)

  #/* max pi_i2c_msg_t per transaction */

  PI_I2C_RDRW_IOCTL_MAX_MSGS =42

  #/* flags for i2cTransaction, pi_i2c_msg_t */

  PI_I2C_M_WR           =0x0000 #/* write data */
  PI_I2C_M_RD           =0x0001 #/* read data */
  PI_I2C_M_TEN          =0x0010 #/* ten bit chip address */
  PI_I2C_M_RECV_LEN     =0x0400 #/* length will be first received byte */
  PI_I2C_M_NO_RD_ACK    =0x0800 #/* if I2C_FUNC_PROTOCOL_MANGLING */
  PI_I2C_M_IGNORE_NAK   =0x1000 #/* if I2C_FUNC_PROTOCOL_MANGLING */
  PI_I2C_M_REV_DIR_ADDR =0x2000 #/* if I2C_FUNC_PROTOCOL_MANGLING */
  PI_I2C_M_NOSTART      =0x4000 #/* if I2C_FUNC_PROTOCOL_MANGLING */

  #/* bbI2CZip and i2cZip commands */

  PI_I2C_END          =0
  PI_I2C_ESC          =1
  PI_I2C_START        =2
  PI_I2C_COMBINED_ON  =2
  PI_I2C_STOP         =3
  PI_I2C_COMBINED_OFF =3
  PI_I2C_ADDR         =4
  PI_I2C_FLAGS        =5
  PI_I2C_READ         =6
  PI_I2C_WRITE        =7

  #/* SPI */

#  PI_SPI_FLAGS_BITLEN(x) =((x&63)<<16)
#  PI_SPI_FLAGS_RX_LSB(x)  =((x&1)<<15)
#  PI_SPI_FLAGS_TX_LSB(x)  =((x&1)<<14)
#  PI_SPI_FLAGS_3WREN(x)  =((x&15)<<10)
#  PI_SPI_FLAGS_3WIRE(x)   =((x&1)<<9)
#  PI_SPI_FLAGS_AUX_SPI(x) =((x&1)<<8)
#  PI_SPI_FLAGS_RESVD(x)   =((x&7)<<5)
#  PI_SPI_FLAGS_CSPOLS(x)  =((x&7)<<2)
#  PI_SPI_FLAGS_MODE(x)    =((x&3))

  #/* BSC registers */

  BSC_DR         =0
  BSC_RSR        =1
  BSC_SLV        =2
  BSC_CR         =3
  BSC_FR         =4
  BSC_IFLS       =5
  BSC_IMSC       =6
  BSC_RIS        =7
  BSC_MIS        =8
  BSC_ICR        =9
  BSC_DMACR     =10
  BSC_TDR       =11
  BSC_GPUSTAT   =12
  BSC_HCTRL     =13
  BSC_DEBUG_I2C =14
  BSC_DEBUG_SPI =15

  BSC_CR_TESTFIFO =2048
  BSC_CR_RXE  =512
  BSC_CR_TXE  =256
  BSC_CR_BRK  =128
  BSC_CR_CPOL  =16
  BSC_CR_CPHA   =8
  BSC_CR_I2C    =4
  BSC_CR_SPI    =2
  BSC_CR_EN     =1

  BSC_FR_RXBUSY =32
  BSC_FR_TXFE   =16
  BSC_FR_RXFF    =8
  BSC_FR_TXFF    =4
  BSC_FR_RXFE    =2
  BSC_FR_TXBUSY  =1

  #/* BSC GPIO */

  BSC_SDA_MOSI =18
  BSC_SCL_SCLK =19
  BSC_MISO     =20
  BSC_CE_N     =21

  #/* Longest busy delay */

  PI_MAX_BUSY_DELAY =100

  #/* timeout: 0-60000 */

  PI_MIN_WDOG_TIMEOUT =0
  PI_MAX_WDOG_TIMEOUT =60000

  #/* timer: 0-9 */

  PI_MIN_TIMER =0
  PI_MAX_TIMER =9

  #/* millis: 10-60000 */

  PI_MIN_MS =10
  PI_MAX_MS =60000

  PI_MAX_SCRIPTS       =32

  PI_MAX_SCRIPT_TAGS   =50
  PI_MAX_SCRIPT_VARS  =150
  PI_MAX_SCRIPT_PARAMS =10

  #/* script status */

  PI_SCRIPT_INITING =0
  PI_SCRIPT_HALTED  =1
  PI_SCRIPT_RUNNING =2
  PI_SCRIPT_WAITING =3
  PI_SCRIPT_FAILED  =4

  #/* signum: 0-63 */

  PI_MIN_SIGNUM =0
  PI_MAX_SIGNUM =63

  #/* timetype: 0-1 */

  PI_TIME_RELATIVE =0
  PI_TIME_ABSOLUTE =1

  PI_MAX_MICS_DELAY =1000000 #/* 1 second */
  PI_MAX_MILS_DELAY =60000   #/* 60 seconds */

  #/* cfgMillis */

  PI_BUF_MILLIS_MIN =100
  PI_BUF_MILLIS_MAX =10000

  #/* cfgMicros: 1, 2, 4, 5, 8, or 10 */

  #/* cfgPeripheral: 0-1 */

  PI_CLOCK_PWM =0
  PI_CLOCK_PCM =1

  #/* DMA channel: 0-14 */

  PI_MIN_DMA_CHANNEL =0
  PI_MAX_DMA_CHANNEL =14

  #/* port */

  PI_MIN_SOCKET_PORT =1024
  PI_MAX_SOCKET_PORT =32000


  #/* ifFlags: */

  PI_DISABLE_FIFO_IF   =1
  PI_DISABLE_SOCK_IF   =2
  PI_LOCALHOST_SOCK_IF =4

  #/* memAllocMode */

  PI_MEM_ALLOC_AUTO    =0
  PI_MEM_ALLOC_PAGEMAP =1
  PI_MEM_ALLOC_MAILBOX =2

  #/* filters */

  PI_MAX_STEADY  =300000
  PI_MAX_ACTIVE =1000000

  #/* gpioCfgInternals */

  PI_CFG_DBG_LEVEL         =0 #/* bits 0-3 */
  PI_CFG_ALERT_FREQ        =4 #/* bits 4-7 */
  PI_CFG_RT_PRIORITY       =(1<<8)
  PI_CFG_STATS             =(1<<9)

  PI_CFG_ILLEGAL_VAL       =(1<<10)

  #/* gpioISR */

  RISING_EDGE  =0
  FALLING_EDGE =1
  EITHER_EDGE  =2


  #/* pads */

  PI_MAX_PAD =2

  PI_MIN_PAD_STRENGTH =1
  PI_MAX_PAD_STRENGTH =16

  #/* files */

  PI_FILE_NONE   =0
  PI_FILE_MIN    =1
  PI_FILE_READ   =1
  PI_FILE_WRITE  =2
  PI_FILE_RW     =3
  PI_FILE_APPEND =4
  PI_FILE_CREATE =8
  PI_FILE_TRUNC  =16
  PI_FILE_MAX    =31

  PI_FROM_START   =0
  PI_FROM_CURRENT =1
  PI_FROM_END     =2

  #/* Allowed socket connect addresses */

  MAX_CONNECT_ADDRESSES =256

  #/* events */

  PI_MAX_EVENT =31

  #/* Event auto generated on BSC slave activity */

  PI_EVENT_BSC =31


  #/*DEF_S Socket Command Codes*/

  PI_CMD_MODES  =0
  PI_CMD_MODEG  =1
  PI_CMD_PUD    =2
  PI_CMD_READ   =3
  PI_CMD_WRITE  =4
  PI_CMD_PWM    =5
  PI_CMD_PRS    =6
  PI_CMD_PFS    =7
  PI_CMD_SERVO  =8
  PI_CMD_WDOG   =9
  PI_CMD_BR1   =10
  PI_CMD_BR2   =11
  PI_CMD_BC1   =12
  PI_CMD_BC2   =13
  PI_CMD_BS1   =14
  PI_CMD_BS2   =15
  PI_CMD_TICK  =16
  PI_CMD_HWVER =17
  PI_CMD_NO    =18
  PI_CMD_NB    =19
  PI_CMD_NP    =20
  PI_CMD_NC    =21
  PI_CMD_PRG   =22
  PI_CMD_PFG   =23
  PI_CMD_PRRG  =24
  PI_CMD_HELP  =25
  PI_CMD_PIGPV =26
  PI_CMD_WVCLR =27
  PI_CMD_WVAG  =28
  PI_CMD_WVAS  =29
  PI_CMD_WVGO  =30
  PI_CMD_WVGOR =31
  PI_CMD_WVBSY =32
  PI_CMD_WVHLT =33
  PI_CMD_WVSM  =34
  PI_CMD_WVSP  =35
  PI_CMD_WVSC  =36
  PI_CMD_TRIG  =37
  PI_CMD_PROC  =38
  PI_CMD_PROCD =39
  PI_CMD_PROCR =40
  PI_CMD_PROCS =41
  PI_CMD_SLRO  =42
  PI_CMD_SLR   =43
  PI_CMD_SLRC  =44
  PI_CMD_PROCP =45
  PI_CMD_MICS  =46
  PI_CMD_MILS  =47
  PI_CMD_PARSE =48
  PI_CMD_WVCRE =49
  PI_CMD_WVDEL =50
  PI_CMD_WVTX  =51
  PI_CMD_WVTXR =52
  PI_CMD_WVNEW =53

  PI_CMD_I2CO  =54
  PI_CMD_I2CC  =55
  PI_CMD_I2CRD =56
  PI_CMD_I2CWD =57
  PI_CMD_I2CWQ =58
  PI_CMD_I2CRS =59
  PI_CMD_I2CWS =60
  PI_CMD_I2CRB =61
  PI_CMD_I2CWB =62
  PI_CMD_I2CRW =63
  PI_CMD_I2CWW =64
  PI_CMD_I2CRK =65
  PI_CMD_I2CWK =66
  PI_CMD_I2CRI =67
  PI_CMD_I2CWI =68
  PI_CMD_I2CPC =69
  PI_CMD_I2CPK =70

  PI_CMD_SPIO  =71
  PI_CMD_SPIC  =72
  PI_CMD_SPIR  =73
  PI_CMD_SPIW  =74
  PI_CMD_SPIX  =75

  PI_CMD_SERO  =76
  PI_CMD_SERC  =77
  PI_CMD_SERRB =78
  PI_CMD_SERWB =79
  PI_CMD_SERR  =80
  PI_CMD_SERW  =81
  PI_CMD_SERDA =82

  PI_CMD_GDC   =83
  PI_CMD_GPW   =84

  PI_CMD_HC    =85
  PI_CMD_HP    =86

  PI_CMD_CF1   =87
  PI_CMD_CF2   =88

  PI_CMD_BI2CC =89
  PI_CMD_BI2CO =90
  PI_CMD_BI2CZ =91

  PI_CMD_I2CZ  =92

  PI_CMD_WVCHA =93

  PI_CMD_SLRI  =94

  PI_CMD_CGI   =95
  PI_CMD_CSI   =96

  PI_CMD_FG    =97
  PI_CMD_FN    =98

  PI_CMD_NOIB  =99

  PI_CMD_WVTXM =100
  PI_CMD_WVTAT =101

  PI_CMD_PADS  =102
  PI_CMD_PADG  =103

  PI_CMD_FO    =104
  PI_CMD_FC    =105
  PI_CMD_FR    =106
  PI_CMD_FW    =107
  PI_CMD_FS    =108
  PI_CMD_FL    =109

  PI_CMD_SHELL =110

  PI_CMD_BSPIC =111
  PI_CMD_BSPIO =112
  PI_CMD_BSPIX =113

  PI_CMD_BSCX  =114

  PI_CMD_EVM   =115
  PI_CMD_EVT   =116

  #/*DEF_E*/

  #/* pseudo commands */

  PI_CMD_SCRIPT =800

  PI_CMD_ADD   =800
  PI_CMD_AND   =801
  PI_CMD_CALL  =802
  PI_CMD_CMDR  =803
  PI_CMD_CMDW  =804
  PI_CMD_CMP   =805
  PI_CMD_DCR   =806
  PI_CMD_DCRA  =807
  PI_CMD_DIV   =808
  PI_CMD_HALT  =809
  PI_CMD_INR   =810
  PI_CMD_INRA  =811
  PI_CMD_JM    =812
  PI_CMD_JMP   =813
  PI_CMD_JNZ   =814
  PI_CMD_JP    =815
  PI_CMD_JZ    =816
  PI_CMD_TAG   =817
  PI_CMD_LD    =818
  PI_CMD_LDA   =819
  PI_CMD_LDAB  =820
  PI_CMD_MLT   =821
  PI_CMD_MOD   =822
  PI_CMD_NOP   =823
  PI_CMD_OR    =824
  PI_CMD_POP   =825
  PI_CMD_POPA  =826
  PI_CMD_PUSH  =827
  PI_CMD_PUSHA =828
  PI_CMD_RET   =829
  PI_CMD_RL    =830
  PI_CMD_RLA   =831
  PI_CMD_RR    =832
  PI_CMD_RRA   =833
  PI_CMD_STA   =834
  PI_CMD_STAB  =835
  PI_CMD_SUB   =836
  PI_CMD_SYS   =837
  PI_CMD_WAIT  =838
  PI_CMD_X     =839
  PI_CMD_XA    =840
  PI_CMD_XOR   =841
  PI_CMD_EVTWT =842

  #/*DEF_S Error Codes*/

  PI_INIT_FAILED       =-1 #Error Code: gpioInitialise failed
  PI_BAD_USER_GPIO     =-2 #Error Code: GPIO not 0-31
  PI_BAD_GPIO          =-3 #Error Code: GPIO not 0-53
  PI_BAD_MODE          =-4 #Error Code: mode not 0-7
  PI_BAD_LEVEL         =-5 #Error Code: level not 0-1
  PI_BAD_PUD           =-6 #Error Code: pud not 0-2
  PI_BAD_PULSEWIDTH    =-7 #Error Code: pulsewidth not 0 or 500-2500
  PI_BAD_DUTYCYCLE     =-8 #Error Code: dutycycle outside set range
  PI_BAD_TIMER         =-9 #Error Code: timer not 0-9
  PI_BAD_MS           =-10 #Error Code: ms not 10-60000
  PI_BAD_TIMETYPE     =-11 #Error Code: timetype not 0-1
  PI_BAD_SECONDS      =-12 #Error Code: seconds < 0
  PI_BAD_MICROS       =-13 #Error Code: micros not 0-999999
  PI_TIMER_FAILED     =-14 #Error Code: gpioSetTimerFunc failed
  PI_BAD_WDOG_TIMEOUT =-15 #Error Code: timeout not 0-60000
  PI_NO_ALERT_FUNC    =-16 #Error Code: DEPRECATED
  PI_BAD_CLK_PERIPH   =-17 #Error Code: clock peripheral not 0-1
  PI_BAD_CLK_SOURCE   =-18 #Error Code: DEPRECATED
  PI_BAD_CLK_MICROS   =-19 #Error Code: clock micros not 1, 2, 4, 5, 8, or 10
  PI_BAD_BUF_MILLIS   =-20 #Error Code: buf millis not 100-10000
  PI_BAD_DUTYRANGE    =-21 #Error Code: dutycycle range not 25-40000
  PI_BAD_DUTY_RANGE   =-21 #Error Code: DEPRECATED (use PI_BAD_DUTYRANGE)
  PI_BAD_SIGNUM       =-22 #Error Code: signum not 0-63
  PI_BAD_PATHNAME     =-23 #Error Code: can't open pathname
  PI_NO_HANDLE        =-24 #Error Code: no handle available
  PI_BAD_HANDLE       =-25 #Error Code: unknown handle
  PI_BAD_IF_FLAGS     =-26 #Error Code: ifFlags > 3
  PI_BAD_CHANNEL      =-27 #Error Code: DMA channel not 0-14
  PI_BAD_PRIM_CHANNEL =-27 #Error Code: DMA primary channel not 0-14
  PI_BAD_SOCKET_PORT  =-28 #Error Code: socket port not 1024-32000
  PI_BAD_FIFO_COMMAND =-29 #Error Code: unrecognized fifo command
  PI_BAD_SECO_CHANNEL =-30 #Error Code: DMA secondary channel not 0-6
  PI_NOT_INITIALISED  =-31 #Error Code: function called before gpioInitialise
  PI_INITIALISED      =-32 #Error Code: function called after gpioInitialise
  PI_BAD_WAVE_MODE    =-33 #Error Code: waveform mode not 0-3
  PI_BAD_CFG_INTERNAL =-34 #Error Code: bad parameter in gpioCfgInternals call
  PI_BAD_WAVE_BAUD    =-35 #Error Code: baud rate not 50-250K(RX)/50-1M(TX)
  PI_TOO_MANY_PULSES  =-36 #Error Code: waveform has too many pulses
  PI_TOO_MANY_CHARS   =-37 #Error Code: waveform has too many chars
  PI_NOT_SERIAL_GPIO  =-38 #Error Code: no bit bang serial read on GPIO
  PI_BAD_SERIAL_STRUC =-39 #Error Code: bad (null) serial structure parameter
  PI_BAD_SERIAL_BUF   =-40 #Error Code: bad (null) serial buf parameter
  PI_NOT_PERMITTED    =-41 #Error Code: GPIO operation not permitted
  PI_SOME_PERMITTED   =-42 #Error Code: one or more GPIO not permitted
  PI_BAD_WVSC_COMMND  =-43 #Error Code: bad WVSC subcommand
  PI_BAD_WVSM_COMMND  =-44 #Error Code: bad WVSM subcommand
  PI_BAD_WVSP_COMMND  =-45 #Error Code: bad WVSP subcommand
  PI_BAD_PULSELEN     =-46 #Error Code: trigger pulse length not 1-100
  PI_BAD_SCRIPT       =-47 #Error Code: invalid script
  PI_BAD_SCRIPT_ID    =-48 #Error Code: unknown script id
  PI_BAD_SER_OFFSET   =-49 #Error Code: add serial data offset > 30 minutes
  PI_GPIO_IN_USE      =-50 #Error Code: GPIO already in use
  PI_BAD_SERIAL_COUNT =-51 #Error Code: must read at least a byte at a time
  PI_BAD_PARAM_NUM    =-52 #Error Code: script parameter id not 0-9
  PI_DUP_TAG          =-53 #Error Code: script has duplicate tag
  PI_TOO_MANY_TAGS    =-54 #Error Code: script has too many tags
  PI_BAD_SCRIPT_CMD   =-55 #Error Code: illegal script command
  PI_BAD_VAR_NUM      =-56 #Error Code: script variable id not 0-149
  PI_NO_SCRIPT_ROOM   =-57 #Error Code: no more room for scripts
  PI_NO_MEMORY        =-58 #Error Code: can't allocate temporary memory
  PI_SOCK_READ_FAILED =-59 #Error Code: socket read failed
  PI_SOCK_WRIT_FAILED =-60 #Error Code: socket write failed
  PI_TOO_MANY_PARAM   =-61 #Error Code: too many script parameters (> 10)
  PI_NOT_HALTED       =-62 #Error Code: DEPRECATED
  PI_SCRIPT_NOT_READY =-62 #Error Code: script initialising
  PI_BAD_TAG          =-63 #Error Code: script has unresolved tag
  PI_BAD_MICS_DELAY   =-64 #Error Code: bad MICS delay (too large)
  PI_BAD_MILS_DELAY   =-65 #Error Code: bad MILS delay (too large)
  PI_BAD_WAVE_ID      =-66 #Error Code: non existent wave id
  PI_TOO_MANY_CBS     =-67 #Error Code: No more CBs for waveform
  PI_TOO_MANY_OOL     =-68 #Error Code: No more OOL for waveform
  PI_EMPTY_WAVEFORM   =-69 #Error Code: attempt to create an empty waveform
  PI_NO_WAVEFORM_ID   =-70 #Error Code: no more waveforms
  PI_I2C_OPEN_FAILED  =-71 #Error Code: can't open I2C device
  PI_SER_OPEN_FAILED  =-72 #Error Code: can't open serial device
  PI_SPI_OPEN_FAILED  =-73 #Error Code: can't open SPI device
  PI_BAD_I2C_BUS      =-74 #Error Code: bad I2C bus
  PI_BAD_I2C_ADDR     =-75 #Error Code: bad I2C address
  PI_BAD_SPI_CHANNEL  =-76 #Error Code: bad SPI channel
  PI_BAD_FLAGS        =-77 #Error Code: bad i2c/spi/ser open flags
  PI_BAD_SPI_SPEED    =-78 #Error Code: bad SPI speed
  PI_BAD_SER_DEVICE   =-79 #Error Code: bad serial device name
  PI_BAD_SER_SPEED    =-80 #Error Code: bad serial baud rate
  PI_BAD_PARAM        =-81 #Error Code: bad i2c/spi/ser parameter
  PI_I2C_WRITE_FAILED =-82 #Error Code: i2c write failed
  PI_I2C_READ_FAILED  =-83 #Error Code: i2c read failed
  PI_BAD_SPI_COUNT    =-84 #Error Code: bad SPI count
  PI_SER_WRITE_FAILED =-85 #Error Code: ser write failed
  PI_SER_READ_FAILED  =-86 #Error Code: ser read failed
  PI_SER_READ_NO_DATA =-87 #Error Code: ser read no data available
  PI_UNKNOWN_COMMAND  =-88 #Error Code: unknown command
  PI_SPI_XFER_FAILED  =-89 #Error Code: spi xfer/read/write failed
  PI_BAD_POINTER      =-90 #Error Code: bad (NULL) pointer
  PI_NO_AUX_SPI       =-91 #Error Code: no auxiliary SPI on Pi A or B
  PI_NOT_PWM_GPIO     =-92 #Error Code: GPIO is not in use for PWM
  PI_NOT_SERVO_GPIO   =-93 #Error Code: GPIO is not in use for servo pulses
  PI_NOT_HCLK_GPIO    =-94 #Error Code: GPIO has no hardware clock
  PI_NOT_HPWM_GPIO    =-95 #Error Code: GPIO has no hardware PWM
  PI_BAD_HPWM_FREQ    =-96 #Error Code: hardware PWM frequency not 1-125M
  PI_BAD_HPWM_DUTY    =-97 #Error Code: hardware PWM dutycycle not 0-1M
  PI_BAD_HCLK_FREQ    =-98 #Error Code: hardware clock frequency not 4689-250M
  PI_BAD_HCLK_PASS    =-99 #Error Code: need password to use hardware clock 1
  PI_HPWM_ILLEGAL    =-100 #Error Code: illegal, PWM in use for main clock
  PI_BAD_DATABITS    =-101 #Error Code: serial data bits not 1-32
  PI_BAD_STOPBITS    =-102 #Error Code: serial (half) stop bits not 2-8
  PI_MSG_TOOBIG      =-103 #Error Code: socket/pipe message too big
  PI_BAD_MALLOC_MODE =-104 #Error Code: bad memory allocation mode
  PI_TOO_MANY_SEGS   =-105 #Error Code: too many I2C transaction segments
  PI_BAD_I2C_SEG     =-106 #Error Code: an I2C transaction segment failed
  PI_BAD_SMBUS_CMD   =-107 #Error Code: SMBus command not supported by driver
  PI_NOT_I2C_GPIO    =-108 #Error Code: no bit bang I2C in progress on GPIO
  PI_BAD_I2C_WLEN    =-109 #Error Code: bad I2C write length
  PI_BAD_I2C_RLEN    =-110 #Error Code: bad I2C read length
  PI_BAD_I2C_CMD     =-111 #Error Code: bad I2C command
  PI_BAD_I2C_BAUD    =-112 #Error Code: bad I2C baud rate, not 50-500k
  PI_CHAIN_LOOP_CNT  =-113 #Error Code: bad chain loop count
  PI_BAD_CHAIN_LOOP  =-114 #Error Code: empty chain loop
  PI_CHAIN_COUNTER   =-115 #Error Code: too many chain counters
  PI_BAD_CHAIN_CMD   =-116 #Error Code: bad chain command
  PI_BAD_CHAIN_DELAY =-117 #Error Code: bad chain delay micros
  PI_CHAIN_NESTING   =-118 #Error Code: chain counters nested too deeply
  PI_CHAIN_TOO_BIG   =-119 #Error Code: chain is too long
  PI_DEPRECATED      =-120 #Error Code: deprecated function removed
  PI_BAD_SER_INVERT  =-121 #Error Code: bit bang serial invert not 0 or 1
  PI_BAD_EDGE        =-122 #Error Code: bad ISR edge value, not 0-2
  PI_BAD_ISR_INIT    =-123 #Error Code: bad ISR initialisation
  PI_BAD_FOREVER     =-124 #Error Code: loop forever must be last command
  PI_BAD_FILTER      =-125 #Error Code: bad filter parameter
  PI_BAD_PAD         =-126 #Error Code: bad pad number
  PI_BAD_STRENGTH    =-127 #Error Code: bad pad drive strength
  PI_FIL_OPEN_FAILED =-128 #Error Code: file open failed
  PI_BAD_FILE_MODE   =-129 #Error Code: bad file mode
  PI_BAD_FILE_FLAG   =-130 #Error Code: bad file flag
  PI_BAD_FILE_READ   =-131 #Error Code: bad file read
  PI_BAD_FILE_WRITE  =-132 #Error Code: bad file write
  PI_FILE_NOT_ROPEN  =-133 #Error Code: file not open for read
  PI_FILE_NOT_WOPEN  =-134 #Error Code: file not open for write
  PI_BAD_FILE_SEEK   =-135 #Error Code: bad file seek
  PI_NO_FILE_MATCH   =-136 #Error Code: no files match pattern
  PI_NO_FILE_ACCESS  =-137 #Error Code: no permission to access file
  PI_FILE_IS_A_DIR   =-138 #Error Code: file is a directory
  PI_BAD_SHELL_STATUS=-139 #Error Code: bad shell return status
  PI_BAD_SCRIPT_NAME =-140 #Error Code: bad script name
  PI_BAD_SPI_BAUD    =-141 #Error Code: bad SPI baud rate, not 50-500k
  PI_NOT_SPI_GPIO    =-142 #Error Code: no bit bang SPI in progress on GPIO
  PI_BAD_EVENT_ID    =-143 #Error Code: bad event id

  PI_PIGIF_ERR_0    =-2000
  PI_PIGIF_ERR_99   =-2099

  PI_CUSTOM_ERR_0   =-3000
  PI_CUSTOM_ERR_999 =-3999

  #/*DEF_E*/

  #/*DEF_S Defaults*/

  PI_DEFAULT_BUFFER_MILLIS           =120
  PI_DEFAULT_CLK_MICROS              =5
  PI_DEFAULT_CLK_PERIPHERAL          =PI_CLOCK_PCM
  PI_DEFAULT_IF_FLAGS                =0
  PI_DEFAULT_FOREGROUND              =0
  PI_DEFAULT_DMA_CHANNEL             =14
  PI_DEFAULT_DMA_PRIMARY_CHANNEL     =14
  PI_DEFAULT_DMA_SECONDARY_CHANNEL   =6
  PI_DEFAULT_SOCKET_PORT             =8888
  PI_DEFAULT_SOCKET_PORT_STR         ="8888"
  PI_DEFAULT_SOCKET_ADDR_STR         ="127.0.0.1"
  PI_DEFAULT_UPDATE_MASK_UNKNOWN     =0xFFFFFFFF
  PI_DEFAULT_UPDATE_MASK_B1          =0x03E7CF93
  PI_DEFAULT_UPDATE_MASK_A_B2        =0xFBC7CF9C
  PI_DEFAULT_UPDATE_MASK_APLUS_BPLUS =0x0080480FFFFFFC
  PI_DEFAULT_UPDATE_MASK_ZERO        =0x0080000FFFFFFC
  PI_DEFAULT_UPDATE_MASK_PI2B        =0x0080480FFFFFFC
  PI_DEFAULT_UPDATE_MASK_PI3B        =0x0000000FFFFFFC
  PI_DEFAULT_UPDATE_MASK_COMPUTE     =0x00FFFFFFFFFFFF
  PI_DEFAULT_MEM_ALLOC_MODE          =PI_MEM_ALLOC_AUTO

  PI_DEFAULT_CFG_INTERNALS           =0

  Pigif_bad_send           = -2000
  Pigif_bad_recv           = -2001
  Pigif_bad_getaddrinfo    = -2002
  Pigif_bad_connect        = -2003
  Pigif_bad_socket         = -2004
  Pigif_bad_noib           = -2005
  Pigif_duplicate_callback = -2006
  Pigif_bad_malloc         = -2007
  Pigif_bad_callback       = -2008
  Pigif_notify_failed      = -2009
  Pigif_callback_not_found = -2010
  Pigif_unconnected_pi     = -2011
  Pigif_too_many_pis       = -2012

end
