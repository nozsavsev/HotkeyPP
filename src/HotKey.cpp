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

#include "hotkeyPP.hpp"

using namespace HKPP::extra;

namespace HKPP
{

    std::function <void(HotkeyEvent)> DefaultHotkeyCallback = [](HotkeyEvent hk)
        {
            std::cerr << std::endl << "HKPP ERROR: No Hotkey callback has been set for Hotkey: id = '" << hk.hotkey.id << "'" << std::endl;
        };


    Hotkey::Hotkey(
        VectorEx <Key> KeyList,
        std::function <void(HotkeyEvent)> Callback,
        kbd_event_propagation KbdEventPropagationStatus,
        injection_permission InjectionPermission,
        parallel_execution ParalelExecutionPolicy
    )
    {
        keyList = KeyList;
        keyList.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });
        paralelExecutionPolicy = ParalelExecutionPolicy;
        callback = Callback;
        allowedInjectionLevel = InjectionPermission;
        kbdEventPropagationStatus = KbdEventPropagationStatus;
    }



    kbd_event_propagation Hotkey::checkAndDispatch(VectorEx <Key>& KState)
    {
        injection_status weakestKey = injection_status::REAL;

        for (auto& key : this->keyList)
        {
            auto _kbKey = KState.Find([key](Key& k) -> bool { return k.key == key.key; });

            if (_kbKey.has_value())
            {
                Key kbKey = _kbKey.value();

                if ((int)kbKey.injected < (int)weakestKey)
                    weakestKey = kbKey.injected;

                if ((int)kbKey.injected >= (int)this->allowedInjectionLevel)
                    continue;
                else
                    return kbd_event_propagation::PROPAGATE;
            }
            else
                return kbd_event_propagation::PROPAGATE;
        }



        ///*
        Manager::instance->HKPP_CallbackHandles_mutex->lock();

        if (Manager::instance->HKPP_CallbackHandles.Contains([this](HotkeyCallbackHandle& handle) -> bool { return handle.hotkeyId == this->id; }))
            if (this->paralelExecutionPolicy == parallel_execution::BLOCK)
            {
                Manager::instance->HKPP_CallbackHandles_mutex->unlock();
                std::cout << "blocked execution" << std::endl;

                return kbd_event_propagation::PROPAGATE;
            }

        Manager::instance->HKPP_CallbackHandles_mutex->unlock();
        //*/

        HotkeyEvent hk_event(*this, weakestKey);
        HotkeyCallbackHandle handle(id, hk_event, callback);

        Manager::instance->HKPP_CallbackHandles_mutex->lock();
        Manager::instance->HKPP_CallbackHandles.push_back(handle);
        Manager::instance->HKPP_CallbackHandles_mutex->unlock();

        Manager::callbackQueue.Enqueue(handle);

        return this->kbdEventPropagationStatus;
    }

    bool Hotkey::operator== (const Hotkey& s) const
    {
        return ((this->id == s.id) || (this->keyList == s.keyList));
    }

    bool Hotkey::operator!= (const Hotkey& s) const { return !operator==(s); }


}