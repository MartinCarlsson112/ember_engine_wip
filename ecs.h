#pragma once
#include <assert.h> 
#include <string.h> 
#include <stdlib.h> 
#include <algorithm>
#include <stdint.h> 

typedef size_t size_type;

struct component_info
{
	size_type id;
	size_type typeSize;
};

inline size_type componentIdGen = 0;
template<typename T> inline const size_type component_id = componentIdGen++;


template <typename T>
component_info get_component_info() { return { component_id<T>, sizeof(T) }; }


template <typename... Ts>
static typename std::enable_if<sizeof...(Ts) == 0>::type get_component_infos_a(component_info* arr, size_t index) { }

template <typename T, typename... Ts>
static void get_component_infos_a(component_info* arr, size_t index) {
	arr[index] = get_component_info<T>();
	get_component_infos_a<Ts...>(arr, index+1);
}

template <typename ...Ts>
constexpr void get_component_infos(component_info* arr)
{
	get_component_infos_a<Ts...>(arr, 0);
}

struct archetype_descriptor
{
	component_info* arr;
	size_t size;
};

template<typename ...T>
struct archetype {
	component_info arr[sizeof...(T)];
	size_t size = sizeof...(T);
	constexpr archetype() :arr() {
		get_component_infos<T...>(arr);
	}

	constexpr archetype_descriptor descriptor()
	{
		return archetype_descriptor{ arr, size };
	}

};


template <typename... Ts>
static typename std::enable_if<sizeof...(Ts) == 0>::type get_component_ids_a(size_type* arr, size_t index) { }

template <typename T, typename... Ts>
static void get_component_ids_a(size_type* arr, size_t index) {
	arr[index] = component_id<T>;
	get_component_ids_a<Ts...>(arr, index + 1);
}

template <typename ...Ts>
constexpr void get_component_ids(size_type* arr)
{
	get_component_ids_a<Ts...>(arr, 0);
}

template <typename ...T>
struct component_id_array
{
	size_type arr[sizeof...(T)];
	size_type size = sizeof...(T);
	constexpr component_id_array() :arr() {
		get_component_ids<T...>(arr);
	}
};


constexpr float GROWTH_FACTOR = 1.5f;
constexpr size_type NOT_INIT = UINT64_MAX;

struct entity
{
	constexpr entity() : id(0), _groupId(0) {}
	explicit entity(uint64_t id) : id(id), _groupId(0) {}
	entity(uint32_t index, uint32_t version) : id((uint64_t)version << 32 | index), _groupId(0) {}

	uint32_t index() const { return id & 0xffffffffUL; }
	uint32_t version() const { return id >> 32; }
	bool is_active(const entity& e) const { return e.version() == version(); }

	void increment_version() {
		uint32_t version = (id >> 32) + 1;
		id = (uint64_t)version << 32 | index();
	}
	uint64_t id;
	uint16_t _groupId;
};
struct sparse_entity_array
{
	constexpr sparse_entity_array() : entityToIndex(nullptr), _size(0), _allocated(0) {}

	uint32_t* entityToIndex;
	size_type _size;
	size_type _allocated;

	uint32_t& operator[](size_type i) const { return entityToIndex[i]; }
	uint32_t& operator[](const uint32_t& i) const { return entityToIndex[i]; }

	void push_back(const uint32_t entityIndex)
	{
		if (_allocated == _size)
		{
			size_type newSize = (_allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR));
			uint32_t* temp = (uint32_t*)calloc(newSize, sizeof(uint32_t));

			if (temp)
			{
				if (entityToIndex) {
					memcpy(temp, entityToIndex, _allocated * sizeof(uint32_t));
					free(entityToIndex);
				}
				entityToIndex = temp;
				_allocated = newSize;
			}
			assert(temp != nullptr);
		}
		entityToIndex[_size] = entityIndex;
		_size++;
	}

	void dispose()
	{
		free(entityToIndex);
		entityToIndex = nullptr;
		_allocated = 0;
		_size = 0;
	}
};

struct entity_free_list
{
	constexpr entity_free_list() : entities(nullptr), _size(0), _allocated(0), _frontIndex(0) {}
	entity* entities;
	size_type _size;
	size_type _allocated;
	size_type _frontIndex;

