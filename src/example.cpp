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

#ifdef _DEBUG

#include "hotkeyPP.hpp"

using namespace HKPP;
using namespace HKPP::extra;

int main(int argc, char** argv)
{


    Manager* mng = HKPP::Manager::GetInstance();

    mng->HKPP_Init();

    size_t hId = mng->RegisterHotkey(HKPP::Hotkey(
        { 'C' },
        [](HotkeyEvent evt) -> void {
            std::cout << "Hotkey pressed, working..." << std::endl;
            Sleep(10'000);
            std::cout << "Hotkey Done" << std::endl;
        },
        kbd_event_propagation::PROPAGATE,
        HKPP::Hotkey::injection_permission::ALLOW_ALL,
        HKPP::Hotkey::parallel_execution::ALLOW
    )
    );

    while (1) Sleep(10);

    mng->UnregisterHotkey(hId);

    return 0;
}
#endif