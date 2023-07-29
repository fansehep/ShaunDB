#pragma once


namespace shaun {


#define Info(str, ...)

#define Warn(str, ...)

#define Debug(str, ...)

#define Trace(str, ...)

#define Error(str, ...)

#define Exit(str, ...)
  

#ifndef DEBUG

#define Dinfo(str, ...)

#define Dwarn(str, ...)

#define Dtrace(str, ...)



#define Dinfo(str, ...) void;
#define 



}