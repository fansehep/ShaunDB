#pragma once


namespace shaun {

// in c++17 or high, 可以使用 inline 直接修饰 static 类内初始化默认参数.
struct SlogConf {
  inline static bool is_init = false;
  inline static int kDefaultLogBufSize = 4096;
  inline static int CurrentLogLevel = 0;
};


}