	void push(const entity& e)
	{
		//check if front index is > 0
			//if so move the array instead of allocating new stuff
			//todo: there is probably some more efficient strategy here. 
				//potentially we end up repeatedly moving the array every time we pop and push, which may or may not be efficient depending on the size of the list
				//this would only happen in some specific situation ex. when the count of elements swaps between _allocated-1 and _allocated
		if (_allocated == _size - _frontIndex)
		{
			size_type count = _size;
			for (size_type i = _frontIndex; i < count; i++) {
				entities[i - _frontIndex] = entities[i];
			}
			_size -= _frontIndex;
			_frontIndex = 0;
		}
		else if (_allocated == _size)
		{
			size_type newSize = _allocated == 0U ? 1U : (size_type)ceil((double)_allocated * GROWTH_FACTOR);
			entity* temp = (entity*)calloc(newSize, sizeof(entity));

			if (temp)
			{
				if (entities != nullptr) {
					memcpy(temp, entities, _allocated * sizeof(entity));
					free(entities);
				}

				entities = temp;
				_allocated = newSize;
			}
			assert(temp != nullptr);
		}
		entities[_size] = e;
		_size++;
	}

	entity& pop()
	{
		_frontIndex++;
		return entities[_frontIndex - 1];
	}

	bool empty() const
	{
		return _size - _frontIndex == 0;
	}

	void dispose()
	{
		free(entities);
		entities = nullptr;
		_allocated = 0;
		_size = 0;
		_frontIndex = 0;
	}

};

struct dense_entity_array
{
	constexpr dense_entity_array() : entities(nullptr), _allocated(0U), _size(0U) { }
	entity& operator[](const size_type& i) const
	{
		return entities[i];
	}

	entity& back()
	{
		return entities[_size - 1];
	}

	void push_back(const entity& e)
	{
		if (_allocated == _size)
		{

			size_type newSize = (_allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR));
			entity* temp = (entity*)calloc(newSize, sizeof(entity));

			if (temp)
			{
				if (entities != nullptr)
				{
					memcpy(temp, entities, _allocated * sizeof(entity));
					free(entities);
				}
				entities = temp;
				_allocated = newSize;
			}
			assert(temp != nullptr);
		}
		entities[_size] = e;
		_size++;
	}

	void pop_back()
	{
		_size--;
	}

	size_type size()
	{
		return _size;
	}

	void dispose()
	{
		free(entities);
		entities = nullptr;
		_allocated = 0;
		_size = 0;
	}
	entity* entities;
private:
	size_type _allocated;
	size_type _size;
};


struct entity_manager
{
	constexpr entity_manager() : counter(0) { }

	bool is_active(const entity& e) const
	{
		return dense[sparse[e.index()]].is_active(e);
	}

	entity create_entity(uint16_t _groupId)
	{
		if (freeList.empty())
		{
			uint32_t index = counter;
			counter++;
			entity e = entity(index, 0);
			e._groupId = _groupId;
			dense.push_back(e);
			sparse.push_back((uint32_t)(dense.size() - 1));
			return e;
		}

		entity e = freeList.pop();
		dense.push_back(e);
		sparse[e.index()] = (uint32_t)(dense.size());
		return e;
	}

	void remove_entity(entity e)
	{
		uint32_t last = dense[dense.size() - 1].index();
		std::swap(dense[sparse[e.index()]], dense.back());
		sparse[last] = sparse[e.index()];
		sparse[e.index()] = 0;
		e.increment_version();
		freeList.push(e);
		dense.pop_back();
	}

	uint32_t get(const entity& e) const
	{
		assert(e.index() < sparse._size);
		return sparse[e.index()];
	}

	void dispose()
	{
		sparse.dispose();
		dense.dispose();
		freeList.dispose();
	}

	uint32_t counter;
	sparse_entity_array sparse;
	dense_entity_array dense;
	entity_free_list freeList;
};

struct group_offset_array
{
	constexpr group_offset_array() : offsets(nullptr), _size(0) {}
	size_type* offsets;
	size_type _size;

	size_type size() const
	{
		return _size;
	}
	size_type& operator[](const size_type i) const
	{
		return offsets[i];
	}
	size_type& operator[](const int i) const
	{
		return offsets[i];
	}

