#ifndef SRC_SERVER_CONFIGPARSE_H_
#define SRC_SERVER_CONFIGPARSE_H_

#include <yaml.h>
#include <string>
#include "src/base/log/logger.hpp"

using ::fver::base::log::Logger;

namespace fver {

namespace server {

struct ServerConfig;

bool ConfigParse(ServerConfig* config, const std::string& path);

}  // namespace server

}  // namespace fver

#endif