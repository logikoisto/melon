#include "http/HttpServer.h"
#include "HttpConnection.h"
#include "Log.h"

#include <functional>

namespace melon {
namespace http {

HttpServer::HttpServer(const IpAddress& listen_addr, Scheduler* scheduler)
		:TcpServer(listen_addr, scheduler) {
	setConnectionHandler(std::bind(&HttpServer::handleClient, this, std::placeholders::_1));	
}

void HttpServer::handleClient(TcpConnection::Ptr conn) {
	HttpConnection::Ptr http_conn = std::make_shared<HttpConnection>(conn);

	auto req = http_conn->recvRequest();
	if (req != nullptr) {
		LOG_DEBUG << "receve http request:" << req->toString();
	}

	HttpResponse::Ptr rsp = std::make_shared<HttpResponse>();

	//todo:
	rsp->setHttpStatus(HttpStatus::OK);
	rsp->setHeader("Content-Length", "5");
	rsp->setContent("hello");

	http_conn->sendResponse(rsp);
	LOG_DEBUG << "send http response:" << rsp->toString();

	//todo:
	LOG_DEBUG << "close http connection";
}

}
}
