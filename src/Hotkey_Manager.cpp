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
#include "hotkeyPP.h"

using namespace HKPP::extra;


namespace HKPP
{
    VectorEx <key_deskriptor> Hotkey_Manager::Keyboard_Deskriptor = {};
    std::mutex* Hotkey_Manager::Keyboard_Deskriptor_Mutex;

    Hotkey_Manager* Hotkey_Manager::instance;

    VectorEx <Hotkey_Deskriptor> Hotkey_Manager::Combinations;
    std::mutex* Hotkey_Manager::comb_vec_mutex;

    VectorEx <callback_descriptor_t> Hotkey_Manager::LLK_Proc_Additional_Callbacks;
    std::mutex* Hotkey_Manager::LLK_Proc_Additional_Callbacks_mutex;

    std::atomic<DWORD>* Hotkey_Manager::hook_proc_thid;

    VectorEx<key_deskriptor> Hotkey_Manager::GetKeyboardState()
    {
        Keyboard_Deskriptor_Mutex->lock();
        VectorEx<key_deskriptor> tmp = Keyboard_Deskriptor;
        Keyboard_Deskriptor_Mutex->unlock();

        return tmp;
    }

    Hotkey_Manager::Hotkey_Manager()
    {
        this->HKPP_Init();
    }

    Hotkey_Manager* Hotkey_Manager::Get_Instance()
    {
        if (instance == NULL)
            instance = new Hotkey_Manager();

        return instance;
    }

    void Hotkey_Manager::hook_main()
    {
        *hook_proc_thid = GetCurrentThreadId();

        HHOOK hook_handle = SetWindowsHookExW(WH_KEYBOARD_LL, Hotkey_Manager::LowLevelKeyboardProc, NULL, NULL);
        MSG msg;

        if (!hook_handle)
            printf("\nHotkey_Manager::hook_main() >> enable to start | code:%d\n", GetLastError());

        while (GetMessageW(&msg, NULL, NULL, NULL))
        {
            TranslateMessage(&msg);

            if (msg.message == WM_QUIT)
                break;


            if (msg.message == WM_HKPP_DEFAULT_CALLBACK_MESSAGE)
                if (msg.lParam)
                {
                    Hotkey_Deskriptor* dsk = (Hotkey_Deskriptor*)msg.lParam;
                    dsk->settings.user_callback(*dsk);
                    delete dsk;
                }

            DispatchMessageW(&msg);

        }

        if (hook_handle != nullptr)
        {
            UnhookWindowsHookEx(hook_handle);
            hook_handle == nullptr;
        }
        hook_proc_thid = 0;
    }

    LRESULT CALLBACK Hotkey_Manager::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        bool repeated_input = false;
        bool block_input = false;

        comb_vec_mutex->lock();
        VectorEx <Hotkey_Deskriptor> Local_Combinations = Combinations;
        comb_vec_mutex->unlock();

        Keyboard_Deskriptor_Mutex->lock();
        if (nCode == HC_ACTION)
        {
            KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

            key_deskriptor key_desk;

            key_desk.Key = kbd->vkCode;
            key_desk.Injected = (((kbd->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED) || ((kbd->flags & LLKHF_INJECTED) == LLKHF_INJECTED))
                ? HKPP::injected_status_enm::INJECTED : HKPP::injected_status_enm::NOT_INJECTED;

            key_desk.Time = std::chrono::high_resolution_clock::now();

            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
            {
                if (!Keyboard_Deskriptor.Contains(key_desk.Key))
                {
                    Keyboard_Deskriptor.push_back(key_desk);  //# insert
                    Keyboard_Deskriptor.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });
                }

                else
                {
                    repeated_input = true;

                    Keyboard_Deskriptor.foreach([&](key_deskriptor& kd) -> void
                        {
                            if (kd.Key == key_desk.Key)
                            {
                                kd = key_desk;
                                repeated_input = false;
                            }
                        });
                }
            }

