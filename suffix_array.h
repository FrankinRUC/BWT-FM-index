#include <utility>
#include <vector>
#include <stack>
#include <algorithm>

#include <iostream>

using namespace std;

template <
	typename Container
> struct suffix_array : public Container {
	const size_t sigma;
	std::vector<size_t> SA;
	std::vector<size_t> cnt;
	inline void SA_init() {
		for (auto &index : SA) {
			index = 0;
		}
	}
	inline void lbucket_init(std::vector<size_t> &bucket) {
		bucket[0] = 0;
		std::copy(cnt.begin(), cnt.end() - 1, bucket.begin() + 1);
	}
	inline void sbucket_init(std::vector<size_t> &bucket) {
		std::copy(cnt.begin(), cnt.end(), bucket.begin());
	}
	inline void lbucket_add(size_t index, std::vector<size_t> &bucket) {
		SA[bucket[(*this)[index]]++] = index;
	}
	inline void sbucket_add(size_t index, std::vector<size_t> &bucket) {
		SA[--bucket[(*this)[index]]] = index;
	}
	inline void induced_sort(const std::vector<bool> &type) {
		std::vector<size_t> bucket(sigma);
		lbucket_init(bucket);
		for (auto iter = SA.begin(); iter != SA.end(); ++iter) {
			if (*iter > 0 && !type[*iter - 1]) {
				lbucket_add(*iter - 1, bucket);
			}
		}
		sbucket_init(bucket);
		for (auto iter = SA.rbegin(); iter != SA.rend(); ++iter) {
			if (*iter > 0 && type[*iter - 1]) {
				sbucket_add(*iter - 1, bucket);
			}
		}
	}
	inline bool induced_sort_judge(std::vector<bool> &type) {
		std::stack<size_t> LMS;
		auto iter = end();
		auto back = sigma;
		for (auto type_iter = type.rbegin(); type_iter != type.rend(); ++type_iter) {
			auto now = *--iter;
			if (now < back || (now == back && *(type_iter - 1))) {
				*type_iter = true;
			} else if (type_iter != type.rbegin() && *(type_iter - 1)) {
				LMS.push(iter - begin() + 1);
			}
			++cnt[back = now];
		}
		size_t sum = 0;
		bool flag = false;
		for (auto &c : cnt) {
			if (c > 1) {
				flag = true;
			}
			c = (sum += c);
		}
		vector<size_t> bucket(sigma);
		sbucket_init(bucket);
		if (flag) {
			for (; !LMS.empty(); LMS.pop()) {
				sbucket_add(LMS.top(), bucket);
			}
		} else {
			for (size_t i = 0; i < size(); ++i) {
				sbucket_add(i, bucket);
			}
		}
		return flag;
	}
	inline bool is_LMS(size_t index, const std::vector<bool> &type) {
		return index && type[index] && !type[index - 1];
	}
	inline bool cmp_LMS(size_t a, size_t b, const std::vector<bool> &type) {
		if ((*this)[a] != (*this)[b]) {
			return true;
		}
		for (++a, ++b; (*this)[a] == (*this)[b] && !is_LMS(a, type) && !is_LMS(b, type); ++a, ++b) {
		}
		return (*this)[a] != (*this)[b];
	}
	void SA_IS() {
		std::vector<bool> type(size(), false);
		if (sigma && !empty() && induced_sort_judge(type)) {
			induced_sort(type);
			size_t next_sigma = 0;
			vector<std::pair<size_t, size_t> > LMS;
			for (auto index : SA) {
				if (is_LMS(index, type)) {
					if (LMS.empty()) {
						LMS.push_back(std::make_pair(index, next_sigma));
					} else {
						if (cmp_LMS(LMS.back().first, index, type)) {
							++next_sigma;
						}
						LMS.push_back(std::make_pair(index, next_sigma));
					}
				}
			}
			sort(LMS.begin(), LMS.end(), [](auto &a, auto &b) {return a.first < b.first;});
			vector<size_t> tmp;
			for (auto &p : LMS) {
				tmp.push_back(p.second);
			}
			suffix_array<std::vector<size_t> > LMS_SA(next_sigma + 1, tmp);
			SA_init();
			vector<size_t> bucket(sigma);
			sbucket_init(bucket);
			for (auto iter = LMS_SA.SA.rbegin(); iter != LMS_SA.SA.rend(); ++iter) {
				sbucket_add(LMS[*iter].first, bucket);
			}
			induced_sort(type);
		}
	}
	suffix_array(size_t sigma, const Container &other) : Container(other), sigma(sigma), SA(size(), 0), cnt(sigma, 0) {
		SA_IS();
	}
	template<
		typename InputIt
	> suffix_array(size_t sigma, InputIt first, InputIt last) : Container(first, last), sigma(sigma), SA(size(), 0), cnt(sigma, 0) {
		SA_IS();
	}
};
