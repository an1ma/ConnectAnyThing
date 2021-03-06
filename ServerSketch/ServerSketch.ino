/*

 Copyright (c) 2013 - Philippe Laulheret, Second Story [http://www.secondstory.com]
 
 This code is licensed under MIT. 
 For more information visit  : https://github.com/secondstory/LYT
 
 */

#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>

#include <libwebsockets.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <syslog.h>
#include <signal.h>

#define N_PANNEL 3
IPAddress ip[N_PANNEL]; 
EthernetUDP udp[N_PANNEL];

int udpPort=3333;
int led = 13; 
boolean currentLED = false;
int sensor1 = 0;
int sensor1Pin = A0;    // select the input pin for the potentiometer

// HW declaration
#define TOTAL_NUM_Dx 14 
#define TOTAL_NUM_Px 12
boolean g_abD[TOTAL_NUM_Dx];
int g_aiP[TOTAL_NUM_Px];

// Serial Interface
int g_iByte = 0;
int g_iNewCode = 0;

#define UP 119 // w
#define DOWN 115 // s
#define LEFT 97 // a
#define RIGHT 100 // d
#define SPACE 32 // space 

EthernetUDP _udpIn;
//char colors[8*N_PANNEL] = "#000000,#000000,#000000";
char  retMessage[(1+1+4+1)]="0,1023";

//-----------------------------------------------------------
// Process recieved message from the webpage here.... 
//------------------------------------------------------------
void procWebMsg(char* _in, size_t _len) {

  /*
  g_abD[i] = false;      digitalWrite(i, LOW);    
   g_aiP[i] = 0;        analogWrite(i, LOW);    
   */

  String sMsg(_in);

  /*
  if( sMsg == "trigger" ) // Debugging
   sMsg = "D13";
   */

  sMsg.toLowerCase();

  Serial.print("input: "); 
  Serial.println(sMsg);
  Serial.print("_len: "); 
  Serial.println(String(_len));

  // Parse incomming message
  if(  sMsg.startsWith("d") && _len <= 3 )
  {
    // Check if we are setting a Dx pin
    int iPin = sMsg.substring(1,sMsg.length()).toInt();
    Serial.print("Pin: ");
    Serial.println(String(iPin));
    g_abD[iPin] = ~g_abD[iPin]; // Toggle state
    digitalWrite(iPin, g_abD[iPin]); // Set state
  }
  else if(  sMsg.startsWith("p") )
  {
    // Check if we are setting a Px pin 

    // Get pin number and value
    int iValue = 0;
    int iPin = 0;

    if( sMsg.charAt(2) == ',' )
    {
      iPin = sMsg.substring(1,2).toInt();
      iValue = sMsg.substring(3,sMsg.length()).toInt();
    }
    else if( sMsg.charAt(3) == ',' )
    {
      iPin = sMsg.substring(1,3).toInt();
      iValue = sMsg.substring(4,sMsg.length()).toInt();      
    }
    else
    {
      Serial.print("Error. Unrecongnized message: ");   
      Serial.println(sMsg);
    }

    // Set pin value
    Serial.print("Pin: ");
    Serial.println(String(iPin));
    Serial.print("Value: ");
    Serial.println(String(iValue));
    g_aiP[iPin] = iValue; // Toggle state
    analogWrite(iPin, g_aiP[iPin]); // Set state
  }
  else
  {
    Serial.print("Error. Unrecongnized message: ");   
    Serial.println(sMsg);
  }

  /*
  if(String(_in) == "trigger"){
   Serial.println("Trigger found");
   if(currentLED){
   digitalWrite(led, LOW);
   currentLED = false;
   }
   else{
   digitalWrite(led, HIGH);
   currentLED = true;
   } 
   }
   */

}

/*
This part of the code is inspired by the stock example coming with libsockets.
 Check it for more details.
 */

int force_exit = 0;
enum lyt_protocols {
  PROTOCOL_HTTP = 0,
  PROTOCOL_LYT,
  PROTOCOL_COUNT
};


#define LOCAL_RESOURCE_PATH "/home/root/srv/"

struct serveable {
  const char *urlpath;
  const char *mimetype;
}; 