	void resize(size_type newsize)
	{
		size_type* temp = (size_type*)calloc(newsize, sizeof(size_type));
		if (temp)
		{
			memset(temp, (int)NOT_INIT, newsize * sizeof(size_type));
			if (offsets != nullptr)
			{
				memcpy(temp, offsets, _size * sizeof(size_type));
				free(offsets);
			}
			offsets = temp;
			_size = newsize;
		}
		assert(temp != nullptr);
	}

	void dispose()
	{
		free(offsets);
		offsets = nullptr;
		_size = 0;
	}
};

struct group_offset_pair
{
	size_type componentId;
	size_type offset;
};

struct group_offset_pair_array {
	group_offset_pair* _groupOffsetPairs = nullptr;
	size_type _size = 0;
	size_type _allocated = 0;

	const size_type size() const { return _size; }

	group_offset_pair operator[](int i) const
	{
		return _groupOffsetPairs[i];
	}

	void push(group_offset_pair pair)
	{
		if (_allocated == _size)
		{

			size_type newSize = (_allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR));
			group_offset_pair* temp = (group_offset_pair*)calloc(newSize, sizeof(group_offset_pair));

			if (temp)
			{
				if (_groupOffsetPairs) {
					memcpy(temp, _groupOffsetPairs, _allocated * sizeof(group_offset_pair));
					free(_groupOffsetPairs);
				}
				_groupOffsetPairs = temp;
				_allocated = newSize;
			}
			assert(temp != nullptr);
		}
		_groupOffsetPairs[_size] = pair;
		_size++;
	}


	void dispose()
	{
		if (_allocated > 0)
		{
			_size = 0;
			free(_groupOffsetPairs);
			_groupOffsetPairs = nullptr;
			_allocated = 0;
		}
	}


};
struct component_free_list_pair
{
	size_type start;
	size_type size;
};

struct component_free_list
{
	constexpr component_free_list() : freeArray(nullptr), _size(0), _allocated(0) {}
	component_free_list_pair* freeArray;
	size_type _size;
	size_type _allocated;

	void push(const component_free_list_pair& e)
	{
		if (_allocated == _size)
		{
			size_type newSize = (_allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR));
			component_free_list_pair* temp = (component_free_list_pair*)calloc(newSize, sizeof(component_free_list_pair));

			if (temp)
			{
				if (freeArray != nullptr) {
					memcpy(temp, freeArray, _allocated * sizeof(component_free_list_pair));
					free(freeArray);
				}

				freeArray = temp;
				_allocated = newSize;
			}
			assert(temp != nullptr);
		}
		freeArray[_size] = e;
		_size++;
	}

	void dispose()
	{
		free(freeArray);
		freeArray = nullptr;
		_size = 0;
		_allocated = 0;
	}

	component_free_list_pair operator[](const size_type t) const
	{
		return freeArray[t];
	}

	void remove_at(const size_t index)
	{
		std::swap(freeArray[index], freeArray[_size]);
		_size--;
	}


	bool empty() const
	{
		return _size == 0;
	}
};

struct component_array
{
	component_array(const size_type typeSize) : _data(nullptr), _size(0), _allocated(0), _typeSize(typeSize) {}
	component_array() : _data(nullptr), _size(0), _allocated(0), _typeSize(0) { }

	char* _data;
	size_type _size;
	size_type _allocated;
	size_type _typeSize;
	component_free_list freeList;

	size_type get_or_create_chunk(const size_type size)
	{
		if (!freeList.empty())
		{
			//iterate free list and check if there exists a hole which we can use
			size_type count = freeList._size;
			for (size_type i = 0; i < count; i++)
			{
				if (freeList[i].size >= size)
				{
					//remove this from the free chunk
					size_type start = freeList[i].start;
					size_type extraSize = freeList[i].size - size;
					freeList.remove_at(i);
					if (extraSize != 0)
					{
						//make a new free zone with the remainder
						return_chunk(start + (size * _typeSize), extraSize);
					}
					return start;
				}
			}
		}
		else if (!(_size + size < _allocated))
		{
			size_type newSize = _size + size;
			char* temp = (char*)calloc(newSize, _typeSize);

			if (temp)
			{
				if (_data != nullptr) {
					memcpy(temp, _data, _allocated * _typeSize);
					free(_data);
				}
				_data = temp;
				_allocated = newSize;
			}
			assert(temp != nullptr);
		}

		size_type start = _size;
		_size += size;
		return start;
	}

