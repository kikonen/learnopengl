#include "ConsoleState.h"

namespace {
    static void strtrim(char* s) {
        char* str_end = s + strlen(s);

        while (str_end > s && str_end[-1] == ' ')
        {
            str_end--;
            *str_end = 0;
        }
    }
}

namespace editor {

    void ConsoleState::trimInput()
    {
        strtrim(m_inputBuffer);
    }
}
