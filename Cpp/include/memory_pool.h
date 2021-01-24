/**
*
* Copyright (C) 2017-2019 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <ostream>
#include <vector>
#include <exception>
#include <typeinfo>

#include "Lockable.h"

#define OPS_MEMORY_POOL_INTEGRITY_CHECK

namespace ops {
	namespace memory_pools {

		// ====================================================================
		// Manager

		// Forward declaration
		class memory_pool_manager;

		// Helper node for linking memory_pool's together
		template <typename T>
		struct node {
			T* owner;
			node* prev;
			node* next;
			node(T* own) noexcept : owner(own), prev(nullptr), next(nullptr) {}
		};

		// Non-templated base class for memory_pools with the interface and needed functionality 
		// to connect pools to the manager and let the manager operate on pools
		class memory_pool_abs {
		private:
			node<memory_pool_abs> _node;
		protected:
			memory_pool_abs();
			virtual ~memory_pool_abs();

			void Log(const char* message, std::exception& e);

			friend class memory_pool_manager;
			virtual void PrintStat(std::ostream& os) = 0;
		};

		class memory_pool_logger
		{
		public:
			virtual void Log(const char* message, std::exception& e) = 0;
		};

		// Singleton manager
		class memory_pool_manager
		{
		public:
			static memory_pool_manager& Instance();

			void setLogger(memory_pool_logger* client) noexcept;

			int numPools() const noexcept { return _numPools; }

			// Print statistics from each memory_pool
			void PrintStat(std::ostream& os, bool skip_header = false);

		protected:
			friend class memory_pool_abs;

			void Add(node<memory_pool_abs>& nd);
			void Remove(node<memory_pool_abs>& nd);
			void Log(const char* message, std::exception& e);

		private:
			memory_pool_manager() noexcept;
			memory_pool_manager(memory_pool_manager const&) = delete;
			memory_pool_manager(memory_pool_manager&&) = delete;
			memory_pool_manager& operator=(memory_pool_manager const&) = delete;
			memory_pool_manager& operator=(memory_pool_manager&&) = delete;

			Lockable _mtx;
            node<memory_pool_abs> _root{ nullptr };
            int _numPools{ 0 };
            memory_pool_logger* _client{ nullptr };
		};

		// ====================================================================
		// Template params:
		//    T   Storage type
		//    S   Type to get name from in PrintStat()

		template<typename T, typename S>
		class memory_pool_base : public memory_pool_abs
		{
		protected:
			struct entry_t
			{
				// Place data first so entry_t address and data address are equal.
				T data;
				uint32_t marker;
				uint32_t inUse;
				entry_t* next;
			};
			entry_t* _storage;			// Pointer to allocated buffer
			entry_t* _storageLast;		// Pointer to last entry, used for integrity check
			entry_t* _freePtr;			// Pointer to first free entry, if any
			size_t _capacity;
			size_t _size;
			Lockable _mtx;
			static constexpr uint32_t marker_c = 0x5BD0DEAD;
			static constexpr uint32_t inUse_c  = 0x5BD0AAAA;
			static constexpr uint32_t free_c   = 0x5BD0DEAD;

			// Statistics
			size_t _minSize;
			size_t _numAllocated;

#ifdef OPS_MEMORY_POOL_INTEGRITY_CHECK
			void check_integrity(entry_t* ent)
			{
				if ((size_t)ent < (size_t)_storage) throw illegal_ref();
				if ((size_t)ent > (size_t)_storageLast) throw illegal_ref();
				if (ent->marker != marker_c) throw pool_corruption();
			}
#endif

			void PrintStat(std::ostream& os)
			{
				os << "[" << typeid(S).name() << "] ";
				os << 
					"Capacity: " << _capacity << 
					", Min: " << _minSize << 
					", Tot.Alloc: " << _numAllocated <<
					std::endl;
			}

		public:
			void* getEntry()
			{
				entry_t* ret = nullptr;
				{
					SafeLock lck(_mtx);
					ret = _freePtr;
					if (ret == nullptr) throw out_of_space();
#ifdef OPS_MEMORY_POOL_INTEGRITY_CHECK
					check_integrity(ret);
#endif
					_freePtr = ret->next;
					--_size;
					_numAllocated++;
					if (_minSize > _size) _minSize = _size;
					ret->inUse = inUse_c;
				}
				ret->next = nullptr;
				return &ret->data;
			}

			void returnEntry(void* data)
			{
				entry_t* ret = (entry_t*)(data);
#ifdef OPS_MEMORY_POOL_INTEGRITY_CHECK
				check_integrity(ret);
#endif
				if (ret->inUse != inUse_c) throw pool_corruption();
				SafeLock lck(_mtx);
				ret->next = _freePtr;
				_freePtr = ret;
				++_size;
				ret->inUse = free_c;
			}

		public:
			// Exceptions:
			struct out_of_space : public std::exception
			{
				const char* what() const noexcept override { return "Out of Space"; }
			};
			struct pool_corruption : public std::exception
			{
				const char* what() const noexcept override { return "Pool integrity check failed"; }
			};
			struct illegal_ref : public std::exception
			{
				const char* what() const noexcept override { return "Reference not from this pool"; }
			};

			memory_pool_base(size_t capacity) : _capacity(capacity), _size(capacity), _minSize(capacity), _numAllocated(0)
			{
				if (_capacity < 1) throw out_of_space();
				_storage = new entry_t[_capacity];
				// Link all entries together
				_freePtr = &_storage[0];
				_storage[0].marker = marker_c;
				_storage[0].inUse = free_c;
				for (size_t i = 1; i < _capacity; ++i) {
					_storage[i - 1].next = &_storage[i];
					_storage[i].marker = marker_c;
					_storage[i].inUse = free_c;
				}
				_storage[_capacity - 1].next = nullptr;
				_storageLast = &_storage[_capacity - 1];
			}

			virtual ~memory_pool_base()
			{
				delete[] _storage;
			}

			size_t capacity() const { return _capacity; }
			size_t size() const { return _size; }

		private:
			memory_pool_base() = delete;
			memory_pool_base(memory_pool_base const&) = delete;
			memory_pool_base(memory_pool_base&&) = delete;
			memory_pool_base& operator=(memory_pool_base const&) = default;
			memory_pool_base& operator=(memory_pool_base&&) = default;
		};

		// ====================================================================

		// A Memory Pool for objects that have a default constructor.
		// All objects are created when the memory_pool() is created.
		// A returned object retains its values and will NOT be cleared or reinitialized.
		// A fetched object will contain the old values and may need to be initialized.
		// All objects exists until the pool is deleted.
		// Objects fetched from the pool must NOT be deleted explicitly, they must be returned to the pool.
		template<typename T>
		class memory_pool : public memory_pool_base<T, T>
		{
		public:
			typedef memory_pool_base<T, T> BaseClass;

			memory_pool(size_t capacity) : BaseClass(capacity) {}

			inline T* getEntry()
			{
				return (T*)BaseClass::getEntry();
			}

			inline void returnEntry(T*& data)
			{
				BaseClass::returnEntry(data);
				data = nullptr;
			}

		private:
			memory_pool() = delete;
			memory_pool(memory_pool const&) = delete;
			memory_pool(memory_pool&&) = delete;
			memory_pool& operator=(memory_pool const&) = default;
			memory_pool& operator=(memory_pool&&) = default;
		};

		// ====================================================================

		// Helper used for space allocation for an object without declaring the actual type.
		// If we use the actual type, the constructor would be called which we don't want.
		template<typename T>
		struct memory_pool_data
		{
			uint8_t data[sizeof(T)];
		};

		// A Memory Pool to be used from an objects new and delete operators.
		// Memory is not cleared, but since the pool is called from the object during
		// creation, the objects constructor will initialize the memory.
		template<typename T>
		class memory_pool_nd : public memory_pool_base<memory_pool_data<T>, T>
		{
		public:
			typedef memory_pool_base<memory_pool_data<T>, T> BaseClass;

			// Exceptions:
			struct illegal_size : public std::exception
			{
				const char* what() const noexcept override { return "Illegal size in allocation request"; }
			};

			memory_pool_nd(size_t capacity) : BaseClass(capacity) {}

			inline void* getEntry(size_t size)
			{
				if (size != sizeof(T)) throw illegal_size();
				return BaseClass::getEntry();
			}

			inline void returnEntry(void* data)
			{
				// No good idea to throw an exception from here since it is called inside operator delete.
				// It will most likely terminate the program.
				// Instead we Log the exception and store it so it can be checked.
				try {
					BaseClass::returnEntry(data);
				}
				catch (std::exception& e) {
					BaseClass::Log(typeid(T).name(), e);
					_ep = std::current_exception();
				}
			}

			void checkException()
			{
				if (_ep) {
					try {
						std::rethrow_exception(_ep);
					} catch (...) {
						_ep = nullptr;
						throw;
					}
				}
			}

		private:
			memory_pool_nd() = delete;
			memory_pool_nd(memory_pool_nd const&) = delete;
			memory_pool_nd(memory_pool_nd&&) = delete;
			memory_pool_nd& operator=(memory_pool_nd const&) = default;
			memory_pool_nd& operator=(memory_pool_nd&&) = default;
			std::exception_ptr _ep;
		};

		// ====================================================================

		// A Memory Pool for fixed size buffers, that take an initial number of blocks and then
		// grows if all blocks are used.
		template <int block_size>
		class memory_pool_exp : public memory_pool_abs
		{
			std::vector<char *> _blocks;
			Lockable _mtx;

			void PrintStat(std::ostream& os) override
			{
				os << "[" << typeid(this).name() << "] ";
				os <<
					"Capacity: " << _blocks.capacity() <<
					", Current: " << _blocks.size() <<
					std::endl;
			}

		public:
			struct illegal_ref : public std::exception
			{
				const char* what() const noexcept override { return "Illegal reference"; }
			};

			memory_pool_exp(int initial_num_blocks)
			{
				_blocks.reserve(initial_num_blocks);
				for (auto i = 0; i < initial_num_blocks; ++i) {
					_blocks.push_back(new char[block_size]);
				}
			}

			~memory_pool_exp()
			{
				for (std::size_t i = 0; i < _blocks.size(); ++i) {
					delete[] _blocks[i];
				}
			}

			inline char* getEntry()
			{
				char* ptr;
				SafeLock lck(_mtx);
				if (_blocks.size() > 0) {
					ptr = _blocks.back();
					_blocks.pop_back();
				} else {
					ptr = new char[block_size];
				}
				return ptr;
			}

			inline void returnEntry(char*& ptr)
			{
				if (ptr == nullptr) throw illegal_ref();

				SafeLock lck(_mtx);
				_blocks.push_back(ptr);
				ptr = nullptr;
			}

			size_t size() const { return _blocks.size(); }
			size_t capacity() const { return _blocks.capacity(); }

		private:
			memory_pool_exp() = delete;
			memory_pool_exp(memory_pool_exp const&) = delete;
			memory_pool_exp(memory_pool_exp&&) = delete;
			memory_pool_exp& operator=(memory_pool_exp const&) = default;
			memory_pool_exp& operator=(memory_pool_exp&&) = default;
		};

	}
} // namespace