	void return_chunk(const size_type start, const size_type size)
	{
		freeList.push(component_free_list_pair{ start, size });
	}

	void dispose()
	{
		freeList.dispose();
		free(_data);
		_data = nullptr;
		_size = 0;
		_allocated = 0;
		_typeSize = 0;
	}

};

struct group
{

	group(const group_offset_pair_array& offsets, size_type groupSize, size_type groupId)
	{
		_size = groupSize;
		_groupId = groupId;
		const size_type size = offsets.size();
		_nComponents = size;
		groupOffsets = (group_offset_array*)calloc(1, sizeof(group_offset_array));
		for (int i = 0; i < size; i++)
		{
			if (!(offsets[i].componentId < groupOffsets->size()))
			{
				groupOffsets->resize(offsets[i].componentId + 1U);
			}
			groupOffsets->operator[](offsets[i].componentId) = offsets[i].offset;
		}

		em = (entity_manager*)calloc(1, sizeof(entity_manager));



	}


	struct group_iterator
	{
		group_iterator(size_type index = 0) : ptr(index) {}
		group_iterator operator++() { ptr++; return *this; }
		bool operator!=(const group_iterator& other) const { return ptr != other.ptr; }
		const size_type& operator*() const { return ptr; }
	private:
		size_type ptr;
	};
	group_iterator begin() const { return group_iterator(); }
	group_iterator end() const { return group_iterator(em->counter); }



	entity create_entity()
	{
		assert(em->counter < _size);
		return em->create_entity((uint16_t)_groupId);
	}

	size_type num() const
	{
		return em->counter;
	}

	void remove_entity(entity& e)
	{
		em->remove_entity(e);

		//swap stuff

	}

	//add component to group

	//remove component from group

	bool is_full() const
	{
		return !(em->counter < _size);
	}

	bool has_all(const component_info* t, const size_type size) const
	{
		for (int i = 0; i < size; i++)
		{
			if (!component_exists(t[i].id))
			{
				return false;
			}
		}
		return true;
	}
	bool has_only(component_info* t, const size_type size) const
	{
		if (size == _nComponents)
		{
			if (has_all(t, size))
			{
				return true;
			}
		}
		return false;
	}

	bool has_all(const size_type* t, const size_type size) const
	{
		for (int i = 0; i < size; i++)
		{
			if (!component_exists(t[i]))
			{
				return false;
			}
		}
		return true;
	}

	bool component_exists(const size_type id) const
	{
		size_type size = groupOffsets->size();
		if (size > id)
		{
			auto val = groupOffsets->operator[](id);

			if (val != NOT_INIT)
			{
				return true;
			}
		}
		return false;
	}

	size_type get_offset(const size_type id) const
	{
		return groupOffsets->operator [](id);
	}

	void dispose()
	{
		em->dispose();
		groupOffsets->dispose();

		free(em);
		free(groupOffsets);
	}

	size_type _groupId;
	size_type _size;
	entity_manager* em;
	group_offset_array* groupOffsets;
	size_type _nComponents;
};

struct group_array
{
	group* _groups = nullptr;
	size_type _size = 0;
	size_type _allocated = 0;

	group& operator[](const entity& e) const
	{
		assert(e._groupId < _size);
		return _groups[e._groupId];
	}

	group& operator[](const size_t i) const
	{
		assert(i < _size);
		return _groups[i];
	}

	bool get_group(component_info* components, const size_type nComponents, group*& foundGroup)
	{
		for (int i = 0; i < _size; i++)
		{
			if (_groups[i].has_only(components, nComponents))
			{
				if (!_groups[i].is_full())
				{
					foundGroup = &_groups[i];
					return true;
				}
			}
		}
		return false;
	}

	group* make_group(group_offset_pair_array group_offset_array, const size_type groupSize)
	{
		if (_size < _allocated)
		{
			_groups[_size] = group(group_offset_array, groupSize, _size);
			_size++;
			return &_groups[_size - 1];
		}

		size_type newSize = _allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR);
		group* temp = (group*)calloc(newSize, sizeof(group));

