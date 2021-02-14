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

/*USAGE
#define _CRT_SECURE_NO_WARNINGS

#include "hotkeyPP.h"

using namespace HKPP;
using namespace HKPP::extra;

int main(int argc, char** argv)
{
    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();

    mng->HKPP_Init();
    
    size_t hId = mng->Add_Hotkey(HKPP::Hotkey_Deskriptor(
        { VK_LWIN , 'C' },
        Hotkey_Settings_t(L"My combination",
            [&](Hotkey_Deskriptor hkd) -> void
            {
                printf("%ws", hkd.settings.name.c_str());
            })));

    while (1);

    mng->Remove_Hotkey(hId);
    mng->HKPP_Stop();

    return 0;
}
//*/