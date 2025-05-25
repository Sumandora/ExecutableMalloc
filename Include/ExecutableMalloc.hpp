#ifndef EXECUTABLEMALLOC_HPP
#define EXECUTABLEMALLOC_HPP

#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <optional>
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

		MemoryRegion(std::uintptr_t from, std::uintptr_t to, MemoryMapping* parent);

		struct PtrOrder {
			bool operator()(const MemoryRegion* lhs, const MemoryRegion* rhs) const
			{
				return lhs->get_from() < rhs->get_from();
			}
		};

	public:
		MemoryRegion() = delete;
		MemoryRegion(const MemoryRegion&) = delete;
		void operator=(const MemoryRegion&) = delete;
		~MemoryRegion();

		auto operator<=>(const MemoryRegion& other) const { return from <=> other.from; }

		[[nodiscard]] std::uintptr_t get_from() const { return from; }
		[[nodiscard]] std::uintptr_t get_to() const { return to; }
		[[nodiscard]] const MemoryMapping* get_parent() const { return parent; }

		[[nodiscard]] bool is_writable() const;
		void set_writable(bool writable);
		void resize(std::size_t size); // The new size needs to be smaller or equally big as the current region
	};

	class MemoryMapping {
		MemoryBlockAllocator* parent;
		std::uintptr_t from;
		std::uintptr_t to;
		std::set<MemoryRegion*, MemoryRegion::PtrOrder> used_regions;
		bool writable;

		[[nodiscard]] std::optional<std::uintptr_t> find_region(std::size_t size) const;
		[[nodiscard]] std::optional<std::uintptr_t> find_region_reverse(std::size_t size) const;

		friend MemoryRegion;

		MemoryMapping(MemoryBlockAllocator* parent, std::uintptr_t from, std::uintptr_t to, bool writable);

		std::strong_ordering operator<=>(const MemoryMapping& other) const { return from <=> other.from; }
		void set_writable(bool new_writable);
		void gc(MemoryRegion* region);

		friend MemoryBlockAllocator;

		[[nodiscard]] std::optional<std::pair<std::uintptr_t, std::size_t>> find_region_in_tolerance(
			std::uintptr_t location,
			std::size_t size,
			std::size_t tolerance) const;
		[[nodiscard]] std::unique_ptr<MemoryRegion> acquire_region(std::uintptr_t location, std::size_t size);

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

		std::pair<decltype(mappings)::iterator, std::uintptr_t> find_closest(
			std::uintptr_t location,
			std::size_t size,
			std::size_t tolerance);

		void gc(MemoryMapping* page);

		std::unique_ptr<MemoryMapping>& allocate_new_map(
			std::uintptr_t preferred_location,
			std::size_t size,
			bool writable = true,
			std::size_t tolerance = INT32_MAX);

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
			std::size_t granularity);

		MemoryBlockAllocator() = delete;
		MemoryBlockAllocator(const MemoryBlockAllocator&) = delete; // Don't copy this around blindly
		void operator=(const MemoryBlockAllocator&) = delete;
		virtual ~MemoryBlockAllocator();

		[[nodiscard]] const auto& get_mappings() const { return mappings; }

		[[nodiscard]] std::unique_ptr<MemoryRegion> get_region(
			std::uintptr_t preferred_location,
			std::size_t size,
			// @note: When set to false, writable pages are still returned, call MemoryMapping#set_writable(false) when an unreadable page is desired
			bool needs_writable = true,
			std::size_t tolerance = INT32_MAX);

		[[nodiscard]] std::size_t get_granularity() const { return granularity; }

		virtual std::uintptr_t find_unused_memory(std::uintptr_t preferred_location, std::size_t tolerance, std::size_t num_pages, bool writable) = 0;
		virtual void deallocate_memory(std::uintptr_t location, std::size_t size) = 0;
		virtual void change_protection(std::uintptr_t location, std::size_t size, bool new_writable) = 0;
	};
}

#endif
