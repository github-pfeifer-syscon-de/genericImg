/*
 * Copyright (C) 2021 rpf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <list>
#include <deque>
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <algorithm>


// adapted from
// https://www.xtof.info/c11-concurrency-wrapping-stdlist-to-make-it-thread-safe.html

/** @brief  A templated *thread-safe* collection based on dequeue
            pop_front() waits for the notification of a filling method if the collection is empty.
            The various "emplace" operations are factorized by using the generic "addData_protected".
            This generic asks for a concrete operation to use, which can be passed as a lambda.
**/
template< typename T >
class TQueueConcurrent {

    using const_iterator = typename std::deque<T>::const_iterator;

public:

    TQueueConcurrent()
    : _active(true)
    {

    }

    //! @brief Emplaces a new instance of T in front of the deque
    template<typename... Args>
    void emplace_front( Args&&... args )
    {
        addData_protected( [&] {
            _collection.emplace_front(std::forward<Args>(args)...);
        } );
    }

    /** @brief Emplaces a new instance of T at the back of the deque **/
    template<typename... Args>
    void emplace_back( Args&&... args )
    {
        addData_protected( [&] {
            _collection.emplace_back(std::forward<Args>(args)...);
        } );
    }

    /** @brief  Returns the front element and removes it from the collection
                No exception is ever returned as we garanty that the deque is not empty
                before trying to return data.
    **/
    T pop_front( void ) noexcept
    {
        std::unique_lock<std::mutex> lock{_mutex};
        while (!_active
             || _collection.empty()) {
            if (!_active) {
                return nullptr;     // works only for pointer data..-
            }
            _condNewData.wait(lock);
        }
        auto elem = std::move(_collection.front());
        _collection.pop_front();
        return elem;
    }

    void finish( void )
    {
        _active = false;
        _condNewData.notify_one();
    }

    bool isActive( void )
    {
        return _active;
    }
private:

    /** @brief  Protects the deque, calls the provided function and notifies the presence of new data
        @param  The concrete operation to be used. It MUST be an operation which will add data to the deque,
                as it will notify that new data are available!
    **/
    template<class F>
    void addData_protected(F&& fct)
    {
        std::unique_lock<std::mutex> lock{ _mutex };
        fct();
        lock.unlock();
        _condNewData.notify_one();
    }

    std::deque<T> _collection;              ///< Concrete, not thread safe, storage.

    std::mutex   _mutex;                    ///< Mutex protecting the concrete storage

    std::condition_variable _condNewData;   ///< Condition used to notify that new data are available.

    bool _active;
};

template< typename T >
class TListConcurrent {

    using const_iterator = typename std::list<T>::const_iterator;

public:

    TListConcurrent()
    {

    }

    /** @brief Emplaces a new instance of T at the back of the list **/
    template<typename... Args>
    void push_back( Args&&... args )
    {
        addData_protected( [&] {
            _collection.push_back(std::forward<Args>(args)...);
        } );
    }

    /** @brief  Returns the front element and removes it from the collection
                No exception is ever returned as we garanty that the list is not empty
                before trying to return data.
    **/
    T pop_front( void ) noexcept
    {
        std::unique_lock<std::mutex> lock{_mutex};
        if (_collection.empty()) {
            return nullptr;     // works only for pointer data..-
        }
        auto elem = std::move(_collection.front());
        _collection.pop_front();
        return elem;
    }

private:

    /** @brief  Protects the deque, calls the provided function
    **/
    template<class F>
    void addData_protected(F&& fct)
    {
        std::unique_lock<std::mutex> lock{ _mutex };
        fct();
        lock.unlock();
    }

    std::list<T> _collection;              ///< Concrete, not thread safe, storage.

    std::mutex   _mutex;                    ///< Mutex protecting the concrete storage

};

template< typename K, typename T >
class TMapConcurrent {

    using const_iterator = typename std::map<K, T>::const_iterator;

public:

    TMapConcurrent()
    {

    }

    /** @brief put a new entry into map **/
    void insert(const K& k,const T& t )
    {
        addData_protected( [&] {
            _collection.insert(std::pair<K, T>(k, t));
        } );
    }

    /** @brief  get a entry from map
    **/
    T get(const K &k ) noexcept
    {
        std::unique_lock<std::mutex> lock{_mutex};
        auto p = _collection.find(k);
        if (p == _collection.end()) {
            return nullptr;     // works only for pointer data..-
        }
        auto elem = p->second;
        return elem;
    }

    /** @brief remove from map
    **/
    bool remove(const K &k ) noexcept
    {
        std::unique_lock<std::mutex> lock{_mutex};
        auto p = _collection.find(k);
        if (p == _collection.end()) {
            return false;
        }
        _collection.erase(p);
        return true;
    }

    bool empty( void )
    {
        return _collection.empty();
    }
private:

    /** @brief  Protects the map, calls the provided function
    **/
    template<class F>
    void addData_protected(F&& fct)
    {
        std::unique_lock<std::mutex> lock{ _mutex };
        fct();
        lock.unlock();
    }

    std::map<K, T> _collection;              ///< Concrete, not thread safe, storage.

    std::mutex   _mutex;                    ///< Mutex protecting the concrete storage

};

template< typename T >
class TVectorConcurrent {

    using const_iterator = typename std::vector<T>::const_iterator;

public:

    TVectorConcurrent()
    {
    }

    //! @brief push a new instance of T
    template<typename... Args>
    void push_back( Args&&... args )
    {
        modifyData_protected( [&] {
            _collection.push_back(std::forward<Args>(args)...);
        } );
    }


   /** @brief  get a entry from vector
    **/
    T operator[]( size_t index ) const
    {
        //std::unique_lock<std::mutex> lock{_mutex}; for our usage we just append elements (rarely clearing does not count) so the access shoud be fine this way
        auto elem = _collection[index];
        return elem;
    }

    /** @brief  sort
    **/
    template< typename S >
    void sort( S s )
    {
        modifyData_protected( [&] {
            std::sort(_collection.begin(), _collection.end(), s);
        } );
    }

    void clear( void )
    {
        modifyData_protected( [&] {
            _collection.clear();
        } );
    }

    size_t size( void )
    {
        return _collection.size();
    }

    const_iterator begin( void ) const
    {
        return _collection.begin();
    }

    const_iterator end( void ) const
    {
        return _collection.end();
    }

    std::mutex& getMutex() {
        return _mutex;
    }
private:

    /** @brief  Protects the vector, calls the provided function
    **/
    template<class F>
    void modifyData_protected(F&& fct)
    {
        std::unique_lock<std::mutex> lock{ _mutex };
        fct();
    }

    std::vector<T> _collection;              ///< Concrete, not thread safe, storage.

    std::mutex   _mutex;                    ///< Mutex protecting the concrete storage

};