/*
static const struct serveable whitelist[] = {
 { 
 "/favicon.ico", "image/x-icon"             }
 ,
 { 
 "/img/2s.png", "image/png"             }
 ,
 { 
 "/img/splash.png", "image/png"             }
 ,
 { 
 "/img/splash2.png", "image/png"             }
 ,
 { 
 "/img/placeholder.png", "image/png"             }
 ,
 { 
 "/img/placeholder-on.png", "image/png"             }
 ,
 { 
 "/css/phone.css", "text/css"             }
 ,
 { 
 "/js/jquery-2.0.3.min.js", "application/javascript"             }
 ,
 { 
 "/js/phone.js", "application/javascript"             }
 ,
 { 
 "/js/pixel.js", "application/javascript"             }
 ,
 { 
 "/js/pixelView.js", "application/javascript"             }
 ,
 { 
 "/js/socketController.js", "application/javascript"             }
 ,
 
 // last one is the default served if no match
 { 
 "/index.html", "text/html"             }
 ,
 };
 */

static const struct serveable whitelist[] = {
  { 
    "/favicon.ico", "image/x-icon"                 }
  ,
  { 
    "/images/Galileo-Ref_01.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_02.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_03.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_03_off.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_04.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_05.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_05_off.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_06.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_06_off.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_07.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_07_off.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_08.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_09.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_10.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_11.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_12.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_12_off.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_13.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_14.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_15.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_15_off.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_16.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_17.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_18.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_18_off.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_19.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_20.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_21.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_21_off.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_22.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_23.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_24.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_25.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_26.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_27.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_28.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_29.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_30.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_31.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_32.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_33.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_34.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_35.png", "image/png"       }
  ,
  { 
    "/images/Galileo-Ref_36.png", "image/png"       }
  ,
  { 
    "/images/blank.gif", "image/png"       }
  ,
  { 
    "/css/phone.css", "text/css"                 }
  ,
  { 
    "/js/jquery-2.0.3.min.js", "application/javascript"                 }
  ,
  { 
    "/js/phone.js", "application/javascript"                 }
  ,
  { 
    "/js/pixel.js", "application/javascript"                 }
  ,
  { 
    "/js/pixelView.js", "application/javascript"                 }
  ,
  { 
    "/js/socketController.js", "application/javascript"                 }
  ,

  /* last one is the default served if no match */
  { 
    "/index.html", "text/html"                 }
  ,
};

/* this protocol server (always the first one) just knows how to do HTTP */

static int callback_http(struct libwebsocket_context *context,
struct libwebsocket *wsi,
enum libwebsocket_callback_reasons reason, void *user,
void *in, size_t len)
{
  //     Serial.println("callback_http()");
  // WE ARE ALWAYS HITTING THIS POINT

  char buf[256];
  int n;

  switch (reason) {

  case LWS_CALLBACK_HTTP:
    lwsl_notice("LWS_CALLBACK_HTTP");

    for (n = 0; n < (sizeof(whitelist) / sizeof(whitelist[0]) - 1); n++)
      if (in && strcmp((const char *)in, whitelist[n].urlpath) == 0)
        break;

    sprintf(buf, LOCAL_RESOURCE_PATH"%s", whitelist[n].urlpath);

    if (libwebsockets_serve_http_file(context, wsi, buf, whitelist[n].mimetype))
      return 1; /* through completion or error, close the socket */

    /*
		 * notice that the sending of the file completes asynchronously,
     		 * we'll get a LWS_CALLBACK_HTTP_FILE_COMPLETION callback when
     		 * it's done
     		 */

    break;

  case LWS_CALLBACK_HTTP_FILE_COMPLETION:

    lwsl_notice("LWS_FILE_COMPLETION");
    return 1;


  default:
    break;
  }

  return 0;
}

/* lyt_protocol*/
static int
callback_lyt_protocol(struct libwebsocket_context *context,
struct libwebsocket *wsi,
enum libwebsocket_callback_reasons reason,
void *user,
void *in, size_t len)
{

  //Serial.println("callback_lyt_protocol()");
  // WE ARE ALWAYS HITTING THIS POINT

  int iNumBytes = -1;

  switch (reason)
  {

  case LWS_CALLBACK_SERVER_WRITEABLE:

    // **********************************
    // Send Data to Website    
    // **********************************
    iNumBytes = sendStatusToWebsite(wsi);
    if (iNumBytes < 0) 
    {
      lwsl_err("ERROR %d writing to socket\n", iNumBytes);
      return 1;
    }

    break;

  case LWS_CALLBACK_RECEIVE:

    // **********************************
    // Process Data Receieved from the Website    
    // **********************************
    procWebMsg((char*) in, len); 		

    break;

  default:
    break;
  }

  return 0;
}

