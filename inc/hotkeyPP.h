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

    struct Hotkey_Settings_t
    {
    public:
        DWORD Thread_Id;
        bool Block_Input;
        bool Allow_Injected;
        UINT Msg;
    };

    class Hotkey_Deskriptor
    {
    public:
        bool Real = false;
        VectorEx <key_deskriptor> Key_List;
        Hotkey_Settings_t settings;

        bool operator!= (Hotkey_Deskriptor& s);
        bool operator== (Hotkey_Deskriptor& s);

        Hotkey_Deskriptor(VectorEx <key_deskriptor> keys_vector, Hotkey_Settings_t set);


        void Init(VectorEx <key_deskriptor> keys_vector, DWORD th_id, bool block_input = false, bool allow_injected = false, UINT msg_arg = WM_HKPP_DEFAULT_CALLBACK_MESSAGE);

        bool Check_Combination(VectorEx <key_deskriptor>& KState);

        void Send_Event();

    };

    struct callback_descroptor_t
    {
        size_t uuid = 0;
        std::function <bool(int, WPARAM, LPARAM)> fnc;
    };

    class Hotkey_Manager
    {
    private:
        Hotkey_Manager();

        static Hotkey_Manager* instance;

        static std::atomic<DWORD>* hook_proc_thid;

    protected:

        static VectorEx <Hotkey_Deskriptor> Combinations;
        static std::mutex* comb_vec_mutex;

        //return -> if true input will be blocked else allowed
        //int nCode, WPARAM wParam, LPARAM lParam > standart LowLevelKeyboardProc args
        //! DO NOT CALL "CallNextHookEx", for love nor money!
        //https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644985(v=vs.85) for details 

        static VectorEx <callback_descroptor_t> LLK_Proc_Additional_Callbacks;
        static std::mutex* LLK_Proc_Additional_Callbacks_mutex;


        std::thread* hook_main_th = NULL;

        static void hook_main();

        static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    public:
        static Hotkey_Manager* Get_Instance();

        void HKPP_Init();
        void HKPP_Stop();

        void Clear();
        bool Add(Hotkey_Deskriptor desk);
        bool Add_Callback(std::function <bool(int, WPARAM, LPARAM)> fnc_p, size_t uuid_p);
        void Remove(Hotkey_Deskriptor desk);

    };
}

namespace HKPP
{
    namespace extra
    {
        template <class T>
        bool VectorEx<T>::Contains(T val)
        {
            for (int i = 0; i < this->size(); i++)
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
        void VectorEx <T>::foreach(std::function <void(T&)> fnc)
        {
            std::for_each(this->begin(), this->end(), fnc);
        }

        template <class T>
        void VectorEx <T>::Sort(std::function <bool(T, T)> fnc)
        {
            std::sort(this->begin(), this->end(), fnc);
        }

        template <class T>
        bool VectorEx <T>::operator==(VectorEx<T>& rhs)
        {
            if (this->size() == rhs.size())
            {
                return std::equal(this->begin(), this->end(), rhs.begin());
            }

            return false;
        }

        template <class T>
        bool VectorEx <T>::operator!=(VectorEx<T>& rhs)
        {
            if (this->size() == rhs.size())
            {
                return !std::equal(this->begin(), this->end(), rhs.begin());
            }

            return true;
        }
    }
}