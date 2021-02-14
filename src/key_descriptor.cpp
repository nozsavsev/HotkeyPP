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
    key_deskriptor::key_deskriptor()
    {
        Key = 0;
        Injected = injected_status_enm::UNDEFINED_INJECTION_STATUS;
    }

    key_deskriptor::key_deskriptor(DWORD key_ARG, injected_status_enm injected_ARG)
    {
        Key = key_ARG;
        Injected = injected_ARG;
    }

    key_deskriptor::key_deskriptor(DWORD key_ARG)
    {
        Key = key_ARG;
        Injected = injected_status_enm::UNDEFINED_INJECTION_STATUS;
    }


    bool key_deskriptor::operator== (key_deskriptor& s)
    {
        if ((this->Injected == injected_status_enm::UNDEFINED_INJECTION_STATUS || s.Injected == injected_status_enm::UNDEFINED_INJECTION_STATUS)) //if we do not know injection status
            return (s.Key == this->Key); //compare just codes
        else //if we know injection status
            return ((s.Key == this->Key) && (s.Injected == this->Injected)); //full comparation
    }
    bool key_deskriptor::operator!= (key_deskriptor& s) { return !(operator==(s)); }



    bool key_deskriptor::operator> (key_deskriptor& s)
    {
        if ((this->Injected == injected_status_enm::UNDEFINED_INJECTION_STATUS || s.Injected == injected_status_enm::UNDEFINED_INJECTION_STATUS))
            return (s.Key > this->Key);
        else
            return ((s.Key > this->Key) && (s.Injected == this->Injected));
    }
    bool key_deskriptor::operator<  (key_deskriptor& s) { return !(operator>(s)); }


    bool key_deskriptor::operator<= (key_deskriptor& s)
    {
        if ((this->Injected == injected_status_enm::UNDEFINED_INJECTION_STATUS || s.Injected == injected_status_enm::UNDEFINED_INJECTION_STATUS))
            return (s.Key <= this->Key);
        else
            return ((s.Key <= this->Key) && (s.Injected == this->Injected));
    }
    bool key_deskriptor::operator>= (key_deskriptor& s) { return (operator<=(s)); }


    bool key_deskriptor::operator== (DWORD& s) { return (s == this->Key); }
    bool key_deskriptor::operator!= (DWORD& s) { return !(operator==(s)); }

    bool key_deskriptor::operator>  (DWORD& s) { return (s > this->Key); }
    bool key_deskriptor::operator<  (DWORD& s) { return !(operator>(s)); }

    bool key_deskriptor::operator<= (DWORD& s) { return (s <= this->Key); }
    bool key_deskriptor::operator>= (DWORD& s) { return !(operator<=(s)); }
}
