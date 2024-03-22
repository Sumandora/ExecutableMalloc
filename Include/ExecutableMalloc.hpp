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

	public:
		MemoryRegion(std::uintptr_t from, std::uintptr_t to, MemoryMapping* parent);
		MemoryRegion() = delete;
		MemoryRegion(const MemoryRegion&) = delete;
		~MemoryRegion();

		std::strong_ordering operator<=>(const MemoryRegion& other) const;

		[[nodiscard]] std::uintptr_t getFrom() const;
		[[nodiscard]] std::uintptr_t getTo() const;
		[[nodiscard]] const MemoryMapping* getParent() const;

		bool isWritable();
		void setWritable(bool writable);
	};

	class MemoryMapping {
		MemoryBlockAllocator* parent;
		std::uintptr_t from;
		std::uintptr_t to;
		std::set<MemoryRegion*> usedRegions;
		bool writable;

		friend MemoryRegion;
		friend MemoryBlockAllocator;

		[[nodiscard]] std::size_t distanceTo(std::uintptr_t address, std::size_t size) const;
		std::strong_ordering operator<=>(const MemoryMapping& other) const;
		[[nodiscard]] bool hasRegion(size_t size) const;
		void setWritable(bool newWritable);
		std::unique_ptr<MemoryRegion> acquireRegion(size_t size);
		void gc(MemoryRegion* region);

	public:
		MemoryMapping(MemoryBlockAllocator* parent, std::uintptr_t from, std::uintptr_t to, bool writable);

		[[nodiscard]] const MemoryBlockAllocator* getParent() const;
		[[nodiscard]] std::uintptr_t getFrom() const;
		[[nodiscard]] std::uintptr_t getTo() const;
		[[nodiscard]] std::set<MemoryRegion*> getUsedRegions() const;
		[[nodiscard]] bool isWritable() const;
	};

	class MemoryBlockAllocator {
		std::vector<std::unique_ptr<MemoryMapping>> mappings;
		std::function<std::uintptr_t(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable)> findUnusedMemory;
		std::function<void(std::uintptr_t location, std::size_t size)> deallocateMemory;
		std::size_t granularity; // (Page size); The functions declared above are expected to also respect this granularity

		friend MemoryMapping;

		[[nodiscard]] std::optional<std::reference_wrapper<std::unique_ptr<MemoryMapping>>> findClosest(std::uintptr_t location, std::size_t size, bool writable = true);
		std::unique_ptr<MemoryMapping>& getBlock(std::uintptr_t preferredLocation, std::size_t size, std::size_t tolerance = INT32_MAX, bool writable = true);
		void gc(MemoryMapping* page);
	public:
		MemoryBlockAllocator(
			decltype(findUnusedMemory)&& findUnusedMemory,
			decltype(deallocateMemory)&& deallocateMemory,
			std::size_t granularity
		);

		[[nodiscard]] const std::vector<std::unique_ptr<MemoryMapping>>& getMappings() const;

		std::shared_ptr<MemoryRegion> getRegion(std::uintptr_t preferredLocation, std::size_t size, std::size_t tolerance = INT32_MAX, bool writable = true);
	};

}

#endif