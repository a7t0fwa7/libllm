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

#include "lyutil/ini_config.h"

#include <tuple>
#include "lyutil/error.h"
#include "lyutil/log.h"
#include "lyutil/reader.h"
#include "lyutil/strings.h"

namespace ly {

// -- class IniConfig ----------------------------------------------------------

IniConfig::IniConfig() {}

std::unique_ptr<IniConfig> IniConfig::read(const std::string &filename) {
  std::unique_ptr<IniConfig> config{new IniConfig()};
  config->_filename = filename;

  auto fp = ReadableFile::open(filename);
  Scanner scanner{fp.get()};

  enum State {
    kBegin,
    kSelfLoop,
  };

  Path ini_dir = Path(filename).dirname();
  IniSection section{ini_dir};
  State state = kBegin;
  while (scanner.scan()) {
    std::string line = ly::trim(scanner.getText());
    if (state == kBegin) {
      if (isEmptyLine(line)) {
        // self-loop
      } else if (isHeader(line)) {
        section._name = parseHeader(line);
        state = kSelfLoop;
      } else {
        throw AbortedError(ly::sprintf("invalid line: %s", line));
      }
    } else if (state == kSelfLoop) {
      if (isEmptyLine(line)) {
        // do nothing.
      } else if (isHeader(line)) {
        config->_table.emplace(section.getName(), std::move(section));
        section = IniSection{ini_dir};
        section._name = parseHeader(line);
      } else {
        std::string key, value;
        std::tie(key, value) = parseKeyValue(line);
        section._kvTable[key] = value;
      }
    } else {
      NOT_IMPL();
    }
  }

  if (state == kBegin) {
    throw AbortedError("ini file is empty.");
  }

  config->_table.emplace(section.getName(), std::move(section));

  return config;
}

bool IniConfig::hasSection(const std::string &section) const {
  auto it = _table.find(section);
  return it != _table.end();
}


const IniSection &IniConfig::getSection(const std::string &name) const {
  auto it = _table.find(name);
  if (it == _table.end()) {
    throw ly::AbortedError(ly::sprintf("section not found: %s", name));
  }

  return it->second;
}

bool IniConfig::isEmptyLine(const std::string &s) {
  if (s.empty() || s.front() == ';') {
    return true;
  } else {
    return false;
  }
}

bool IniConfig::isHeader(const std::string &s) {
  if (s.front() == '[' && s.back() == ']') {
    return true;
  } else {
    return false;
  }
}

std::string IniConfig::parseHeader(const std::string &s) {
  if (!isHeader(s)) {
    throw AbortedError(ly::sprintf("invalid line: %s", s));
  }
  
  std::string name = s.substr(1, s.size() - 2);
  name = ly::trim(name);
  if (name.empty()) {
    throw AbortedError(ly::sprintf("invalid ini section: %s", s));
  }

  return name;
}

std::pair<std::string, std::string> IniConfig::parseKeyValue(const std::string &s) {
  auto row = ly::split(s, "=");
  if (row.size() != 2) {
    throw AbortedError(ly::sprintf("invalid line: %s", s));
  }
  std::string key = ly::toLower(ly::trim(row[0]));
  std::string value = ly::trim(row[1]);
  if (key.empty() || value.empty()) {
    throw AbortedError(ly::sprintf("invalid line: %s", s));
  }

  return std::make_pair(key, value);
}


// -- class IniSection ---------------------------------------------------------

IniSection::IniSection(const Path &iniDir) : _iniDir(iniDir) {}

std::string IniSection::getString(const std::string &key) const {
  auto it = _kvTable.find(key);
  if (it == _kvTable.end()) {
    throw AbortedError(ly::sprintf("key not found (ini_session=%s): %s", _name, key));
  }

  return it->second;
}

int IniSection::getInt(const std::string &key) const {
  std::string s = getString(key);
  return ly::atoi(s);
}

float IniSection::getFloat(const std::string &key) const {
  std::string s = getString(key);
  return ly::atof(s);
}

bool IniSection::getBool(const std::string &key) const {
  return ly::stox<bool>(getString(key));
}

Path IniSection::getPath(const std::string &key) const {
  std::string s = getString(key);

  Path path(s);
  if (!path.isabs()) {
    return _iniDir / path;
  } else {
    return path;
  }
}

} // namespace ly
