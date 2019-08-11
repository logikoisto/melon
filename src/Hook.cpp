#include "Coroutine.h"
#include "CoroutineScheduler.h"
#include "Hook.h"
#include "Log.h"

#include "assert.h"
#include <dlfcn.h>
#include <errno.h>
#include <memory>

namespace melon {

struct HookIniter {
	HookIniter() {
		accept_origin = (accept_func)::dlsym(RTLD_NEXT, "accept");
		read_origin = (read_func)::dlsym(RTLD_NEXT, "read");
		write_origin = (write_func)::dlsym(RTLD_NEXT, "write");
	}
};

static HookIniter hook_initer;

}

extern "C" {

accept_func accept_origin = nullptr;
read_func read_origin = nullptr;
write_func write_origin = nullptr;

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen) {
	ssize_t n;
retry:
	do {
		n = accept_origin(fd, addr, addrlen);
	} while (n == -1 && errno == EINTR);

	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
		if (!scheduler) {
			LOG_FATAL << "call hooked api in non io thread";
		}

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		assert(melon::Coroutine::GetCurrentCoroutine().use_count() == 1);
		scheduler->updateEvent(fd, melon::Poller::kReadEvent, melon::Coroutine::GetCurrentCoroutine());
		melon::Coroutine::Yield();
		//清除事件
		scheduler->updateEvent(fd, melon::Poller::kNoneEvent, melon::Coroutine::GetCurrentCoroutine());
		
		goto retry;
	}

	return n;
}

ssize_t read(int fd, void *buf, size_t count) {
	ssize_t n;
retry:
	do {
		n = read_origin(fd, buf, count);
	} while (n == -1 && errno == EINTR);

	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
		if (!scheduler) {
			LOG_FATAL << "call hooked api in non io thread";
		}

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		assert(melon::Coroutine::GetCurrentCoroutine().use_count() == 1);
		scheduler->updateEvent(fd, melon::Poller::kReadEvent, melon::Coroutine::GetCurrentCoroutine());
		melon::Coroutine::Yield();
		//清除事件
		scheduler->updateEvent(fd, melon::Poller::kNoneEvent, melon::Coroutine::GetCurrentCoroutine());

		goto retry;
	}

	return n;
}

ssize_t write(int fd, const void *buf, size_t count) {
	ssize_t n;
retry:
	do {
		n = write_origin(fd, buf, count);
	} while (n == -1 && errno == EINTR);

	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
		if (!scheduler) {
			LOG_FATAL << "call hooked api in non io thread";
		}

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		assert(melon::Coroutine::GetCurrentCoroutine().use_count() == 1);
		scheduler->updateEvent(fd, melon::Poller::kWriteEvent, melon::Coroutine::GetCurrentCoroutine());
		melon::Coroutine::Yield();
		//清除事件
		scheduler->updateEvent(fd, melon::Poller::kNoneEvent, melon::Coroutine::GetCurrentCoroutine());

		goto retry;
	}

	return n;
}

}