		if (temp)
		{
			if (_groups) {
				memcpy(temp, _groups, _allocated * sizeof(group));
				free(_groups);
			}
			_groups = temp;
			_allocated = newSize;
		}
		assert(temp != nullptr);

		_groups[_size] = group(group_offset_array, groupSize, (uint16_t)_size);
		_size++;
		return &_groups[_size - 1];
	}


	void dispose()
	{
		for (int i = 0; i < _size; i++)
		{
			_groups[i].dispose();
		}

		free(_groups);
		_groups = nullptr;
		_size = 0;
		_allocated = 0;
	}

};


struct view
{
	group** _groups;
	size_type _size;
	size_type* _components;
	size_type _nComponents;


	struct view_iterator
	{
		view_iterator(group** ptr) : ptr(ptr) {}
		view_iterator operator++() { ++ptr; return *this; }
		bool operator!=(const view_iterator& other) const { return ptr != other.ptr; }
		const group* operator*() const { return *ptr; }

		group ** ptr;
	};


	view_iterator begin() const { return view_iterator(_groups); }
	view_iterator end() const { return view_iterator(_groups + _size); }

	size_type get_count() const
	{
		size_type total_count = 0;
		for (int i = 0; i < _size; i++)
		{
			total_count += _groups[i]->_size;
		}
		return total_count;
	}


	view(const size_type* components, size_type nComponents) :  _groups(nullptr), _size(0), _nComponents(nComponents), _allocated(0)
	{
		_components = (size_type*)calloc(nComponents, sizeof(size_type));
		if (_components)
		{
			memcpy(_components, components, nComponents * sizeof(size_type));
		}
		assert(_components != nullptr);
	}

	bool has_component(const size_type id) const
	{
		for (int i = 0; i < _nComponents; i++)
		{
			if (_components[i] == id)
			{
				return true;
			}
		}
		return false;
	}

	bool has_all(const size_t* components, const size_type nComponents) const
	{
		for (int i = 0; i < nComponents; i++)
		{
			if (!has_component(components[i]))
			{
				return false;
			}
		}
		return true;
	}

	bool has_all(const component_info* components, const size_type nComponents) const
	{
		for (int i = 0; i < nComponents; i++)
		{
			if (!has_component(components[i].id))
			{
				return false;
			}
		}
		return true;
	}

	bool has_only(const size_t* components, const size_type nComponents) const
	{
		if (_nComponents == nComponents)
		{
			for (int i = 0; i < nComponents; i++)
			{
				if (!has_component(components[i]))
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	void dispose()
	{
		free(_components);
		free(_groups);

		_components = nullptr;
		_groups = nullptr;
		_nComponents = 0;
		_allocated = 0;
		_size = 0;
	}

	void add_group(group* g)
	{
		if (_allocated == _size)
		{
			size_type newSize = _allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR);
			group** temp = (group**)calloc(newSize, sizeof(group*));

			if (temp)
			{
				if (_groups) {
					memcpy(temp, _groups, _allocated * sizeof(group*));
					free(_groups);
				}
				_groups = temp;
				_allocated = newSize;
			}
			assert(temp != nullptr);
		}
		if (_groups)
		{
			_groups[_size] = g;
			_size++;
		}
	}

private:
	size_type _allocated;

};


struct view_array
{
	view* _views;
	size_type _size;
	size_type _allocated;

	bool get_view(const size_t* components, const size_type nComponents, view*& v)
	{
		for (int i = 0; i < _size; i++)
		{
			if (_views[i].has_only(components, nComponents))
			{
				v = &_views[i];
				return true;
			}
		}
		return false;
	}

	view* create_view(const size_type* ids, size_type n)
	{
		if (_size < _allocated)
		{
			_views[_size] = view(ids, n);
			_size++;
			return &_views[_size - 1];
		}

		size_type newSize = (_allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR));
		view* temp = (view*)calloc(newSize, sizeof(view));

		if (temp)
		{
			if (_views) {
				memcpy(temp, _views, _allocated * sizeof(view));
				free(_views);
			}
			_views = temp;
			_allocated = newSize;
		}
		assert(temp != nullptr);

		_views[_size] = view(ids, n);
		_size++;
		return &_views[_size - 1];
	}



	void remove_view()
	{

	}

	void dispose()
	{
		for (int i = 0; i < _size; i++)
		{
			_views[i].dispose();
		}

		free(_views);
		_views = nullptr;
		_size = 0;
		_allocated = 0;
	}

};

struct component_container
{
	component_array* _componentArrays;
	size_type nComponents;



