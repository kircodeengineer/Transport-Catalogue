#pragma once

#include <iterator>
#include <string_view> // зачем?
#include <unordered_map> // зачем?
#include <unordered_set> // зачем?

namespace ranges {
	/// @brief класс, реализующий поиск кратчайшего пути во взвешенном ориентированном графе
	template <typename It>
	class Range {
	public:
		using ValueType = typename std::iterator_traits<It>::value_type;

		Range(It begin, It end)
			: begin_(begin)
			, end_(end) {
		}
		It begin() const {
			return begin_;
		}
		It end() const {
			return end_;
		}

	private:
		It begin_;
		It end_;
	};

	template <typename C>
	auto AsRange(const C& container) {
		return Range{ container.begin(), container.end() };
	}

}  // namespace ranges
