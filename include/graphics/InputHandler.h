#pragma once

#include "core/Types.h"
#include <array>

// Forward declaration for SDL types
struct SDL_KeyboardEvent;
struct SDL_MouseButtonEvent;
struct SDL_MouseMotionEvent;

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
    
    // Input state tracking
    std::array<bool, 10> key_states_;        // Track key states
    std::array<bool, 3> mouse_button_states_; // Track mouse button states
    Position mouse_position_;                 // Current mouse position
    
    // Event handlers
    void HandleKeyEvent(const SDL_KeyboardEvent& key_event, bool pressed);
    void HandleMouseButtonEvent(const SDL_MouseButtonEvent& button_event, bool pressed);
    void HandleMouseMotionEvent(const SDL_MouseMotionEvent& motion_event);
};

} // namespace Einstein
