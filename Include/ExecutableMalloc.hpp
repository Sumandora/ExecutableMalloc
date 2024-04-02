#ifndef EXECUTABLEMALLOC_HPP
#define EXECUTABLEMALLOC_HPP

#include <cstdint>
#include <memory>
#include <set>
#include <vector>
#include <utility>
#include <compare>
#include <optional>
#include <functional>

namespace ExecutableMalloc {
	class MemoryBlockAllocator;
	class MemoryMapping;

	class MemoryRegion {
		std::uintptr_t from;
		std::uintptr_t to;
		MemoryMapping* parent;

		friend MemoryMapping;

		MemoryRegion(std::uintptr_t from, std::uintptr_t to, MemoryMapping* parent);

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

		std::strong_ordering operator<=>(const MemoryRegion& other) const;

		[[nodiscard]] std::uintptr_t getFrom() const;
		[[nodiscard]] std::uintptr_t getTo() const;
		[[nodiscard]] const MemoryMapping* getParent() const;

		[[nodiscard]] bool isWritable() const;
		void setWritable(bool writable);
		void resize(std::size_t size); // The new size needs to be smaller or equally big as the current region
	};

	class MemoryMapping {
		MemoryBlockAllocator* parent;
		std::uintptr_t from;
		std::uintptr_t to;
		std::set<MemoryRegion*, MemoryRegion::PtrOrder> usedRegions;
		bool writable;

		friend MemoryRegion;
		friend MemoryBlockAllocator;

		MemoryMapping(MemoryBlockAllocator* parent, std::uintptr_t from, std::uintptr_t to, bool writable);

		std::strong_ordering operator<=>(const MemoryMapping& other) const;
		template <bool Reverse>
		[[nodiscard]] std::optional<std::uintptr_t> findRegion(std::size_t size) const;
		[[nodiscard]] std::optional<std::pair<std::uintptr_t, std::size_t>> findRegionInTolerance(std::uintptr_t location, std::size_t size, std::size_t tolerance) const;
		void setWritable(bool newWritable);
		[[nodiscard]] std::unique_ptr<MemoryRegion> acquireRegion(std::uintptr_t location, std::size_t size);
		void gc(MemoryRegion* region);
	public:
		MemoryMapping() = delete;
		MemoryMapping(const MemoryMapping&) = delete;
		void operator=(const MemoryMapping&) = delete;

		[[nodiscard]] const MemoryBlockAllocator* getParent() const;
		[[nodiscard]] std::uintptr_t getFrom() const;
		[[nodiscard]] std::uintptr_t getTo() const;
		[[nodiscard]] const decltype(usedRegions)& getUsedRegions() const;
		[[nodiscard]] bool isWritable() const;
	};

	class MemoryBlockAllocator {
		std::vector<std::unique_ptr<MemoryMapping>> mappings;
		std::function<std::uintptr_t(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable)> findUnusedMemory;
		std::function<void(std::uintptr_t location, std::size_t size)> deallocateMemory;
		std::function<void(MemoryMapping& mapping, bool newWritable)> changePermissions;
		std::size_t granularity; // (Page size); The functions declared above are expected to also respect this granularity

		friend MemoryMapping;

		[[nodiscard]] std::optional<std::pair<std::reference_wrapper<std::unique_ptr<MemoryMapping>>, std::uintptr_t>> findClosest(std::uintptr_t location, std::size_t size, std::size_t tolerance);
		void gc(MemoryMapping* page);
	public:
		MemoryBlockAllocator(
			decltype(findUnusedMemory)&& findUnusedMemory,
			decltype(deallocateMemory)&& deallocateMemory,
			decltype(changePermissions)&& changePermissions,
			std::size_t granularity
		);
		MemoryBlockAllocator() = delete;
		MemoryBlockAllocator(const MemoryBlockAllocator&) = delete; // Don't copy this around blindly
		void operator=(const MemoryBlockAllocator&) = delete;
		virtual ~MemoryBlockAllocator();

		[[nodiscard]] const std::vector<std::unique_ptr<MemoryMapping>>& getMappings() const;

		/**
		 * Note that when searching for a non-writable memory page, you may get a writable memory page.
		 * To enforce non-writable memory pages, you may call setWritable yourself.
		 */
		[[nodiscard]] std::unique_ptr<MemoryRegion> getRegion(std::uintptr_t preferredLocation, std::size_t size, bool writable = true, std::size_t tolerance = INT32_MAX);
	};

}

#endif
