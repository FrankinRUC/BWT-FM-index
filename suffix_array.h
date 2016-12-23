#pragma once

//#include <utility>
#include <vector>

template<typename Type>
class suffix_array : public std::vector<Type> {
private:
	using super = std::vector<Type>;
	struct LMS_iterator : public super::const_iterator {
		using super = typename suffix_array::super::const_iterator;
		using value_type = typename super::value_type;
		using difference_type = typename super::difference_type;
		using pointer = value_type *;
		using reference = value_type &;
		inline LMS_iterator(super iter) : super(iter) { }
		inline LMS_iterator &operator++() { super::operator+=(2); return *this; }
		inline LMS_iterator operator++(int) { LMS_iterator ret = *this; ++(*this); return ret; }
		inline LMS_iterator &operator--() { super::operator-=(2); return *this; }
		inline LMS_iterator operator--(int) { LMS_iterator ret = *this; --(*this); return ret; }
		inline LMS_iterator &operator+=(const difference_type &n) { super::operator+=(2 * n); return *this; }
		inline LMS_iterator operator+(const difference_type &n) const { return LMS_iterator(*this) += n; }
		friend inline LMS_iterator operator+(const difference_type &n, const LMS_iterator &iter) { return iter + n; }
		inline LMS_iterator &operator-=(const difference_type &n) { super::operator-=(2 * n); return *this; }
		inline LMS_iterator operator-(const difference_type &n) const { return LMS_iterator(*this) -= n; }
		friend inline difference_type operator-(const LMS_iterator &a, const LMS_iterator &b) { return ((super)a - (super)b) / 2; }
		inline reference operator[](const difference_type &n) { return super::operator[](n * 2); }
	};
	super rank;
	const Type sigma;
	super count;
	inline void init() {
		super::assign(super::size(), 0);
	}
	inline void bucketl(super &b) {
		b.assign(count.begin(), count.end() - 1);
	}
	inline void buckets(super &b) {
		b.assign(count.begin() + 1, count.end());
	}
	template<typename RandomIt>
	inline void pushl(RandomIt first, Type index, super &bucket) {
		(*this)[bucket[*(first + index)]++] = index;
	}
	template<typename RandomIt>
	inline void pushs(RandomIt first, Type index, super &bucket) {
		(*this)[--bucket[*(first + index)]] = index;
	}
	inline bool is_LMS(Type index, const std::vector<bool> &type) {
		return index && type[index] && !type[index - 1];
	}
	template<typename RandomIt>
	inline bool cmp_LMS(RandomIt first, Type a, Type b, const std::vector<bool> &type);
	template<typename RandomIt>
	inline void induced_sort(RandomIt first, const std::vector<bool> &type);
	template<typename RandomIt>
	bool induced_sort_judge(RandomIt first, std::vector<bool> &type);
	template<typename RandomIt>
	inline void SA_IS(RandomIt first);
public:
	template<
		typename RandomIt
	> inline suffix_array(RandomIt first, RandomIt last, Type sigma) : std::vector<Type>(last - first, 0), rank(last - first, 0), sigma(sigma), count(sigma + 1, 0) {
		SA_IS(first);
	}
	~suffix_array() {
	}
	inline const super &get_count() const {
		return count;
	}
};

template<typename Type>
template<typename RandomIt>
inline bool suffix_array<Type>::cmp_LMS(RandomIt first, Type a, Type b, const std::vector<bool> &type) {
	if (*(first + a) != *(first + b)) {
		return true;
	}
	for (++a, ++b; *(first + a) == *(first + b) && !is_LMS(a, type) && !is_LMS(b, type); ++a, ++b) {
	}
	return *(first + a) != *(first + b);
}

template<typename Type>
template<typename RandomIt>
inline void suffix_array<Type>::induced_sort(RandomIt first, const std::vector<bool> &type) {
	super b;
	bucketl(b);
	for (auto iter = super::begin(); iter != super::end(); ++iter) {
		if (*iter > 0 && !type[*iter - 1]) {
			pushl(first, *iter - 1, b);
		}
	}
	buckets(b);
	for (auto iter = super::rbegin(); iter != super::rend(); ++iter) {
		if (*iter > 0 && type[*iter - 1]) {
			pushs(first, *iter - 1, b);
		}
	}
}

template<typename Type>
template<typename RandomIt>
bool suffix_array<Type>::induced_sort_judge(RandomIt first, std::vector<bool> &type) {
	auto stack = rank.begin();
	auto iter = first + super::size();
	auto back = sigma;
	for (auto type_iter = type.rbegin(); type_iter != type.rend(); ++type_iter) {
		auto now = *--iter;
		if (now < back || (now == back && *(type_iter - 1))) {
			*type_iter = true;
		} else if (type_iter != type.rbegin() && *(type_iter - 1)) {
			*stack++ = (iter - first) + 1;
		}
		++count[back = now];
	}
	Type sum = 0;
	bool flag = false;
	for (auto &cnt : count) {
		if (cnt > 1) {
			flag = true;
		}
		auto tmp = sum + cnt;
		cnt = sum;
		sum = tmp;
	}
	super b;
	buckets(b);
	if (flag) {
		while (stack != rank.begin()) {
			pushs(first, *--stack, b);
			*stack = 0;
		}
	} else {
		for (Type i = 0; i < super::size(); ++i) {
			pushs(first, i, b);
		}
	}
	return flag;
}

template<typename Type>
template<typename RandomIt>
void suffix_array<Type>::SA_IS(RandomIt first) {
	std::vector<bool> type(super::size(), false);
	if (sigma && !super::empty() && induced_sort_judge(first, type)) {
		induced_sort(first, type);
		Type last_index = super::size() - 1;
		Type next_sigma = 0;
		for (auto iter = super::begin() + 1; iter != super::end(); ++iter) {
			if (is_LMS(*iter, type)) {
				if (cmp_LMS(first, *iter, last_index, type)) {
					++next_sigma;
				}
				rank[last_index = *iter] = next_sigma;
			}
		}
		auto top = rank.begin();
		for (auto iter = rank.begin(); iter != rank.end() - 1; ++iter) {
			if (*iter) {
				*top++ = *iter;
				*top++ = iter - rank.begin();
			}
		}
		*top++ = 0;
		*top++ = rank.end() - rank.begin() - 1;
		suffix_array LMS_array(LMS_iterator(rank.begin()), LMS_iterator(top), next_sigma + 1);
		init();
		super b;
		buckets(b);
		for (auto iter = LMS_array.rbegin(); iter != LMS_array.rend(); ++iter) {
			pushs(first, rank[*iter * 2 + 1], b);
		}
		induced_sort(first, type);
	}
}
