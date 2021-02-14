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

    void Hotkey_Deskriptor::Init(VectorEx <key_deskriptor> keys_vector, Hotkey_Settings_t set)
    {
        Key_List = keys_vector;
        Key_List.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });

        settings = set;
    }

    bool Hotkey_Deskriptor::Check_Combination(VectorEx <key_deskriptor>& KState)
    {
        for (size_t i = 0; i < this->Key_List.size(); i++)
            if (!KState.Contains(Key_List[i].Key))
                return false;

        return true;
    }

    void Hotkey_Deskriptor::Send_Event() noexcept
    {
        Hotkey_Deskriptor* temp_hotkey_descriptor = new Hotkey_Deskriptor(*this);

        PostThreadMessageW(this->settings.Thread_Id, this->settings.Msg, NULL, (LPARAM)temp_hotkey_descriptor);
    }

    bool Hotkey_Deskriptor::operator== (Hotkey_Deskriptor& s)
    {
        return (
            (this->Key_List == s.Key_List) &&
            (this->settings.Thread_Id == s.settings.Thread_Id) &&
            (this->settings.Allow_Injected == s.settings.Allow_Injected)
            );
    }
    bool Hotkey_Deskriptor::operator!= (Hotkey_Deskriptor& s) { return !operator==(s); }
}