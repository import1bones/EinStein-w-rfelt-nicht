#pragma once

#include "core/Types.h"

namespace Einstein {

enum class MouseButton {
    Left,
    Right,
    Middle
};

enum class Key {
    Up,
    Down,
    Left,
    Right,
    Enter,
    Escape,
    Space,
    N, // New Game
    U, // Undo
    R, // Redo
    Q, // Quit
    F11 // Fullscreen
};

class InputHandler {
public:
    InputHandler();
    ~InputHandler();
    
    // Initialization
    bool Initialize();
    
    // Event processing
    void ProcessEvents();
    
    // Mouse input
    bool IsMouseButtonPressed(MouseButton button) const;
    Position GetMousePosition() const;
    void SetMouseEnabled(bool enabled);
    bool IsMouseEnabled() const;
    
    // Keyboard input
    bool IsKeyPressed(Key key) const;
    
    // Coordinate transformation
    Position ScreenToBoard(const Position& screen_pos) const;
    Position BoardToScreen(const Position& board_pos) const;
    
private:
    bool mouse_enabled_;
};

} // namespace Einstein
