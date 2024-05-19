/*Copyright 2024 Ilia Nozdrachev

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <windows.h>

#include <mutex>
#include <thread>
#include <vector>
#include <future>
#include <optional>
#include <iostream>
#include <algorithm>
#include <functional>
#include <queue>

namespace HKPP
{
    namespace extra
    {
        template<typename T>
        class ThreadSafeQueue {
        private:
            std::queue<T> queue;
            mutable std::mutex mtx;
            std::condition_variable cv;

        public:
            ThreadSafeQueue() {}

            ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;
            ThreadSafeQueue& operator=(const ThreadSafeQueue<T>&) = delete;

            void Enqueue(T message) {
                std::lock_guard<std::mutex> lock(mtx);
                queue.push(std::move(message));
                cv.notify_one();
            }

            T Dequeue() {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return !queue.empty(); });
                T message = std::move(queue.front());
                queue.pop();
                return message;
            }

            bool isEmpty() const {
                std::lock_guard<std::mutex> lock(mtx);
                return queue.empty();
            }
        };

        class Bencher
        {
        private:
            std::chrono::steady_clock::time_point start_time;
            std::chrono::steady_clock::time_point end_time;

        public:
            void Start();
            void Stop();
        };

        template <class T> class VectorEx : public std::vector <T>
        {
        public:
            using std::vector<T>::vector;

            bool Contains(T val);
            bool Contains(std::function<bool(T&)> callback);
            std::optional<T> Find(T val);
            std::optional<T> Find(std::function<bool(T&)> callback);
            void RemAll(T val);
            void RemIf(std::function <bool(T&)> fnc);
            void Foreach(std::function <void(T&)> fnc);
            void Sort(std::function <bool(T&, T&)> fnc);

            bool operator==(VectorEx<T>& rhs);
            bool operator!=(VectorEx<T>& rhs);
        };
    }

    using namespace HKPP::extra;

    class Key;
    class Hotkey;
    class HotkeyEvent;
    class KBDEventCallback;
    class Manager;

    extern std::function <void(HotkeyEvent)> DefaultHotkeyCallback;

    enum kbd_event_propagation
    {
        PROPAGATE = 0,
        BLOCK
    };

    enum class injection_status
    {
        INJECTED = 0,
        LL_INJECTED,
        REAL
    };

    class Key
    {
    public:
        DWORD key = NULL;
        std::chrono::steady_clock::time_point time = std::chrono::high_resolution_clock::now();
        injection_status injected = injection_status::INJECTED;

        Key(DWORD key_ARG, injection_status injected_ARG = injection_status::INJECTED);

        bool operator== (const Key& s) const;
        bool operator!= (const Key& s) const;
        bool operator>  (const Key& s) const;
        bool operator<  (const Key& s) const;
        bool operator<= (const Key& s) const;
        bool operator>= (const Key& s) const;

        bool operator== (const DWORD& s) const;
        bool operator!= (const DWORD& s) const;
        bool operator>  (const DWORD& s) const;
        bool operator<  (const DWORD& s) const;
        bool operator<= (const DWORD& s) const;
        bool operator>= (const DWORD& s) const;
    };

    class Hotkey
    {
    public:

        void* userdata = nullptr;

        enum injection_permission
        {
            ALLOW_ALL = 0,
            ALLOW_LL_INJECTED,
            ONLY_REAL_INPUT
        };

        enum parallel_execution
        {
            BLOCK = 0,
            ALLOW,
        };


        size_t id = 0;

        kbd_event_propagation kbdEventPropagationStatus = kbd_event_propagation::PROPAGATE;
        injection_permission allowedInjectionLevel = injection_permission::ONLY_REAL_INPUT;
        parallel_execution paralelExecutionPolicy = parallel_execution::BLOCK;

        VectorEx <Key> keyList;

        std::function <void(HotkeyEvent)> callback = DefaultHotkeyCallback;

        Hotkey(
            VectorEx <Key> KeyList,
            std::function <void(HotkeyEvent)> Callback = DefaultHotkeyCallback,
            kbd_event_propagation KbdEventPropagationStatus = kbd_event_propagation::PROPAGATE,
            injection_permission InjectionPermission = injection_permission::ONLY_REAL_INPUT,
            parallel_execution ParalelExecutionPolicy = parallel_execution::BLOCK
        );

        kbd_event_propagation checkAndDispatch(VectorEx <Key>& KeyboardState, Key& eventTrigger);

        bool operator!= (const Hotkey& s) const;
        bool operator== (const Hotkey& s) const;
    };


    class KBDEventCallback
    {
    public:
        KBDEventCallback(size_t CallbackId, std::function <kbd_event_propagation(int, WPARAM, LPARAM, VectorEx<Key>&, bool)> CallbackFunction)
        {
            callbackId = CallbackId;
            callbackFunction = CallbackFunction;
        }

        size_t callbackId = 0;
        std::function <kbd_event_propagation(int, WPARAM, LPARAM, VectorEx<Key>&, bool)> callbackFunction;

        bool operator==(const KBDEventCallback& s) const {
            return callbackId == s.callbackId;
        }

        bool operator!=(const KBDEventCallback& s) const {
            return !(*this == s);
        }
    };


    class HotkeyEvent
    {
    public:
        HotkeyEvent(Hotkey Hotkey, injection_status InjectionStatus);
        Hotkey hotkey;
        injection_status injectionStatus = injection_status::INJECTED;
    };

    class HotkeyCallbackHandle {
    public:
        HotkeyCallbackHandle(size_t HotkeyId, HotkeyEvent _hotkeyEvent, std::function <void(HotkeyEvent)> Callback);

        HotkeyEvent hotkeyEvent;
        std::function <void(HotkeyEvent)> callback;
        size_t hotkeyId = 0;
    };



    class Manager
    {
    private:

        Manager();

        static Manager* instance;
        static std::atomic<DWORD>* hook_proc_thid;

        std::atomic <size_t> hotkeyIdAutoincrement = 1;
        std::atomic <size_t> callbackIdAutoincrement = 1;

        friend class Hotkey;

        /*
        Unused
        void HKPP_Stop();
        //*/

    protected:

        VectorEx <Key> keyboardState;
        std::mutex* keyboardState_mutex = nullptr;

        VectorEx <Hotkey> hotkeys;
        std::mutex* hotkeys_mutex = nullptr;

        VectorEx <KBDEventCallback> LLKP_AdditionalCallbacks;
        std::mutex* LLKP_AdditionalCallbacks_mutex = nullptr;

        VectorEx<HotkeyCallbackHandle> HKPP_CallbackHandles;
        std::mutex* HKPP_CallbackHandles_mutex = nullptr;

        static ThreadSafeQueue<HotkeyCallbackHandle> callbackQueue;

        size_t getNewHotkeyId();
        size_t getNewCallbackId();

        std::thread* hook_main_thread = nullptr;
        static void hook_main();
        static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
        static void RunCallbacksThread();

        std::thread* callback_thread = nullptr;


    public:
        static Manager* GetInstance();
        static VectorEx<Key> GetKeyboardState();

        void HKPP_Init();


        size_t RegisterHotkey(Hotkey desk);
        std::optional<Hotkey> GetHotkey(size_t uuid);
        bool UpdateHotkey(Hotkey hotkey);
        void UnregisterHotkey(size_t uuid);
        void UnregisterAllHotkeys();

        size_t RegisterCallback(std::function <kbd_event_propagation(int, WPARAM, LPARAM, VectorEx<Key>&, bool)> fnc_p);
        void UnregisterAllCallbacks();
        void UnregisterCallback(size_t uuid);
    };
}

