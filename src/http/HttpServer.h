#ifndef _MELON_HTTP_SERVER_H_
#define _MELON_HTTP_SERVER_H_

#include "TcpServer.h"

namespace melon {
namespace http {
	
	
class HttpServer : public TcpServer {
public:
	HttpServer(const IpAddress& listen_addr, std::shared_ptr<Scheduler> scheduler);
	~HttpServer() {}
	
protected:
	virtual void handleClient(TcpConnection::Ptr conn) override;

};

}
}

#endif