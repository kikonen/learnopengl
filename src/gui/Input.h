#pragma once

#include <unordered_map>
#include <type_traits>

class Window;

enum class Key : std::underlying_type_t<std::byte> {
    NONE,
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
    MOUSE_LOCK,
    KEY_COUNT,
};

enum class Modifier : std::underlying_type_t<std::byte> {
    NONE,
    SHIFT,
    CONTROL,
    ALT,
    KEY_COUNT,
};

struct ModifierMapping {
    Modifier key;
    int code{ 0 };
};

struct KeyMapping {
    Key key;
    int code{ 0 };
};

struct KeyState {
    bool wasDown{ false };
    bool down{ false };
    bool released{ false };
    bool pressed{ false };
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

    // @return true if any key was pressed
    bool updateKeyStates();

    // @return true if any key was pressed
    bool updateModifierStates();

    void updateMouseState();

    void addKeyMapping(
        Key key,
        const std::vector<int>& codes
    );

    void addModifierMapping(
        Modifier key,
        const std::vector<int>& codes
    );

    bool isKeyDown(Key key) const noexcept;
    bool isModifierDown(Modifier modifier) const noexcept;

    bool isModifierPressed(Modifier modifier) const noexcept;

    // @param button GLFW_MOUSE_BUTTON_LEFT | GLFW_MOUSE_BUTTON_RIGHT
    // @return true if button is pressed with modifier
    bool isMouseButtonPressed(int button) const noexcept;

    bool isHighPrecisionMode() const noexcept;

    bool isMouseCaptured() const noexcept;

    void onMouseMove(float xpos, float ypos);
    void onMouseWheel(float xoffset, float yoffset);

    inline bool allowMouse() const noexcept
    {
        return !(imGuiHasMouse || imGuiHasKeyboard);
    }

    inline bool allowKeyboard() const noexcept
    {
        return !imGuiHasKeyboard && !isReclaimFocus();
    }

    inline bool isReclaimFocus() const noexcept
    {
        return imGuiReclaimFocus > 0;
    }

    inline void reclaimFocus() noexcept
    {
        imGuiReclaimFocus = 2;
    }

    inline void claimedFocus() noexcept
    {
        if (imGuiReclaimFocus > 0)
            imGuiReclaimFocus--;
    }

    InputState getState() const;

public:
    float mouseX{ 0.f };
    float mouseY{ 0.f };

    float mousePreviousX{ 0.f };
    float mousePreviousY{ 0.f };
    bool mouseHasPosition{ false };

    // relative to previous mouse position
    float mouseRelativeX{ 0.f };
    float mouseRelativeY{ 0.f };

    float mouseWheelXOffset{ 0.f };
    float mouseWheelYOffset{ 0.f };

    bool imGuiHasKeyboard{ false };
    bool imGuiHasMouse{ false };
    int imGuiReclaimFocus{ 0 };

    Window* window;

private:
    bool m_prepared = false;

    std::vector<KeyMapping> m_keyMapping;
    std::vector<KeyState> m_keyStates;

    std::vector<ModifierMapping> m_modifierMapping;
    std::vector<KeyState> m_modifierStates;

    bool m_firstMouse = true;
};
