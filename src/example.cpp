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

///*USAGE
#define _CRT_SECURE_NO_WARNINGS

#include "hotkeyPP.h"

using namespace HKPP;
using namespace HKPP::extra;


class HotkeyCallbackHandle_ {
public:
    HotkeyCallbackHandle_() = default;
    HotkeyCallbackHandle_(const HotkeyCallbackHandle_& other) : hotkeyId(other.hotkeyId), future(std::move(const_cast<HotkeyCallbackHandle_&>(other).future)) {}
    HotkeyCallbackHandle_(HotkeyCallbackHandle_&& other) noexcept : hotkeyId(std::exchange(other.hotkeyId, 0)), future(std::move(other.future)) {}

    HotkeyCallbackHandle_(size_t HotkeyId, std::future<void> Future)
    {
        hotkeyId = HotkeyId;
        std::swap(Future, future);
    }

    HotkeyCallbackHandle_& operator=(HotkeyCallbackHandle_ other) {
        std::swap(hotkeyId, other.hotkeyId);
        std::swap(future, other.future);
        return *this;
    }

    std::future<void> future;
    size_t hotkeyId = 0;

    bool operator==(const HotkeyCallbackHandle_& s) const {
        return hotkeyId == s.hotkeyId;
    }

    bool operator!=(const HotkeyCallbackHandle_& s) const {
        return !(*this == s);
    }
};


int main(int argc, char** argv)
{
    ///*


    Manager* mng = HKPP::Manager::GetInstance();

    mng->HKPP_Init();

    size_t hId = mng->RegisterHotkey(HKPP::Hotkey(
        { 'C' , 'V' },
        [](HotkeyEvent evt) -> void {
            std::cout << "Hotkey pressed working..." << std::endl;
            Sleep(10'000);
            std::cout << "Done" << std::endl;     


        },               
        kbd_event_propagation::PROPAGATE,
        HKPP::Hotkey::injection_permission::ALLOW_ALL,
        HKPP::Hotkey::parallel_execution::BLOCK
    )
    );

    while (1);

    mng->UnregisterHotkey(hId);
    mng->HKPP_Stop();

    //*/

    return 0;
}
//*/