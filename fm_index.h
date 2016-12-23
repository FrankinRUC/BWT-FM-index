#pragma once

#include <cstring>
#include <string>
#include <fstream>
#include <algorithm>

#include "fmap.h"
#include "suffix_array.h"

template<
	typename Char,
	typename Index,
	Index sigma = 128,
	Index length = (sigma - 1) * sizeof(Index) / sizeof(Char) * 4,
	Index distance = sizeof(Index) / sizeof(Char) * 4
> class fm_index {
private:
	class config {
	public:
	//private:
		Index count[sigma + 1];
		friend inline std::istream &operator >> (std::istream &in, config &conf) {
			return in.read((char *)&conf, sizeof(config));
		}
		friend inline std::ostream &operator<<(std::ostream &out, const config &conf) {
			return out.write((char *)&conf, sizeof(config));
		}
	public:
		inline config(const std::vector<Index> &v) { std::copy(v.begin(), v.begin() + sigma + 1, count); }
		inline config(const std::string &filename) { std::ifstream(filename, std::ios::binary) >> *this; }
		inline Index operator[](Index ch) { return count[ch]; }
	};
	struct block {
		Index check_point[sigma - 1];
		Char string[length];
		Index index[length / distance];
		inline block() { memset(this, 0, sizeof(block)); }
		friend inline std::ostream &operator<<(std::ostream &out, const block &bl) {
			return out.write((char *)&bl, sizeof(block));
		}
		inline Index count(Index offset, Char ch) const {
			auto ret = check_point[ch - 1];
			for (Index i = 0; i < offset; ++i) {
				if (string[i] == ch) {
					++ret;
				}
			}
			return ret;
		}
		inline Index count(Index offset) const {
			return check(offset, string[offset]);
		}
	};
	config conf;
	basic_ifmap<block> map;
public:
	template<typename RandomIt>
	static void create(RandomIt first, RandomIt last, const std::string &filename) {
		suffix_array<Index> sa(first, last, sigma);
		std::ofstream(filename + ".config") << config(sa.get_count());
		std::ofstream out(filename, std::ios::binary);
		Index cnt[sigma] = { 0 };
		Index ptr = 0;
		block tmp;
		std::copy(cnt + 1, cnt + sigma, tmp.check_point);
		for(auto i : sa) {
			if(ptr == length) {
				out << tmp;
				std::copy(cnt + 1, cnt + sigma, tmp.check_point);
				ptr = 0;
			}
			++cnt[tmp.string[ptr] = (i ? *(first + (i - 1)) : 0)];
			if(!(ptr % distance)) {
				tmp.index[ptr / distance] = i;
			}
			++ptr;
		}
		if(ptr) {
			for(; ptr != length; ++ptr) {
				tmp.string[ptr] = 0;
				if(!(ptr % distance)) {
					tmp.index[ptr / distance] = 0;
				}
			}
			out << tmp;
		}
	}
	inline fm_index(const std::string &filename) : conf(filename + ".config"), map(filename) {
	}
	inline Char operator[](const Index &offset) {
		return map[offset / length].string[offset % length];
	}
	inline Index count(Index offset, Char ch) {
		return conf[ch] + map[offset / length].count(offset % length, ch);
	}
	inline Index count(Index offset) {
		return count(offset, (*this)[offset]);
	}
	inline Index index(Index offset) {
		Index ret = 0;
		for (; (offset % length) % distance && (*this)[offset]; ++ret) {
			offset = count(offset);
		}
		if ((*this)[offset]) {
			ret += map[offset / length].index[(offset % length) / distance];
		}
		return ret;
	}
	template <typename BidirIt>
	std::vector<Index> query(BidirIt first, BidirIt last) { // reverse search
		Index up = 0, down = conf[sigma];
		while(last != first) {
			if (up == down) {
				break;
			}
			--last;
			up = count(up, *last);
			down = count(down, *last);
		}
		std::vector<Index> ret;
		for (; up != down; ++up) {
			ret.push_back(index(up));
		}
		std::sort(ret.begin(), ret.end());
		return ret;
	}
	inline std::vector<Index> query(const std::string &str) {
		return query(str.begin(), str.end());
	}
};
