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
    VectorEx <key_deskriptor> Hotkey_Manager::Local_Keyboard_Deskriptor = {};
    std::mutex* Hotkey_Manager::Local_Keyboard_Deskriptor_Mutex;

    Hotkey_Manager* Hotkey_Manager::instance;

    VectorEx <Hotkey_Deskriptor> Hotkey_Manager::Combinations;
    std::mutex* Hotkey_Manager::comb_vec_mutex;

    VectorEx <callback_descroptor_t> Hotkey_Manager::LLK_Proc_Additional_Callbacks;
    std::mutex* Hotkey_Manager::LLK_Proc_Additional_Callbacks_mutex;

    std::atomic<DWORD>* Hotkey_Manager::hook_proc_thid;

    VectorEx<key_deskriptor> Hotkey_Manager::GetKeyboardState()
    {
        Local_Keyboard_Deskriptor_Mutex->lock();
        VectorEx<key_deskriptor> tmp = Local_Keyboard_Deskriptor;
        Local_Keyboard_Deskriptor_Mutex->unlock();

        return tmp;
    }

    Hotkey_Manager::Hotkey_Manager()
    {
        comb_vec_mutex = new std::mutex;
        hook_proc_thid = new std::atomic<DWORD>;
        Local_Keyboard_Deskriptor_Mutex = new std::mutex;
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

        printf("Setting up LL keyboard hook\n");
        HHOOK hook_handle = SetWindowsHookExW(WH_KEYBOARD_LL, Hotkey_Manager::LowLevelKeyboardProc, NULL, NULL);
        MSG msg;

        if (!hook_handle)
        {
            printf("\nHotkey_Manager::hook_main() >> %d\n", GetLastError());
            std::exit(-1);
        }
        printf("Ready to rock!\n");

        while (GetMessageW(&msg, NULL, NULL, NULL))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        UnhookWindowsHookEx(hook_handle);
    }

    LRESULT CALLBACK Hotkey_Manager::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        bool repeated_input = false;

        bool block_input = false;

        if (nCode == HC_ACTION)
        {
            Local_Keyboard_Deskriptor_Mutex->lock();

            KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

            key_deskriptor key_desk;

            key_desk.Key = kbd->vkCode;
            key_desk.Injected = (((kbd->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED) || ((kbd->flags & LLKHF_INJECTED) == LLKHF_INJECTED)) ? HKPP::injected_status_enm::INJECTED : HKPP::injected_status_enm::NOT_INJECTED;


            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
            {
                if (!Local_Keyboard_Deskriptor.Contains(key_desk.Key))
                {
                    Local_Keyboard_Deskriptor.push_back(key_desk);  //# insert
                    Local_Keyboard_Deskriptor.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });
                }

                else
                {
                    repeated_input = true;

                    Local_Keyboard_Deskriptor.foreach([&](key_deskriptor& kd) -> void
                        {
                            if (kd.Key == key_desk.Key)
                            {
                                kd = key_desk;

                                repeated_input = false;
                            }
                        });
                }
            }

            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
            {
                Local_Keyboard_Deskriptor.Rem_All(kbd->vkCode);
            }


            comb_vec_mutex->lock();
            {

                VectorEx <Hotkey_Deskriptor> combs;

                Combinations.foreach([&](Hotkey_Deskriptor& desk) -> void
                    {

                        if (desk.Check_Combination(Local_Keyboard_Deskriptor))
                        {
                            if (desk.Key_List.Contains(key_desk))// only if key that pressed is in key list 
                            {
                                if (!repeated_input)
                                {
                                    desk.Real = true;

                                    for (size_t i = 0; i < desk.Key_List.size(); i++)
                                    {
                                        if (Local_Keyboard_Deskriptor.Contains({ desk.Key_List[i].Key, HKPP::injected_status_enm::INJECTED }) && !Local_Keyboard_Deskriptor.Contains({ desk.Key_List[i].Key, HKPP::injected_status_enm::NOT_INJECTED }))
                                        {
                                            desk.Real = false;
                                            break;
                                        }
                                    }
                                    combs.push_back(desk);
                                    //desk.Send_Event();
                                }
                                block_input |= desk.settings.Block_Input;
                            }
                        }
                    });

                combs.foreach([&](Hotkey_Deskriptor& d) -> void
                    {
                        bool send = true;

                        combs.foreach([&](Hotkey_Deskriptor& d_in) -> void { if (d != d_in && d.Check_Combination(d_in.Key_List)) { send = false; } });

                        if (send)
                            d.Send_Event();
                    });
            }

            comb_vec_mutex->unlock();

            Local_Keyboard_Deskriptor_Mutex->unlock();
        }



        LLK_Proc_Additional_Callbacks_mutex->lock();
        {
            LLK_Proc_Additional_Callbacks.foreach([&](callback_descroptor_t& dsk) -> void { block_input |= dsk.fnc(nCode, wParam, lParam); });
        }
        LLK_Proc_Additional_Callbacks_mutex->unlock();


        if (block_input)
            return 1;
        else
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    void Hotkey_Manager::HKPP_Init()
    {
        printf("starting threads\n");
        if (!hook_main_th)
            hook_main_th = new std::thread(&hook_main);

        if (!LLK_Proc_Additional_Callbacks_mutex)
            LLK_Proc_Additional_Callbacks_mutex = new std::mutex();
    }

    void Hotkey_Manager::HKPP_Stop()
    {
        PostThreadMessageW(*hook_proc_thid, WM_QUIT, (WPARAM)NULL, (LPARAM)NULL);
        this->Clear();

        LLK_Proc_Additional_Callbacks_mutex->lock();
        this->LLK_Proc_Additional_Callbacks.clear();
        LLK_Proc_Additional_Callbacks_mutex->unlock();
    }

    void Hotkey_Manager::Clear()
    {
        comb_vec_mutex->lock();
        Combinations.clear();
        comb_vec_mutex->unlock();
    }


    bool Hotkey_Manager::Add_Callback(std::function <bool(int, WPARAM, LPARAM)> fnc_p, size_t uuid_p)
    {
        //# == check
        LLK_Proc_Additional_Callbacks_mutex->lock();
        LLK_Proc_Additional_Callbacks.push_back({ uuid_p ,fnc_p });
        LLK_Proc_Additional_Callbacks_mutex->unlock();

        return true;
    }

    bool Hotkey_Manager::Add(Hotkey_Deskriptor desk)
    {
        comb_vec_mutex->lock();

        if (!Combinations.Contains(desk))
        {

            if (desk.settings.Allow_Injected == HKPP_DENY_INJECTED)
            {
                desk.Key_List.foreach([&](key_deskriptor& k) -> void { k.Injected = HKPP::injected_status_enm::NOT_INJECTED; });
            }

            else
            {
                desk.Key_List.foreach([&](key_deskriptor& k) -> void { k.Injected = HKPP::injected_status_enm::UNDEFINED_INJECTION_STATUS; });
            }

            desk.Key_List.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });

            Combinations.push_back(desk);

            comb_vec_mutex->unlock();

            return true;
        }
        else
            comb_vec_mutex->unlock();

        return false;
    }

    void Hotkey_Manager::Remove(Hotkey_Deskriptor desk)
    {
        comb_vec_mutex->lock();
        Combinations.Rem_All(desk);
        comb_vec_mutex->unlock();

    }
}