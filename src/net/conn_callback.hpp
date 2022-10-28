#ifndef SRC_NET_CONN_CALLBACK_H_
#define SRC_NET_CONN_CALLBACK_H_

#include <cstdint>
#include <functional>
extern "C" {
#include <event2/bufferevent.h>
#include <event2/event.h>
}
namespace fver {
namespace net {

class Connection;

namespace callback {

void ConnectionWriteCallback(struct bufferevent* buf, void* data);
void ConnectionReadCallback(struct bufferevent* buf, void* data);
void ConnectionEventCallback(struct bufferevent* buf, short eventWhat,
                             void* data);

}  // namespace callback

using writeHandle = std::function<int(Connection*)>;
using closeHandle = std::function<int(Connection*)>;
using timeoutHandle = std::function<int(Connection*)>;
using readHandle = std::function<int(char*, size_t, Connection*)>;

}  // namespace net

}  // namespace fver

#endif