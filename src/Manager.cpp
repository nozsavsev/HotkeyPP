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
#include "hotkeyPP.h"

using namespace HKPP::extra;


namespace HKPP
{

    Manager* Manager::instance;

    std::atomic<DWORD>* Manager::hook_proc_thid;

    VectorEx<Key> Manager::GetKeyboardState()
    {
        instance->keyboardState_mutex->lock();
        VectorEx<Key> tmp = instance->keyboardState;
        instance->keyboardState_mutex->unlock();

        return tmp;
    }

    Manager::Manager()
    {
        this->HKPP_Init();
    }

    void Manager::runUserCallback(HotkeyEvent Evt, std::function <void(HotkeyEvent)> Callback, size_t Id)
    {
        Callback(Evt);

        Manager::instance->HKPP_CallbackHandles_mutex->lock();
        Manager::instance->HKPP_CallbackHandles.RemIf([&](HotkeyCallbackHandle& handle) -> bool { return handle.hotkeyId == Id; });
        Manager::instance->HKPP_CallbackHandles_mutex->unlock();
    }

    Manager* Manager::GetInstance()
    {
        if (instance == NULL)
            instance = new Manager();

        return instance;
    }

    void Manager::hook_main()
    {
        *hook_proc_thid = GetCurrentThreadId();

        HHOOK hook_handle = SetWindowsHookExW(WH_KEYBOARD_LL, Manager::LowLevelKeyboardProc, NULL, NULL);
        MSG msg;

        if (!hook_handle)
            std::cerr << std::endl << "HKPP_ERROR: Hotkey_Manager was unable to start due to unknown error | code" << GetLastError() << std::endl;

        while (GetMessageW(&msg, NULL, NULL, NULL))
        {
            TranslateMessage(&msg);

            if (msg.message == WM_QUIT)
                break;

            DispatchMessageW(&msg);
        }

        if (hook_handle != nullptr)
        {
            UnhookWindowsHookEx(hook_handle);
            hook_handle == nullptr;
        }

        hook_proc_thid = 0;
    }

    LRESULT CALLBACK Manager::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (nCode == HC_ACTION)
        {

            //get millisecond timestamp
            auto start_time = std::chrono::high_resolution_clock::now();

            bool repeated_input = false;
            kbd_event_propagation block_input = kbd_event_propagation::PROPAGATE;

            ///*
            instance->hotkeys_mutex->lock();
            VectorEx <Hotkey> LocalHotkeyList = instance->hotkeys;
            instance->hotkeys_mutex->unlock();
            
            instance->keyboardState_mutex->lock();

            KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;

            Key key_desk(kbd->vkCode);

            key_desk.time = std::chrono::high_resolution_clock::now();

            if ((kbd->flags & LLKHF_INJECTED) == LLKHF_INJECTED)
                key_desk.injected = injection_status::INJECTED;
            else if ((kbd->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED)
                key_desk.injected = injection_status::LL_INJECTED;
            else
                key_desk.injected = injection_status::REAL;


            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
                if (instance->keyboardState.Contains(key_desk) == false)
                {
                    instance->keyboardState.push_back(key_desk);
                    instance->keyboardState.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });
                }
                else
                    repeated_input = true;

            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
                instance->keyboardState.RemAll(key_desk);

            auto LocalKeyboardState = instance->keyboardState;

            instance->keyboardState_mutex->unlock();

            if (!repeated_input && false)
            {

                std::cout << "\033[1;1H";
                for (int i = 4; i-- > 0; std::cout << "\033[2K\n");
                std::cout << "\033[1;1H";

                for (const auto& key : LocalKeyboardState)
                {
                    std::cout << "[ " << key.key << " ] ";
                }

            }


            if (repeated_input == false)
                LocalHotkeyList.Foreach([&](Hotkey& hotkey) -> void
                    {
                        if (hotkey.checkAndDispatch(LocalKeyboardState) == kbd_event_propagation::BLOCK)
                        {
                            block_input = kbd_event_propagation::BLOCK;
                            std::cout << "Propagation blocked by one of hotkeys" << std::endl;
                        }
                    });

