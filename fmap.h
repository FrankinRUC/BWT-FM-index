#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>

template<
	typename Block
> class basic_ifmap {
private:
	int fd;
	const Block *addr;
	size_t len;
	size_t *referer;
public:
	inline bool is_open() {
		return referer;
	}
	void close() {
		if (is_open() && !--*referer) {
			munmap((void *)addr, len * sizeof(Block));
			::close(fd);
			delete referer;
		}
		fd = 0;
		addr = nullptr;
		len = 0;
		referer = nullptr;
	}
	void open(const char *filename) {
		close();
		fd = ::open(filename, O_RDONLY);
		if (fd == -1) {
			return;
		}
		struct stat fdstat;
		if (fstat(fd, &fdstat) == -1) {
			return;
		}
		void *ptr = mmap(NULL, fdstat.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if ((long long)ptr == (long long)(-1) || fdstat.st_size % sizeof(Block)) {
			return;
		}
		addr = (Block *)ptr;
		len = fdstat.st_size / sizeof(Block);
		referer = new size_t;
		*referer = 1;
	}
	basic_ifmap(const char *filename) : fd(0), addr(nullptr), len(0), referer(nullptr) {
		open(filename);
	}
	inline basic_ifmap(const std::string &filename) : basic_ifmap(filename.c_str()) {
	}
	inline basic_ifmap(const basic_ifmap &other) : fd(other.fd), addr(other.addr), len(other.len), referer(other.referer) {
		++*referer;
	}
	inline ~basic_ifmap() {
		close();
	}
	inline size_t size() { return len; }
	inline bool empty() { return !len; }
	inline const Block *begin() {
		return addr;
	}
	inline const Block *end() {
		return addr + len;
	}
	inline const Block &operator[](size_t offset) {
		return *(addr + offset);
	}
};

using ifmap = basic_ifmap<char>;
