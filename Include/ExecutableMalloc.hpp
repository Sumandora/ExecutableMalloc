#ifndef EXECUTABLEMALLOC_HPP
#define EXECUTABLEMALLOC_HPP

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <new>
#include <optional>
#include <ranges>
#include <set>
#include <utility>
#include <vector>

namespace ExecutableMalloc {
	class MemoryBlockAllocator;
	class MemoryMapping;

	class MemoryRegion {
		std::uintptr_t from;
		std::uintptr_t to;
		MemoryMapping* parent;

		friend MemoryMapping;

		MemoryRegion(std::uintptr_t from, std::uintptr_t to, MemoryMapping* parent)
			: from(from)
			, to(to)
			, parent(parent)
		{
		}

		struct PtrOrder {
			bool operator()(const MemoryRegion* lhs, const MemoryRegion* rhs) const
			{
				return std::less{}(lhs->get_from(), rhs->get_from());
			}
		};

	public:
		MemoryRegion() = delete;
		MemoryRegion(const MemoryRegion&) = delete;
		void operator=(const MemoryRegion&) = delete;
		~MemoryRegion();

		std::strong_ordering operator<=>(const MemoryRegion& other) const { return from <=> other.from; }

		[[nodiscard]] std::uintptr_t get_from() const { return from; }
		[[nodiscard]] std::uintptr_t get_to() const { return to; }
		[[nodiscard]] const MemoryMapping* get_parent() const { return parent; }

		[[nodiscard]] bool is_writable() const;
		void set_writable(bool writable);
		void resize(std::size_t size) // The new size needs to be smaller or equally big as the current region
		{
			if (size > to - from)
				throw std::exception{};

			to = from + size;
		}
	};

	class MemoryMapping {
		MemoryBlockAllocator* parent;
		std::uintptr_t from;
		std::uintptr_t to;
		std::set<MemoryRegion*, MemoryRegion::PtrOrder> used_regions;
		bool writable;

		[[nodiscard]] std::optional<std::uintptr_t> find_region(std::size_t size) const
		{
			std::uintptr_t p = from;
			for (const auto& region : used_regions) {
				if (region->get_from() - p >= size)
					return p;
				p = region->get_to();
			}
			if (to - p >= size)
				return p;
			return std::nullopt;
		}

		[[nodiscard]] std::optional<std::uintptr_t> find_region_reverse(std::size_t size) const
		{
			std::uintptr_t p = to;
			for (const auto& region : used_regions | std::ranges::views::reverse) {
				if (p - region->get_to() >= size)
					return p - size;
				p = region->get_from();
			}
			if (p - from >= size)
				return p - size;
			return std::nullopt;
		}

		friend MemoryRegion;

		MemoryMapping(MemoryBlockAllocator* parent, std::uintptr_t from, std::uintptr_t to, bool writable)
			: parent(parent)
			, from(from)
			, to(to)
			, writable(writable)
		{
		}

		std::strong_ordering operator<=>(const MemoryMapping& other) const { return from <=> other.from; }
		void set_writable(bool new_writable);
		void gc(MemoryRegion* region);

		friend MemoryBlockAllocator;

		static constexpr size_t dist(std::size_t a, std::size_t b)
		{
			return std::max(a, b) - std::min(a, b);
		}

		[[nodiscard]] std::optional<std::pair<std::uintptr_t, std::size_t>> find_region_in_tolerance(
			std::uintptr_t location,
			std::size_t size,
			std::size_t tolerance) const
		{
			if (dist(from, location) > tolerance || dist(to, location) > tolerance)
				return std::nullopt;
			for (const bool reverse : { false, true }) {
				auto region = reverse ? find_region(size) : find_region_reverse(size);
				if (!region.has_value())
					continue;
				const std::size_t distance = dist(region.value(), location);
				if (distance > tolerance || dist(region.value() + size, location) > tolerance)
					continue;
				return { { region.value(), distance } };
			}
			return std::nullopt;
		}

		[[nodiscard]] std::unique_ptr<MemoryRegion> acquire_region(std::uintptr_t location, std::size_t size)
		{
			if (location < from || location + size > to)
				throw std::bad_alloc{};
			auto* region_ptr = new MemoryRegion{ location, location + size, this };
			used_regions.insert(region_ptr);
			return std::unique_ptr<MemoryRegion>{ region_ptr };
		}

	public:
		MemoryMapping() = delete;
		MemoryMapping(const MemoryMapping&) = delete;
		void operator=(const MemoryMapping&) = delete;

		[[nodiscard]] const MemoryBlockAllocator* get_parent() const { return parent; }
		[[nodiscard]] std::uintptr_t get_from() const { return from; }
		[[nodiscard]] std::uintptr_t get_to() const { return to; }
		[[nodiscard]] const auto& get_used_regions() const { return used_regions; }
		[[nodiscard]] bool is_writable() const { return writable; }
	};

	template <typename Func>
	concept AllocatorFunc = requires(const Func& f, std::uintptr_t address, std::size_t length, bool writable) {
		{ f(address, length, writable) } -> std::same_as<std::optional<std::uintptr_t>>;
	};

	class MemoryBlockAllocator {
		std::vector<std::unique_ptr<MemoryMapping>> mappings;
		std::size_t granularity;

		friend MemoryMapping;

