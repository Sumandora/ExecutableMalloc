#ifndef EXECUTABLEMALLOC_HPP
#define EXECUTABLEMALLOC_HPP

#include <cstdint>
#include <memory>
#include <set>
#include <vector>
#include <utility>
#include <compare>

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

	public:
		MemoryMapping(MemoryBlockAllocator* parent, std::uintptr_t from, std::uintptr_t to, bool writable);

		[[nodiscard]] const MemoryBlockAllocator* getParent() const;
		[[nodiscard]] std::uintptr_t getFrom() const;
		[[nodiscard]] std::uintptr_t getTo() const;
		[[nodiscard]] std::set<MemoryRegion*> getUsedRegions() const;
		[[nodiscard]] bool isWritable() const;

		[[nodiscard]] std::size_t distanceTo(std::uintptr_t address, std::size_t size) const;
		std::strong_ordering operator<=>(const MemoryMapping& other) const;
		[[nodiscard]] bool hasRegion(size_t size) const;
		void setWritable(bool newWritable);
		std::unique_ptr<MemoryRegion> acquireRegion(size_t size);
		void gc(MemoryRegion* region);
	};

	class MemoryBlockAllocator {
		std::vector<std::unique_ptr<MemoryMapping>> mappings;

	public:
		static void* findUnusedMemory(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages);

		const std::vector<std::unique_ptr<MemoryMapping>>& getMappings() const;

		[[nodiscard]] decltype(mappings)::pointer findClosest(std::uintptr_t location, std::size_t size);
		std::unique_ptr<MemoryMapping>& getBlock(std::uintptr_t preferredLocation, std::size_t size, std::size_t tolerance = INT32_MAX);
		std::shared_ptr<MemoryRegion> getRegion(std::uintptr_t preferredLocation, std::size_t size, std::size_t tolerance = INT32_MAX);
		void gc(MemoryMapping* page);
	};

}

#endif