            Keyboard_Deskriptor.Rem_If([&](HKPP::key_deskriptor hd) -> bool
                {
                    return (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - hd.Time).count() > 1200);
                });

            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
                Keyboard_Deskriptor.Rem_All(kbd->vkCode);

            VectorEx <Hotkey_Deskriptor> combs;

            Local_Combinations.foreach([&](Hotkey_Deskriptor& desk) -> void //check which combination pressed now
                {

                    if (desk.Check_Combination(Keyboard_Deskriptor))
                    {
                        if (desk.Key_List.Contains(key_desk))// only if key that pressed is in key list 
                        {
                            if (!repeated_input)
                            {
                                desk.Real = true;

                                for (size_t i = 0; i < desk.Key_List.size(); i++)
                                {
                                    if (Keyboard_Deskriptor.Contains({ desk.Key_List[i].Key, HKPP::injected_status_enm::INJECTED }) && !Keyboard_Deskriptor.Contains({ desk.Key_List[i].Key, HKPP::injected_status_enm::NOT_INJECTED }))
                                    {
                                        desk.Real = false;
                                        break;
                                    }
                                }
                                combs.push_back(desk);
                            }
                            block_input |= desk.settings.Block_Input;
                        }
                    }
                });

            /*if you have 2 combinations call bigger one:
            if you have 2 combs pressed

            1) CTRL + ALT + SHIFT + DELETE
            2) CTRL + ALT + SHIFT

            only 1 will be callsed because second was pressed only because of imposition

            */
            combs.foreach([&](Hotkey_Deskriptor& d) -> void
                {
                    bool send = true;
                    combs.foreach([&](Hotkey_Deskriptor& d_in) -> void
                        {
                            if (d != d_in && d.Check_Combination(d_in.Key_List))
                                send = false;
                        });

                    if (send && !repeated_input)
                        d.Send_Event();
                });

        }
        Keyboard_Deskriptor_Mutex->unlock();

        LLK_Proc_Additional_Callbacks_mutex->lock();
        //{
        /**/LLK_Proc_Additional_Callbacks.foreach([&](callback_descriptor_t& dsk) -> void
            {
                block_input |= dsk.fnc_p(nCode, wParam, lParam, Keyboard_Deskriptor, repeated_input);
            });
        //}
        LLK_Proc_Additional_Callbacks_mutex->unlock();


        if (block_input)
            return 1;
        else
            return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    void Hotkey_Manager::HKPP_Init()
    {
        if (!comb_vec_mutex)
            comb_vec_mutex = new std::mutex;

        if (!hook_proc_thid)
            hook_proc_thid = new std::atomic<DWORD>;

        if (!Keyboard_Deskriptor_Mutex)
            Keyboard_Deskriptor_Mutex = new std::mutex;

        if (!LLK_Proc_Additional_Callbacks_mutex)
            LLK_Proc_Additional_Callbacks_mutex = new std::mutex();

        if (!hook_main_th)
            hook_main_th = new std::thread(&hook_main);

        while (!hook_proc_thid->load()) Sleep(10);
    }

    void Hotkey_Manager::HKPP_Stop()
    {
        PostThreadMessageW(*hook_proc_thid, WM_QUIT, (WPARAM)NULL, (LPARAM)NULL);
        this->Clear_Hotkeys();

        LLK_Proc_Additional_Callbacks_mutex->lock();
        this->LLK_Proc_Additional_Callbacks.clear();
        LLK_Proc_Additional_Callbacks_mutex->unlock();
    }

    size_t Hotkey_Manager::Add_Callback(std::function <bool(int, WPARAM, LPARAM, VectorEx<key_deskriptor>&, bool)> fnc_p)
    {
        size_t uuid = 0;

        LLK_Proc_Additional_Callbacks_mutex->lock();
        LLK_Proc_Additional_Callbacks.push_back({ uuid ,fnc_p });
        uuid = LLK_Proc_Additional_Callbacks.size();
        LLK_Proc_Additional_Callbacks_mutex->unlock();

        return uuid;
    }

    void Hotkey_Manager::Remove_Callback(size_t uuid)
    {
        LLK_Proc_Additional_Callbacks_mutex->lock();
        LLK_Proc_Additional_Callbacks.Rem_If([&](callback_descriptor_t ds) -> bool { return(ds.uuid == uuid); });
        LLK_Proc_Additional_Callbacks_mutex->unlock();
    }

    void Hotkey_Manager::Clear_Callbacks()
    {
        LLK_Proc_Additional_Callbacks_mutex->lock();
        LLK_Proc_Additional_Callbacks.clear();
        LLK_Proc_Additional_Callbacks_mutex->unlock();
    }

    size_t Hotkey_Manager::Add_Hotkey(Hotkey_Deskriptor desk)
    {
        size_t uuid = 0;

        comb_vec_mutex->lock();
        {
            if (!Combinations.Contains(desk))
            {
                desk.Key_List.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });

                if (desk.settings.Allow_Injected == HKPP_DENY_INJECTED)
                    desk.Key_List.foreach([&](key_deskriptor& k) -> void { k.Injected = HKPP::injected_status_enm::NOT_INJECTED; });
                else
                    desk.Key_List.foreach([&](key_deskriptor& k) -> void { k.Injected = HKPP::injected_status_enm::UNDEFINED_INJECTION_STATUS; });

                if (!desk.settings.Thread_Id)
                    desk.settings.Thread_Id = hook_proc_thid->load();

                if (!desk.settings.Thread_Id)
                    desk.settings.Thread_Id = this->hook_proc_thid->load();

                desk.settings.uuid = Combinations.size() + 1;
                Combinations.push_back(desk);
                uuid = Combinations.size();
            }
        }
        comb_vec_mutex->unlock();

        return uuid;
    }

    void Hotkey_Manager::Remove_Hotkey(size_t uuid)
    {
        comb_vec_mutex->lock();
        Combinations.Rem_If([&](Hotkey_Deskriptor ds) -> bool { return(ds.settings.uuid == uuid); });
        comb_vec_mutex->unlock();
    }

    Hotkey_Deskriptor Hotkey_Manager::Get_Hotkey(size_t uuid)
    {
        Hotkey_Deskriptor rt({}, {});

        comb_vec_mutex->lock();
        Combinations.foreach([&](Hotkey_Deskriptor ds) -> void { if (ds.settings.uuid == uuid) rt = ds; });
        comb_vec_mutex->unlock();

        return rt;
    }

    void Hotkey_Manager::Clear_Hotkeys()
    {
        comb_vec_mutex->lock();
        Combinations.clear();
        comb_vec_mutex->unlock();
    }
}