		std::pair<decltype(mappings)::iterator, std::uintptr_t> find_closest(std::uintptr_t location, std::size_t size, std::size_t tolerance)
		{
			auto best = mappings.end();
			std::uintptr_t best_location = 0;
			std::uintptr_t best_distance = tolerance;

			for (auto it = mappings.begin(); it != mappings.end(); it++) {
				auto& mapping = *it;
				if (mapping->to - mapping->from < size)
					continue;
				auto region = mapping->find_region_in_tolerance(location, size, tolerance);
				if (!region.has_value())
					continue;
				auto [region_begin, distance] = region.value();
				if (distance < best_distance) {
					best = it;
					best_location = region_begin;
					best_distance = distance;
				}
			}
			return { best, best_location };
		}

		void gc(MemoryMapping* page)
		{
			auto it = std::ranges::find_if(mappings,
				[&](const std::unique_ptr<MemoryMapping>& other) {
					return other.get() == page;
				});
			if (it != mappings.end()) {
				deallocate_memory(page->from, page->to - page->from);
				mappings.erase(it);
			}
		}

		std::unique_ptr<MemoryMapping>& allocate_new_map(
			std::uintptr_t preferred_location,
			std::size_t size,
			bool writable = true,
			std::size_t tolerance = INT32_MAX)
		{
			// round up integer division
			const size_t num_pages = (size + granularity - 1) / granularity;

			const std::uintptr_t new_mem = find_unused_memory(preferred_location, tolerance, num_pages, writable);
			auto& new_region = mappings.emplace_back(
				new MemoryMapping{ this, new_mem, new_mem + num_pages * granularity, writable });
			return new_region;
		}

	protected:
		template <typename Func>
			requires AllocatorFunc<Func>
		static auto search(std::size_t granularity, const Func& func)
		{
			return [func, granularity](std::uintptr_t preferred_location, std::size_t tolerance, std::size_t num_pages, bool writable) {
				for (std::size_t offset = 0; offset < tolerance; offset += granularity)
					for (const bool positive : { false, true }) {
						std::uintptr_t address = preferred_location - preferred_location % granularity;
						if (positive)
							address += offset;
						else
							address -= offset;
						if (auto pointer = func(address, granularity * num_pages, writable); pointer.has_value())
							return pointer.value();
					}
				throw std::bad_alloc{};
			};
		}

	public:
		explicit MemoryBlockAllocator(
			// (Page size); The virtual functions are expected to also respect this granularity
			std::size_t granularity)
			: granularity(granularity)
		{
		}
		MemoryBlockAllocator() = delete;
		MemoryBlockAllocator(const MemoryBlockAllocator&) = delete; // Don't copy this around blindly
		void operator=(const MemoryBlockAllocator&) = delete;
		virtual ~MemoryBlockAllocator() = default;

		[[nodiscard]] const auto& get_mappings() const { return mappings; }

		[[nodiscard]] std::unique_ptr<MemoryRegion> get_region(
			std::uintptr_t preferred_location,
			std::size_t size,
			// @note: When set to false, writable pages are still returned, call MemoryMapping#set_writable(false) when an unreadable page is desired
			bool needs_writable = true,
			std::size_t tolerance = INT32_MAX)
		{
			if (size == 0)
				throw std::bad_alloc{};

			auto [iter, region_begin] = find_closest(preferred_location, size, tolerance);
			if (iter != mappings.end()) {
				auto& mapping = *iter;
				if (needs_writable && !mapping->is_writable())
					mapping->set_writable(needs_writable);
				return mapping->acquire_region(region_begin, size);
			}

			// Mhm, no luck, new memory needs to be allocated
			std::unique_ptr<MemoryMapping>& new_region = allocate_new_map(preferred_location, size, needs_writable, tolerance);

			auto region_bounds = new_region->find_region_in_tolerance(preferred_location, size, tolerance);
			if (!region_bounds.has_value()) // If this optional happens to be empty, then find_unused_memory didn't give a page inside the tolerance
				throw std::bad_alloc{};

			return new_region->acquire_region(region_bounds->first, size);
		}

		[[nodiscard]] std::size_t get_granularity() const
		{
			return granularity;
		}

		virtual std::uintptr_t find_unused_memory(std::uintptr_t preferred_location, std::size_t tolerance, std::size_t num_pages, bool writable) = 0;
		virtual void deallocate_memory(std::uintptr_t location, std::size_t size) = 0;
		virtual void change_protection(std::uintptr_t location, std::size_t size, bool new_writable) = 0;
	};

	// Definitions:
	inline MemoryRegion::~MemoryRegion()
	{
		parent->gc(this);
	}

	inline bool MemoryRegion::is_writable() const
	{
		return parent->is_writable();
	}

	inline void MemoryRegion::set_writable(bool writable)
	{
		parent->set_writable(writable);
	}

	inline void MemoryMapping::set_writable(bool new_writable)
	{
		if (this->writable == new_writable)
			return;

		parent->change_protection(from, to - from, new_writable);
		this->writable = new_writable;
	}

	inline void MemoryMapping::gc(MemoryRegion* region)
	{
		used_regions.erase(region);
		if (used_regions.empty()) {
			// This mapping is no longer needed.
			parent->gc(this);
		}
	}
}

#endif
