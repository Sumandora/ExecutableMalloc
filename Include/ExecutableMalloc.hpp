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
				return std::less{}(lhs->getFrom(), rhs->getFrom());
			}
		};

	public:
		MemoryRegion() = delete;
		MemoryRegion(const MemoryRegion&) = delete;
		void operator=(const MemoryRegion&) = delete;
		~MemoryRegion();

		std::strong_ordering operator<=>(const MemoryRegion& other) const { return from <=> other.from; }

		[[nodiscard]] std::uintptr_t getFrom() const { return from; }
		[[nodiscard]] std::uintptr_t getTo() const { return to; }
		[[nodiscard]] const MemoryMapping* getParent() const { return parent; }

		[[nodiscard]] bool isWritable() const;
		void setWritable(bool writable);
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
		std::set<MemoryRegion*, MemoryRegion::PtrOrder> usedRegions;
		bool writable;

		[[nodiscard]] std::optional<std::uintptr_t> findRegion(std::size_t size) const
		{
			std::uintptr_t p = from;
			for (const auto& region : usedRegions) {
				if (region->getFrom() - p >= size)
					return p;
				p = region->getTo();
			}
			if (to - p >= size)
				return p;
			return std::nullopt;
		}

		[[nodiscard]] std::optional<std::uintptr_t> findRegionReverse(std::size_t size) const
		{
			std::uintptr_t p = to;
			for (const auto& region : usedRegions | std::ranges::views::reverse) {
				if (p - region->getTo() >= size)
					return p - size;
				p = region->getFrom();
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
		void setWritable(bool newWritable);
		void gc(MemoryRegion* region);

		friend MemoryBlockAllocator;

		static constexpr size_t dist(std::size_t a, std::size_t b)
		{
			return std::max(a, b) - std::min(a, b);
		}

		[[nodiscard]] std::optional<std::pair<std::uintptr_t, std::size_t>> findRegionInTolerance(
			std::uintptr_t location,
			std::size_t size,
			std::size_t tolerance) const
		{
			if (dist(from, location) > tolerance || dist(to, location) > tolerance)
				return std::nullopt;
			for (const bool reverse : { false, true }) {
				auto region = reverse ? findRegion(size) : findRegionReverse(size);
				if (!region.has_value())
					continue;
				const std::size_t distance = dist(region.value(), location);
				if (distance > tolerance || dist(region.value() + size, location) > tolerance)
					continue;
				return { { region.value(), distance } };
			}
			return std::nullopt;
		}

		[[nodiscard]] std::unique_ptr<MemoryRegion> acquireRegion(std::uintptr_t location, std::size_t size)
		{
			if (location < from || location + size > to)
				throw std::bad_alloc{};
			auto* regionPtr = new MemoryRegion{ location, location + size, this };
			usedRegions.insert(regionPtr);
			return std::unique_ptr<MemoryRegion>{ regionPtr };
		}

	public:
		MemoryMapping() = delete;
		MemoryMapping(const MemoryMapping&) = delete;
		void operator=(const MemoryMapping&) = delete;

		[[nodiscard]] const MemoryBlockAllocator* getParent() const { return parent; }
		[[nodiscard]] std::uintptr_t getFrom() const { return from; }
		[[nodiscard]] std::uintptr_t getTo() const { return to; }
		[[nodiscard]] const auto& getUsedRegions() const { return usedRegions; }
		[[nodiscard]] bool isWritable() const { return writable; }
	};

	template <typename Func>
	concept AllocatorFunc = requires(const Func& f, std::uintptr_t address, std::size_t length, bool writable) {
		{ f(address, length, writable) } -> std::same_as<std::optional<std::uintptr_t>>;
	};

	class MemoryBlockAllocator {
		std::vector<std::unique_ptr<MemoryMapping>> mappings;
		std::function<std::uintptr_t(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable)> findUnusedMemory;
		std::function<void(std::uintptr_t location, std::size_t size)> deallocateMemory;
		std::function<void(std::uintptr_t location, std::size_t size, bool newWritable)> changeProtection;
		std::size_t granularity; // (Page size); The functions declared above are expected to also respect this granularity

		friend MemoryMapping;

		std::pair<decltype(mappings)::iterator, std::uintptr_t> findClosest(std::uintptr_t location, std::size_t size, std::size_t tolerance)
		{
			auto best = mappings.end();
			std::uintptr_t bestLocation = 0;
			std::uintptr_t bestDistance = tolerance;

			for (auto it = mappings.begin(); it != mappings.end(); it++) {
				auto& mapping = *it;
				if (mapping->to - mapping->from < size)
					continue;
				auto region = mapping->findRegionInTolerance(location, size, tolerance);
				if (!region.has_value())
					continue;
				auto [regionBegin, distance] = region.value();
				if (distance < bestDistance) {
					best = it;
					bestLocation = regionBegin;
					bestDistance = distance;
				}
			}
			return { best, bestLocation };
		}

		void gc(MemoryMapping* page)
		{
			auto it = std::ranges::find_if(mappings,
				[&](const std::unique_ptr<MemoryMapping>& other) {
					return other.get() == page;
				});
			if (it != mappings.end()) {
				deallocateMemory(page->from, page->to - page->from);
				mappings.erase(it);
			}
		}

		std::unique_ptr<MemoryMapping>& allocateNewMap(
			std::uintptr_t preferredLocation,
			std::size_t size,
			bool writable = true,
			std::size_t tolerance = INT32_MAX)
		{
			// round up integer division
			const size_t numPages = (size + granularity - 1) / granularity;

			const std::uintptr_t newMem = findUnusedMemory(preferredLocation, tolerance, numPages, writable);
			auto& newRegion = mappings.emplace_back(
				new MemoryMapping{ this, newMem, newMem + numPages * granularity, writable });
			return newRegion;
		}

	protected:
		template <typename Func>
			requires AllocatorFunc<Func>
		static auto search(std::size_t granularity, const Func& func)
		{
			return [func, granularity](std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable) {
				for (std::size_t offset = 0; offset < tolerance; offset += granularity)
					for (const bool positive : { false, true }) {
						std::uintptr_t address = preferredLocation - preferredLocation % granularity;
						if (positive)
							address += offset;
						else
							address -= offset;
						if (auto pointer = func(address, granularity * numPages, writable); pointer.has_value())
							return pointer.value();
					}
				throw std::bad_alloc{};
			};
		}

	public:
		MemoryBlockAllocator(
			decltype(findUnusedMemory)&& findUnusedMemory,
			decltype(deallocateMemory)&& deallocateMemory,
			decltype(changeProtection)&& changePermissions,
			std::size_t granularity)
			: findUnusedMemory(std::move(findUnusedMemory))
			, deallocateMemory(std::move(deallocateMemory))
			, changeProtection(std::move(changePermissions))
			, granularity(granularity)
		{
		}
		MemoryBlockAllocator() = delete;
		MemoryBlockAllocator(const MemoryBlockAllocator&) = delete; // Don't copy this around blindly
		void operator=(const MemoryBlockAllocator&) = delete;
		virtual ~MemoryBlockAllocator() = default;

		[[nodiscard]] const auto& getMappings() const { return mappings; }

		[[nodiscard]] std::unique_ptr<MemoryRegion> getRegion(
			std::uintptr_t preferredLocation,
			std::size_t size,
			// @note: When set to false, writable pages are still returned, call MemoryMapping#setWritable(false) when a unreadable page is desired
			bool needsWritable = true,
			std::size_t tolerance = INT32_MAX)
		{
			if (size == 0)
				throw std::bad_alloc{};

			auto [iter, regionBegin] = findClosest(preferredLocation, size, tolerance);
			if (iter != mappings.end()) {
				auto& mapping = *iter;
				if (needsWritable && !mapping->isWritable())
					mapping->setWritable(needsWritable);
				return mapping->acquireRegion(regionBegin, size);
			}

			// Mhm, no luck, new memory needs to be allocated
			std::unique_ptr<MemoryMapping>& newRegion = allocateNewMap(preferredLocation, size, needsWritable, tolerance);

			auto regionBounds = newRegion->findRegionInTolerance(preferredLocation, size, tolerance);
			if (!regionBounds.has_value()) // If this optional happens to be empty then findUnusedMemory didn't give a page inside the tolerance
				throw std::bad_alloc{};

			return newRegion->acquireRegion(regionBounds->first, size);
		}

		[[nodiscard]] std::size_t getGranularity() const
		{
			return granularity;
		}
	};

	// Definitions:
	inline MemoryRegion::~MemoryRegion()
	{
		parent->gc(this);
	}

	inline bool MemoryRegion::isWritable() const
	{
		return parent->isWritable();
	}

	inline void MemoryRegion::setWritable(bool writable)
	{
		parent->setWritable(writable);
	}

	inline void MemoryMapping::setWritable(bool newWritable)
	{
		if (this->writable == newWritable)
			return;

		parent->changeProtection(from, to - from, newWritable);
		this->writable = newWritable;
	}

	inline void MemoryMapping::gc(MemoryRegion* region)
	{
		usedRegions.erase(region);
		if (usedRegions.empty()) {
			// This mapping is no longer needed.
			parent->gc(this);
		}
	}
}

#endif
