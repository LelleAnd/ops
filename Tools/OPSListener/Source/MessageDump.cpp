
#include "MessageDump.h"

namespace message_dump {

    std::string Trim(std::string const str)
    {
        const std::string::size_type pos1 = str.find_first_not_of(' ');
        const std::string::size_type pos2 = str.find_last_not_of(' ');
        if ((pos1 == pos2) && (pos1 == std::string::npos)) { return ""; }
        return str.substr(pos1 == std::string::npos ? 0 : pos1,
            pos2 == std::string::npos ? str.length() - 1 : pos2 - pos1 + 1);
    }

    std::ostream& operator<<(std::ostream& os, const indent& ind) {
        os << std::setw(4 + (ind.level * 2)) << " ";
        return os;
    }

}
