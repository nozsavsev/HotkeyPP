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

    Key::Key(DWORD key_ARG, injection_status injected_ARG)
    {
        key = key_ARG;
        injected = injected_ARG;
    }


    bool Key::operator== (const Key& s)const
    {
        return ((s.key == this->key) && (s.injected == this->injected));
    }

    bool Key::operator!= (const Key& s) const { return !(operator==(s)); }



    bool Key::operator> (const Key& s) const
    {
        return ((s.key > this->key) && (s.injected == this->injected));
    }

    bool Key::operator<  (const Key& s) const { return !(operator>(s)); }


    bool Key::operator<= (const Key& s) const
    {
        return ((s.key <= this->key) && (s.injected == this->injected));
    }

    bool Key::operator>= (const Key& s)const { return (operator<=(s)); }


    bool Key::operator== (const DWORD& s)const { return (s == this->key); }
    bool Key::operator!= (const DWORD& s)const { return !(operator==(s)); }

    bool Key::operator>  (const DWORD& s)const { return (s > this->key); }
    bool Key::operator<  (const DWORD& s)const { return !(operator>(s)); }

    bool Key::operator<= (const DWORD& s)const { return (s <= this->key); }
    bool Key::operator>= (const DWORD& s)const { return !(operator<=(s)); }
}
