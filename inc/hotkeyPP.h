/*Copyright 2020 Nozdrachev Ilia

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
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <functional>

#define WM_HKPP_DEFAULT_CALLBACK_MESSAGE (WM_APP+1)

#define HKPP_BLOCK_INPUT true
#define HKPP_ALLOW_INPUT false

#define HKPP_ALLOW_INJECTED true
#define HKPP_DENY_INJECTED false

namespace HKPP
{
    namespace extra
    {

        template <class T> class VectorEx : public std::vector <T>
        {
        public:
            using std::vector<T>::vector;

            bool Contains(T val);
            void Rem_All(T val);
            void Rem_If(std::function <bool(T)> fnc);
            void foreach(std::function <void(T&)> fnc);
            void Sort(std::function <bool(T, T)> fnc);

            bool operator==(VectorEx<T>& rhs);
            bool operator!=(VectorEx<T>& rhs);
        };
    }

    using namespace HKPP::extra;

    enum class injected_status_enm
    {
        UNDEFINED_INJECTION_STATUS = 0,
        INJECTED = 1,
        NOT_INJECTED = 2
    };

    class key_deskriptor
    {
    public:
        DWORD Key = NULL;
        std::chrono::steady_clock::time_point Time = std::chrono::high_resolution_clock::now();
        injected_status_enm Injected = injected_status_enm::UNDEFINED_INJECTION_STATUS;

        key_deskriptor();

        key_deskriptor(DWORD key_ARG, injected_status_enm injected_ARG);

        key_deskriptor(DWORD key_ARG);

        bool operator== (key_deskriptor& s);
        bool operator!= (key_deskriptor& s);
        bool operator>  (key_deskriptor& s);
        bool operator<  (key_deskriptor& s);
        bool operator<= (key_deskriptor& s);
        bool operator>= (key_deskriptor& s);

        bool operator== (DWORD& s);
        bool operator!= (DWORD& s);
        bool operator>  (DWORD& s);
        bool operator<  (DWORD& s);
        bool operator<= (DWORD& s);
        bool operator>= (DWORD& s);
    };

    class Hotkey_Deskriptor;

    struct Hotkey_Settings_t
    {
        void* userdata;
        std::function <void(void*)> userdata_destructor;

    public:

        void Set_User_Data(void* udata, std::function <void(void*)> udata_destructor)
        {
            userdata = udata;
            userdata_destructor = udata_destructor;
        }

        void* Get_User_Data()
        {
            return userdata;
        }

        DWORD Thread_Id = 0;
        bool Block_Input = 0;
        bool Allow_Injected = 0;
        UINT Msg = NULL;

        size_t uuid = 0;
        std::wstring name = L"";
        std::function <void(Hotkey_Deskriptor)> user_callback = {};

        Hotkey_Settings_t(
            std::wstring name_,
            std::function <void(Hotkey_Deskriptor)> user_callback_ = {},
            DWORD Thread_Id_ = 0,
            bool Block_Input_ = HKPP_ALLOW_INPUT,
            bool Allow_Injected_ = HKPP_DENY_INJECTED,
            UINT Msg_ = WM_HKPP_DEFAULT_CALLBACK_MESSAGE
        )

        {
            Thread_Id = Thread_Id_;
            Block_Input = Block_Input_;
            Allow_Injected = Allow_Injected_;
            Msg = Msg_;

            uuid = uuid;
            name = name_;
            user_callback = user_callback_;
        }

        Hotkey_Settings_t()
        {
            userdata_destructor(userdata);
        }
    };

    class Hotkey_Deskriptor
    {
    public:
        bool Real = false;
        VectorEx <key_deskriptor> Key_List;
        Hotkey_Settings_t settings;

        bool operator!= (Hotkey_Deskriptor& s);
        bool operator== (Hotkey_Deskriptor& s);

        Hotkey_Deskriptor(VectorEx <key_deskriptor> keys_vector, Hotkey_Settings_t set)
        {
            Init(keys_vector, set);
        }

        void Init(VectorEx <key_deskriptor> keys_vector, Hotkey_Settings_t set);
        bool Check_Combination(VectorEx <key_deskriptor>& KState);
        void Send_Event() noexcept;
    };

    struct callback_descriptor_t
    {
        size_t uuid = 0;
        std::function <bool(int, WPARAM, LPARAM, VectorEx<key_deskriptor>&, bool)> fnc_p;
    };

    class Hotkey_Manager
    {
    private:
        Hotkey_Manager();
        static Hotkey_Manager* instance;
        static std::atomic<DWORD>* hook_proc_thid;

    protected:

        static VectorEx <key_deskriptor> Keyboard_Deskriptor;
        static std::mutex* Keyboard_Deskriptor_Mutex;

        static VectorEx <Hotkey_Deskriptor> Combinations;
        static std::mutex* comb_vec_mutex;

        //return -> if true input will be blocked else allowed
        static VectorEx <callback_descriptor_t> LLK_Proc_Additional_Callbacks;
        static std::mutex* LLK_Proc_Additional_Callbacks_mutex;


        std::thread* hook_main_th = NULL;
        static void hook_main();
        static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    public:
        static Hotkey_Manager* Get_Instance();
        static VectorEx<key_deskriptor> GetKeyboardState();

        void HKPP_Init();
        void HKPP_Stop();


        size_t Add_Hotkey(Hotkey_Deskriptor desk);
        void Clear_Hotkeys();
        void Remove_Hotkey(size_t uuid);

        size_t Add_Callback(std::function <bool(int, WPARAM, LPARAM, VectorEx<key_deskriptor>&, bool)> fnc_p); // if succeeded returns uuid for callback else 0 
        void Clear_Callbacks();
        void Remove_Callback(size_t uuid);
    };
}

namespace HKPP
{
    namespace extra
    {
        template <class T>
        bool VectorEx<T>::Contains(T val)
        {
            for (size_t i = 0; i < this->size(); i++)
                if ((*this)[i] == val)
                    return true;

            return false;
        }

        template <class T>
        void VectorEx <T>::Rem_All(T val)
        {
            this->erase(
                std::remove_if(this->begin(), this->end(), [&](T& item) -> bool { return (item == val); })
                , this->end());
        }

        template <class T>
        void VectorEx <T>::Rem_If(std::function <bool(T)> fnc) { std::remove_if(this->begin(), this->end(), fnc); }
        template <class T>
        void VectorEx <T>::foreach(std::function <void(T&)> fnc) { std::for_each(this->begin(), this->end(), fnc); }
        template <class T>
        void VectorEx <T>::Sort(std::function <bool(T, T)> fnc) { std::sort(this->begin(), this->end(), fnc); }

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