            instance->LLKP_AdditionalCallbacks_mutex->lock();
            instance->LLKP_AdditionalCallbacks.Foreach([&](KBDEventCallback& dsk) -> void
                {
                    if (dsk.callbackFunction(nCode, wParam, lParam, LocalKeyboardState, repeated_input) == kbd_event_propagation::BLOCK)
                    {
                        block_input = kbd_event_propagation::BLOCK;
                        std::cout << "Propagation blocked by one of callbacks" << std::endl;
                    }
                });
            instance->LLKP_AdditionalCallbacks_mutex->unlock();

            //*/
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << "Duration: " << duration.count() << "ms" << std::endl;

            return block_input | CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        else
            return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    void Manager::HKPP_Init()
    {
        if (hotkeys_mutex == nullptr)
            hotkeys_mutex = new std::mutex;

        if (hook_proc_thid == nullptr)
            hook_proc_thid = new std::atomic<DWORD>(0);

        if (keyboardState_mutex == nullptr)
            keyboardState_mutex = new std::mutex;

        if (HKPP_CallbackHandles_mutex == nullptr)
            HKPP_CallbackHandles_mutex = new std::mutex();

        if (LLKP_AdditionalCallbacks_mutex == nullptr)
            LLKP_AdditionalCallbacks_mutex = new std::mutex();

        if (hook_main_thread == nullptr)
            hook_main_thread = new std::thread(&hook_main);

        while (hook_proc_thid->load() == 0) Sleep(10);
    }

    void Manager::HKPP_Stop()
    {
        PostThreadMessageW(*hook_proc_thid, WM_QUIT, (WPARAM)NULL, (LPARAM)NULL);
        while (hook_proc_thid->load() != 0) Sleep(10);

        UnregisterAllHotkeys();
        UnregisterAllCallbacks();
    }

    size_t Manager::getNewHotkeyId() { return hotkeyIdAutoincrement++; }
    size_t Manager::getNewCallbackId() { return callbackIdAutoincrement++; }



    size_t Manager::RegisterCallback(std::function <kbd_event_propagation(int, WPARAM, LPARAM, VectorEx<Key>&, bool)> fnc_p)
    {
        auto uuid = getNewCallbackId();

        LLKP_AdditionalCallbacks_mutex->lock();
        LLKP_AdditionalCallbacks.push_back(KBDEventCallback(uuid, fnc_p));
        LLKP_AdditionalCallbacks_mutex->unlock();

        return uuid;
    }

    void Manager::UnregisterCallback(size_t uuid)
    {
        LLKP_AdditionalCallbacks_mutex->lock();
        LLKP_AdditionalCallbacks.RemIf([&](KBDEventCallback ds) -> bool { return(ds.callbackId == uuid); });
        LLKP_AdditionalCallbacks_mutex->unlock();

    }

    void Manager::UnregisterAllCallbacks()
    {
        LLKP_AdditionalCallbacks_mutex->lock();
        LLKP_AdditionalCallbacks.clear();
        LLKP_AdditionalCallbacks_mutex->unlock();
    }




    size_t Manager::RegisterHotkey(Hotkey desk)
    {
        size_t uuid = 0;

        hotkeys_mutex->lock();
        if (!hotkeys.Contains(desk))
        {
            desk.keyList.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });
            desk.id = uuid;
            hotkeys.push_back(desk);
        }
        hotkeys_mutex->unlock();

        return uuid;
    }

    void Manager::UnregisterHotkey(size_t uuid)
    {
        hotkeys_mutex->lock();
        hotkeys.RemIf([&](Hotkey ds) -> bool { return(ds.id == uuid); });
        hotkeys_mutex->unlock();
    }

    std::optional<Hotkey> Manager::GetHotkey(size_t uuid)
    {
        hotkeys_mutex->lock();
        auto value = hotkeys.Find([uuid](Hotkey hk) -> bool { return hk.id == uuid; });
        hotkeys_mutex->unlock();

        return value;
    }

    void Manager::UnregisterAllHotkeys()
    {
        hotkeys_mutex->lock();
        hotkeys.clear();
        hotkeys_mutex->unlock();
    }

    bool Manager::UpdateHotkey(Hotkey hotkey)
    {
        hotkeys_mutex->lock();

        if (hotkeys.Contains(hotkey))
        {
            hotkeys.RemAll(hotkey);
            hotkeys.push_back(hotkey);
        }
        else
        {
            hotkeys_mutex->unlock();
            return false;
        }

        hotkeys_mutex->unlock();

        return true;
    }

}
