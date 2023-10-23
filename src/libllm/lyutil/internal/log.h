// The MIT License (MIT)
//
// Copyright (c) 2023 Xiaoyang Chen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once 

#include "lyutil/log.h"

namespace ly {
namespace internal {

extern LogSeverity gLogLevel;

class LogWrapper {
 public:
  LogWrapper(LogSeverity severity, const char *source_file, int source_line);
  ~LogWrapper();

  LogWrapper(LogWrapper &) = delete;
  LogWrapper &operator=(LogWrapper &) = delete;

  template <typename T>
  LogWrapper& operator<<(const T &value) { os_ << value; return *this; }

  // set the default message to LogWrapper. If no message appended, it will
  // log the `message` instead
  LogWrapper &DefaultMessage(const char *message);

 private:
  std::ostringstream os_;
  const char *default_message_;

  LogSeverity severity_;
  const char *source_file_;
  int source_line_;
  char time_[200];

  const char *Time();
  const char *Severity() const;
};

// log wrappers for each severity
class LogWrapperDEBUG : public LogWrapper {
 public:
  LogWrapperDEBUG(const char *source_file, int source_line) : 
      LogWrapper(LogSeverity::DEBUG, source_file, source_line) {}
};
class LogWrapperINFO : public LogWrapper {
 public:
  LogWrapperINFO(const char *source_file, int source_line) : 
      LogWrapper(LogSeverity::INFO, source_file, source_line) {}
};
class LogWrapperWARN : public LogWrapper {
 public:
  LogWrapperWARN(const char *source_file, int source_line) : 
      LogWrapper(LogSeverity::WARN, source_file, source_line) {}
};
class LogWrapperERROR : public LogWrapper {
 public:
  LogWrapperERROR(const char *source_file, int source_line) : 
      LogWrapper(LogSeverity::ERROR, source_file, source_line) {}
};
class LogWrapperFATAL : public LogWrapper {
 public:
  LogWrapperFATAL(const char *source_file, int source_line) : 
      LogWrapper(LogSeverity::FATAL, source_file, source_line) {}
};

}  // namespace internal
} // namespace ly

