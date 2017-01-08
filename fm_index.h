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
	Index Sigma = 128,
	Index QueryLen = 0x100,
	Index SliceLen = 0x8000000,
	Index BlockLen = (Sigma - 1) * sizeof(Index) / sizeof(Char) * 4,
	Index IndexDist = sizeof(Index) / sizeof(Char) * 4
> class fm_index {
private:
	struct block {
		Char string[BlockLen];
		Index index[(BlockLen + IndexDist - 1) / IndexDist];
		Index check_point[Sigma - 1];
		inline block() { memset(this, 0, sizeof(block)); }
		friend inline std::ostream &operator<<(std::ostream &out, const block &bl) {
			return out.write((char *)&bl, sizeof(block));
		}
	};
	basic_ifmap<block> map;
	template<typename RandomIt>
	static void slice_create(RandomIt first, RandomIt last, std::ostream &out) {
		suffix_array<Index> sa(first, last, Sigma);
		block tmp;
		std::copy(sa.get_count().begin() + 1, sa.get_count().begin() + Sigma, tmp.check_point);
		Index ptr = 0;
		for(auto i : sa) {
			if(i) {
				++tmp.check_point[(tmp.string[ptr] = *(first + (i - 1))) - 1];
			} else {
				tmp.string[ptr] = 0;
			}
			if(!(ptr % IndexDist)) {
				tmp.index[ptr / IndexDist] = i;
			}
			if(++ptr == BlockLen) {
				out << tmp;
				ptr = 0;
			}
		}
		if(ptr) {
			for(; ptr != BlockLen; ++ptr) {
				tmp.string[ptr] = 0;
				if(!(ptr % IndexDist)) {
					tmp.index[ptr / IndexDist] = 0;
				}
			}
			out << tmp;
		}
		return;
	}
	inline Char slice_get(const block *slice, Index pos) {
		return slice[pos / BlockLen].string[pos % BlockLen];
	}
	inline Index slice_count(const block *slice, Index pos, Char ch) {
		Index segment = pos / BlockLen, offset = pos % BlockLen, ret = 0;
		if(segment && offset < BlockLen / 2) {
			ret = slice[segment - 1].check_point[ch - 1];
			for(auto ptr = slice[segment].string; ptr < slice[segment].string + offset; ret += (*ptr++ == ch)) {
			}
		} else {
			ret = slice[segment].check_point[ch - 1];
			for(auto ptr = slice[segment].string + BlockLen; ptr > slice[segment].string + offset; ret -= (*--ptr == ch)) {
			}
		}
		return ret;
	}
	inline Index slice_count(const block *slice, Index pos) {
		return slice_count(slice, pos, slice_get(slice, pos));
	}
	inline Index slice_index(const block *slice, Index pos) {
		Index ret = 0;
		for (; (pos % BlockLen) % IndexDist && slice_get(slice, pos); ++ret) {
			pos = slice_count(slice, pos);
		}
		if (slice_get(slice, pos)) {
			ret += slice[pos / BlockLen].index[(pos % BlockLen) / IndexDist];
		}
		return ret;
	}
	template <typename BidirIt>
	std::vector<Index> slice_query(const block *slice_first, const block *slice_last, BidirIt first, BidirIt last) {
		Index up = 0, down = (slice_last - 1)->check_point[Sigma - 2];
		while(last != first) {
			if (up == down) {
				break;
			}
			--last;
			up = slice_count(slice_first, up, *last);
			down = slice_count(slice_first, down, *last);
		}
		std::vector<Index> ret;
		for (; up != down; ++up) {
			ret.push_back(slice_index(slice_first, up));
		}
		std::sort(ret.begin(), ret.end());
		return ret;
	}
public:
	template<typename RandomIt>
	static void create(RandomIt first, RandomIt last, const std::string &filename) {
		std::ofstream out(filename, std::ios::binary);
		for(; last - first > SliceLen + QueryLen; first += SliceLen) {
			auto iter = first + (SliceLen + QueryLen - 1);
			auto tmp = *iter;
			*iter = 0;
			slice_create(first, iter + 1, out);
			*iter = tmp;
		}
		if(last - first > QueryLen) {
			slice_create(first, last, out);
		}
		out.close();
		return;
	}
	inline fm_index(const std::string &filename) : map(filename) {
	}
	template<typename BidirIt>
	std::vector<Index> query(BidirIt first, BidirIt last) {
		const Index slice_size = (SliceLen + QueryLen + BlockLen - 1) / BlockLen;
		std::vector<Index> ret;
		auto ptr = map.begin();
		for(; map.end() - ptr > slice_size; ptr += slice_size) {
			for(auto i : slice_query(ptr, ptr + slice_size, first, last)) {
				auto index = i + (ptr- map.begin()) / slice_size
				 * SliceLen;
				if(ret.empty() || ret.back() < index) {
					ret.push_back(index);
				}
			}
		}
		for(auto i : slice_query(ptr, map.end(), first, last)) {
			auto index = i + (ptr- map.begin()) * SliceLen;
			if(ret.empty() || ret.back() < index) {
				ret.push_back(index);
			}
		}
		return ret;
	}
};
