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

    HKPPHotkey::HKPPHotkey(VectorEx <HKPPKey> KeyList, HotkeyConfig Config)
    {
        config = Config;
        keyList = KeyList;
        keyList.Sort([&](auto d1, auto d2) -> bool { return (d1 < d2); });
    }

    bool HKPPHotkey::Check_Combination(VectorEx <HKPPKey>& KState)
    {
        for (size_t i = 0; i < this->keyList.size(); i++)
            if (!KState.Contains(keyList[i].key))
                return false;

        return true;
    }

    bool HKPPHotkey::operator== (HKPPHotkey& s)
    {
        return (
            (this->keyList == s.keyList) &&
            (this->config.allowInjected == s.config.allowInjected)
            );
    }

    bool HKPPHotkey::operator!= (HKPPHotkey& s) { return !operator==(s); }
}