namespace HKPP
{
    namespace extra
    {
        template <class T>
        bool VectorEx<T>::Contains(T val)
        {
            auto iterator = std::find(this->begin(), this->end(), val);
            return iterator != this->end();
        }

        template <class T>
        bool VectorEx<T>::Contains(std::function<bool(T&)> callback)
        {
            auto iterator = std::find_if(this->begin(), this->end(), callback);
            return iterator != this->end();
        }

        template <class T>
        std::optional<T> VectorEx<T>::Find(T val)
        {
            auto iterator = std::find(this->begin(), this->end(), val);
            return iterator != this->end() ? std::optional<T>{*iterator} : std::nullopt;
        }

        template <class T>
        std::optional<T> VectorEx<T>::Find(std::function<bool(T&)> callback)
        {
            auto iterator = std::find_if(this->begin(), this->end(), callback);
            return iterator != this->end() ? std::optional<T>{*iterator} : std::nullopt;
        }


        template <class T>
        void VectorEx <T>::RemAll(T val)
        {
            this->erase(
                std::remove_if(this->begin(), this->end(), [&](T& item) -> bool { return (item == val); })
                , this->end());
        }

        template <class T>
        void VectorEx <T>::RemIf(std::function <bool(T&)> fnc) { this->erase(std::remove_if(this->begin(), this->end(), fnc), this->end()); }
        template <class T>
        void VectorEx <T>::Foreach(std::function <void(T&)> fnc) { std::for_each(this->begin(), this->end(), fnc); }
        template <class T>
        void VectorEx <T>::Sort(std::function <bool(T&, T&)> fnc) { std::sort(this->begin(), this->end(), fnc); }

        template <class T>
        bool VectorEx <T>::operator==(VectorEx<T>& rhs)
        {
            if (this->size() == rhs.size())
                return std::equal(this->begin(), this->end(), rhs.begin());

            return false;
        }

        template <class T>
        bool VectorEx <T>::operator!=(VectorEx<T>& rhs)
        {
            if (this->size() == rhs.size())
                return !std::equal(this->begin(), this->end(), rhs.begin());

            return true;
        }
    }
}