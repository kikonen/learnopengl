#pragma once

#include <unordered_map>

class Window;

enum class Key : std::underlying_type_t<std::byte> {
    EXIT,
    FULL_SCREEN_TOGGLE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD,
    ROTATE_LEFT,
    ROTATE_RIGHT,
    ZOOM_IN,
    ZOOM_OUT,
};

enum class Modifier : std::underlying_type_t<std::byte> {
    SHIFT,
    CONTROL,
    ALT,
};

struct InputState {
    bool ctrl{ false };
    bool shift{ false };
    bool alt{ false };
    bool mouseLeft{ false };
    bool mouseRight{ false };
};

//
// Handle keyboard mapping
//
class Input final
{
public:
    Input(Window* window);
    ~Input();

    void prepare();

    void updateKeyStates();

    bool isKeyDown(Key key) const noexcept;
    bool isModifierDown(Modifier modifier) const noexcept;

    bool isModifierPressed(Modifier modifier) const noexcept;

    void onMouseMove(float xpos, float ypos);
    void onMouseButton(int button, int action, int modifiers);

    inline bool allowMouse() const noexcept
    {
        return !(imGuiHasMouse || imGuiHasKeyboard);
    }

    inline bool allowKeyboard() const noexcept
    {
        return !imGuiHasKeyboard;
    }

public:
    float mouseX = 0;
    float mouseY = 0;

    float mouseXoffset = 0;
    float mouseYoffset = 0;

    bool imGuiHasKeyboard{ false };
    bool imGuiHasMouse{ false };

    Window* window;

private:
    bool m_prepared = false;

    std::unordered_map<Key, int*> m_keyMappings;

    std::unordered_map<Modifier, int*> m_modifierMappings;

    std::unordered_map<Modifier, bool> m_modifierDown;
    std::unordered_map<Modifier, bool> m_modifierPressed;
    std::unordered_map<Modifier, bool> m_modifierReleased;

    bool m_firstMouse = true;

};