	void resize(const size_type newSize)
	{
		component_array* temp = (component_array*)calloc(newSize, sizeof(component_array));
		if (temp)
		{
			if (_componentArrays) {
				memcpy(temp, _componentArrays, nComponents * sizeof(component_array));
				free(_componentArrays);
			}
			_componentArrays = temp;
			nComponents = newSize;
		}
		assert(temp != nullptr);
	}

	component_array* operator[](size_type i)  {
		assert(i < nComponents);
		return &_componentArrays[i];
	}
	void dispose()
	{
		for (int i = 0; i < nComponents; i++)
		{
			_componentArrays[i].dispose();
		}
		free(_componentArrays);
		_componentArrays = nullptr;
		nComponents = 0;
	}

};

struct entity_key_free_list
{
	constexpr entity_key_free_list() : _entityKeys(nullptr), _size(0), _allocated(0), _frontIndex(0) {}
	size_type* _entityKeys;
	size_type _size;
	size_type _allocated;
	size_type _frontIndex;

	void push(const size_type e)
	{
		//check if front index is > 0
			//if so move the array instead of allocating new stuff
			//todo: there is probably some more efficient strategy here. //probably the best way is to fill the array from the front instead of moving
				//potentially we end up repeatedly moving the array every time we push, which may or may not be efficient depending on the size of the list
				//this would only happen in some specific situation ex. when the count of elements swaps between _allocated-1 and _allocated
				//need benchmarking
		if (_allocated == _size - _frontIndex)
		{
			for (size_type i = _frontIndex; i < _size; i++) {
				_entityKeys[i - _frontIndex] = _entityKeys[i];
			}
			_size -= _frontIndex;
			_frontIndex = 0;
		}
		else if (_allocated == _size)
		{
			size_type newSize = (_allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR));
			size_type* temp = (size_type*)calloc(newSize, sizeof(size_type));

			if (temp)
			{
				if (_entityKeys != nullptr) {
					memcpy(temp, _entityKeys, _allocated * sizeof(size_type));
					free(_entityKeys);
				}
				_entityKeys = temp;
				_allocated = newSize;
			}
			assert(temp != nullptr);
		}
		_entityKeys[_size] = e;
		_size++;
	}

	size_type pop()
	{
		size_type e = _entityKeys[_frontIndex];
		_frontIndex++;
		return e;
	}

	bool empty() const
	{
		return _size - _frontIndex == 0;
	}

	void dispose()
	{
		free(_entityKeys);
		_entityKeys = nullptr;
		_size = 0;
		_frontIndex = 0;
		_allocated = 0;
	}

};


struct entity_key
{
	uint32_t index = 0;
	uint32_t version = 0;
	entity e;
};

struct entity_key_manager
{
	entity_key_manager() : _size(0), _allocated(0), keys(nullptr) {	}

	entity_key create(entity e)
	{
		if (freeList.empty())
		{
			if (!(_size < _allocated))
			{

				size_type newSize = _allocated == 0 ? 1 : (size_type)ceil((double)_allocated * GROWTH_FACTOR);
				entity_key* temp = (entity_key*)calloc(newSize, sizeof(entity_key));
				if (temp)
				{
					if (keys != nullptr)
					{
						memcpy(temp, keys, _allocated * sizeof(entity_key));
						free(keys);
					}
					keys = temp;
					_allocated = newSize;
				}
				assert(temp != nullptr);

			}
			keys[_size] = entity_key{ (uint32_t)_size, 0, e };
			_size++;
			return keys[_size - 1];
		}
		size_type index = freeList.pop();
		keys[index].e = e;
		return keys[index];
	}

	void remove(entity_key key)
	{
		keys[key.index].version++;
		freeList.push(key.index);
	}

	entity operator[](entity_key key) const
	{
		return keys[key.index].e;
	}

	void dispose()
	{
		free(keys);
		keys = nullptr;
		freeList.dispose();
		_size = 0;
		_allocated = 0;
	}