// **********************************
// Send pin status to the Website    
// **********************************
int  sendStatusToWebsite(struct libwebsocket *wsi)
{
  int n;
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  /*
  getSerialCommand(); 
   if( g_iNewCode )
   {
   */
  // NEW CODE
  //Serial.println("NEW CODE");
  // Read pin state HW

  // Send HW status to website
  n = sprintf((char *)p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
  g_abD[0],
  g_abD[1],
  g_abD[2],
  g_aiP[3],   // Px
  g_abD[4],
  g_aiP[5],   // Px
  g_aiP[6],   // Px
  g_abD[7],
  g_abD[8],
  g_aiP[9],   // Px
  g_aiP[10],  // Px
  g_aiP[11],  // Px
  g_abD[12],
  g_abD[13],
  analogRead(A0),
  analogRead(A1),
  analogRead(A2),
  analogRead(A3),
  analogRead(A4),
  analogRead(A5)
    );
  /*
  }
   else
   {
   // OLD CODE
   Serial.println("OLD CODE");
   sensor1 = analogRead(sensor1Pin);
   n = sprintf((char *)p, "%d,%d", currentLED, sensor1);
   }
   */

  //Serial.print("Output Message [sendStatusToWebsite()]:");
  //Serial.println(n);
  n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT); 

  return n;
}


/* list of supported protocols and callbacks */

static struct libwebsocket_protocols protocols[] = {
  /* first protocol must always be HTTP handler */

  {
    "http-only",		/* name */
    callback_http,		/* callback */
    0,			/* per_session_data_size */
    0,			/* max frame size / rx buffer */
  }
  ,
  {
    "touchserver-protocol",
    callback_lyt_protocol,
    0,
    128,
  }
  ,

  { 
    NULL, NULL, 0, 0                 } /* terminator */
};

void sighandler(int sig)
{
  force_exit = 1;
}

static struct option options[] = {

  { 
    NULL, 0, 0, 0                 }
};

int initWebsocket()
{
  int n = 0;
  struct libwebsocket_context *context;
  int opts = LWS_SERVER_OPTION_SKIP_SERVER_CANONICAL_NAME;
  char interface_name[128] = "wlan0";
  const char *iface = NULL;

  unsigned int oldus = 0;
  struct lws_context_creation_info info;

  int debug_level = 7;

  memset(&info, 0, sizeof info);
  info.port = 80;


  signal(SIGINT, sighandler);
  int syslog_options =  LOG_PID | LOG_PERROR;

  /* we will only try to log things according to our debug_level */
  setlogmask(LOG_UPTO (LOG_DEBUG));
  openlog("lwsts", syslog_options, LOG_DAEMON);

  /* tell the library what debug level to emit and to send it to syslog */
  lws_set_log_level(debug_level, lwsl_emit_syslog);

  info.iface = iface;
  info.protocols = protocols;

  info.extensions = libwebsocket_get_internal_extensions();

  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;

  info.gid = -1;
  info.uid = -1;
  info.options = opts;

  context = libwebsocket_create_context(&info);
  if (context == NULL) {
    lwsl_err("libwebsocket init failed\n");
    return -1;
  }

  n = 0;
  while (n >= 0 && !force_exit) {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    if (((unsigned int)tv.tv_usec - oldus) > 50000) {
      libwebsocket_callback_on_writable_all_protocol(&protocols[PROTOCOL_LYT]);
      oldus = tv.tv_usec;

    }
    n = libwebsocket_service(context, 50);

    loop(); // call  Arduino loop as we have taken over the execution flow :[

  }
  libwebsocket_context_destroy(context);
  closelog();
  return 0;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting WebServer");
  system("/home/root/startAP");

  // Init Galileo HW

  // Enable all digital pins as outputs
  for(int i=0; i<14; i++)
  {
    pinMode(i, OUTPUT);
  }

  // Init pins
  digitalWrite(0, LOW);
  digitalWrite(1, LOW);
  digitalWrite(2, LOW);
  digitalWrite(4, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);

  analogWrite(3, 0);
  analogWrite(5, 0);
  analogWrite(6, 0);
  analogWrite(9, 0);
  analogWrite(10, 0);
  analogWrite(11, 0);

  // Init pin state variables
  int i=0;
  for(i=0; i<TOTAL_NUM_Dx; i++)    
    g_abD[i] = false;
  for(i=0; i<TOTAL_NUM_Px; i++)
    g_aiP[i] = 0;   

  _udpIn.begin(3333);
  _udpIn.listen();

  Serial.println("Starting WebSocket");
  initWebsocket();  

}

void loop()
{


}

// Parce Serial Commands
void getSerialCommand() 
{
  if (Serial.available() > 0) {
    // get incoming byte:
    g_iByte = Serial.read();
    Serial.println(g_iByte);

    switch (g_iByte)
    {
    case UP:
      g_iNewCode = 0;      
      break;

    case DOWN:
    case LEFT:
    case RIGHT:
      g_iNewCode = 1;            
      break;
    }
  }

}







