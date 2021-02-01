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

/* USAGE
#define _CRT_SECURE_NO_WARNINGS

#include "hotkeyPP.h"

using namespace HKPP;
using namespace HKPP::extra;

int main(int argc, char** argv)
{
    Hotkey_Manager* mng = HKPP::Hotkey_Manager::Get_Instance();

    mng->HKPP_Init();

    mng->Add(HKPP::Hotkey_Deskriptor({ VK_LWIN , 'C' }, Hotkey_Settings_t(GetCurrentThreadId(), HKPP_BLOCK_INPUT, HKPP_ALLOW_INJECTED, WM_HKPP_DEFAULT_CALLBACK_MESSAGE,L"name") ));

    MSG msg;
    while (GetMessageW(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);


        if (msg.message == WM_HKPP_DEFAULT_CALLBACK_MESSAGE)
        {
            if (msg.lParam)
            {
                Hotkey_Deskriptor* dsk = (Hotkey_Deskriptor*)msg.lParam;

                wprintf(L"pressed %s real:%s\n",dsk->settings.name.c_str(), dsk->Real ? L"true" : L"false");

                delete dsk;
            }
        }


        DispatchMessageW(&msg);
    }

    return 0;
}
*/