	size_type _size;
	size_type _allocated;

	entity_key* keys;
	entity_key_free_list freeList;
};


struct entity_component_system
{
	entity_component_system() : _viewCache(view_array()), _groups(group_array()), _componentArrays(component_container()) { }

	const entity_key create_entity(archetype_descriptor components, size_type groupSize = 1600U)
	{
		group* g = nullptr;
		get_or_make_group(components.arr, components.size, g, groupSize);
		if (g)
		{
			entity e = g->create_entity();
			return _entityKeys.create(e);
		}
		assert(g != nullptr);
		return entity_key();
	}

	void remove_entity(entity_key& eKey)
	{
		entity e = (_entityKeys[eKey]);
		_entityKeys.remove(eKey);
		_groups[e].remove_entity(e);
	}

	//void add_component(const entityKey& eKey, size_type id)
	//{
	//	//move entity from current group to another group
	//}

	//void remove_component(const entityKey& eKey, const size_type id)
	//{
	//	//move entity from current group to another group
	//}

	bool has_component(const entity_key& eKey, const size_type id) const
	{
		entity e = (_entityKeys[eKey]);
		return _groups[e].component_exists(id);
	}

	template<typename T>
	T* get_component_array()
	{
		auto id = component_id<T>;
		if (!(id < _componentArrays.nComponents))
		{
			_componentArrays.resize(id + 1);
		}
		return (T*)_componentArrays[id]->_data;
	}

	template<typename T>
	T& get_component(const entity_key& e)
	{
		const size_type componentId = component_id<T>;

		auto data_array = _componentArrays[componentId];

		assert(_componentArrays.nComponents > componentId);
		assert(data_array->_data != nullptr);
		assert(_groups[e.e].component_exists(componentId));
		T* compArray = (T*)data_array->_data;
		return compArray[_groups[e.e].get_offset(componentId) + e.e.index()];
	}


	template <typename T>
	void set_component(const entity_key& e, T value)
	{
		const size_type componentId = component_id<T>;
		assert(_componentArrays.nComponents > componentId);
		assert(_componentArrays[componentId]._data != nullptr);
		assert(_groups[e.e].component_exists(componentId));

		T* compArray = (T*)_componentArrays[componentId]._data;
		compArray[_groups[e.e].get_offset(componentId) + e.e.index()] = value;
	}

	view* get_view(const size_type* ids, const size_type n)
	{
		view* v;
		if (_viewCache.get_view(ids, n, v))
		{
			return v;
		}

		v = _viewCache.create_view(ids, n);

		const size_type count = _groups._size;
		for (int i = 0; i < count; i++)
		{
			if (_groups[i].has_all(ids, n))
			{
				v->add_group(&_groups[i]);
			}
		}
		return v;
	}

	void get_or_make_group(component_info* components, const size_type nComponents, group*& g, const size_type groupSize)
	{
		if (_groups.get_group(components, nComponents, g))
		{
			return;
		}

		group_offset_pair_array arr = group_offset_pair_array();

		for (int i = 0; i < nComponents; i++)
		{
			group_offset_pair pair = group_offset_pair();
			if (!(components[i].id < _componentArrays.nComponents))
			{
				_componentArrays.resize(components[i].id + 1U);
			}
			if (_componentArrays[components[i].id]->_typeSize == 0)
			{
				_componentArrays[components[i].id]->_typeSize = components[i].typeSize;
			}

			pair.componentId = components[i].id;
			pair.offset = _componentArrays[components[i].id]->get_or_create_chunk(groupSize);
			arr.push(pair);
		}


		if (arr._size > 0)
		{
			g = _groups.make_group(arr, groupSize);

			const size_type nViews = _viewCache._size;
			for (size_type i = 0; i < nViews; i++)
			{
				if (g->has_all(_viewCache._views[i]._components, _viewCache._views[i]._nComponents))
				{
					_viewCache._views[i].add_group(&(*g));
				}
			}

		}

		arr.dispose();
	}

	void dispose()
	{
		_entityKeys.dispose();
		_groups.dispose();
		_componentArrays.dispose();
		_viewCache.dispose();
	}

private:

	entity_key_manager _entityKeys;
	view_array _viewCache;
	group_array _groups;
	component_container _componentArrays;
};