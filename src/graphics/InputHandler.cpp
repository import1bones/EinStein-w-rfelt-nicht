#include "graphics/InputHandler.h"
#include <iostream>
#include <SDL2/SDL.h>

namespace Einstein {

InputHandler::InputHandler() : mouse_enabled_(true) {
    // Initialize key states
    for (auto& state : key_states_) {
        state = false;
    }
    for (auto& state : mouse_button_states_) {
        state = false;
    }
    mouse_position_ = std::make_pair(0, 0);
}

InputHandler::~InputHandler() = default;

bool InputHandler::Initialize() {
    std::cout << "Input handler initialized" << std::endl;
    return true;
}

void InputHandler::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                // Handle quit event
                break;
            case SDL_KEYDOWN:
                HandleKeyEvent(event.key, true);
                break;
            case SDL_KEYUP:
                HandleKeyEvent(event.key, false);
                break;
            case SDL_MOUSEBUTTONDOWN:
                HandleMouseButtonEvent(event.button, true);
                break;
            case SDL_MOUSEBUTTONUP:
                HandleMouseButtonEvent(event.button, false);
                break;
            case SDL_MOUSEMOTION:
                HandleMouseMotionEvent(event.motion);
                break;
        }
    }
}

bool InputHandler::IsMouseButtonPressed(MouseButton button) const {
    if (!mouse_enabled_) return false;
    
    switch (button) {
        case MouseButton::Left:
            return mouse_button_states_[0];
        case MouseButton::Right:
            return mouse_button_states_[1];
        case MouseButton::Middle:
            return mouse_button_states_[2];
        default:
            return false;
    }
}

Position InputHandler::GetMousePosition() const {
    return mouse_position_;
}

bool InputHandler::IsKeyPressed(Key key) const {
    switch (key) {
        case Key::Escape:
            return key_states_[0];
        case Key::Space:
            return key_states_[1];
        case Key::Enter:
            return key_states_[2];
        case Key::Left:
            return key_states_[3];
        case Key::Right:
            return key_states_[4];
        case Key::Up:
            return key_states_[5];
        case Key::Down:
            return key_states_[6];
        case Key::N:
            return key_states_[7];
        case Key::U:
            return key_states_[8];
        case Key::R:
            return key_states_[9];
        default:
            return false;
    }
}

void InputHandler::SetMouseEnabled(bool enabled) {
    mouse_enabled_ = enabled;
}

bool InputHandler::IsMouseEnabled() const {
    return mouse_enabled_;
}

Position InputHandler::ScreenToBoard(const Position& screen_pos) const {
    // Convert screen coordinates to board coordinates
    // Assume board is centered and scaled appropriately
    const int board_offset_x = 100;  // Offset from screen edge
    const int board_offset_y = 100;
    const int cell_size = 80;        // Size of each board cell
    
    int board_x = (screen_pos.first - board_offset_x) / cell_size;
    int board_y = (screen_pos.second - board_offset_y) / cell_size;
    
    // Clamp to board bounds
    board_x = std::max(0, std::min(4, board_x));
    board_y = std::max(0, std::min(4, board_y));
    
    return std::make_pair(board_x, board_y);
}

Position InputHandler::BoardToScreen(const Position& board_pos) const {
    // Convert board coordinates to screen coordinates
    const int board_offset_x = 100;
    const int board_offset_y = 100;
    const int cell_size = 80;
    
    int screen_x = board_offset_x + board_pos.first * cell_size + cell_size / 2;
    int screen_y = board_offset_y + board_pos.second * cell_size + cell_size / 2;
    
    return std::make_pair(screen_x, screen_y);
}

void InputHandler::HandleKeyEvent(const SDL_KeyboardEvent& key_event, bool pressed) {
    switch (key_event.keysym.sym) {
        case SDLK_ESCAPE:
            key_states_[0] = pressed;
            break;
        case SDLK_SPACE:
            key_states_[1] = pressed;
            break;
        case SDLK_RETURN:
            key_states_[2] = pressed;
            break;
        case SDLK_LEFT:
            key_states_[3] = pressed;
            break;
        case SDLK_RIGHT:
            key_states_[4] = pressed;
            break;
        case SDLK_UP:
            key_states_[5] = pressed;
            break;
        case SDLK_DOWN:
            key_states_[6] = pressed;
            break;
    }
}

void InputHandler::HandleMouseButtonEvent(const SDL_MouseButtonEvent& button_event, bool pressed) {
    if (!mouse_enabled_) return;
    
    switch (button_event.button) {
        case SDL_BUTTON_LEFT:
            mouse_button_states_[0] = pressed;
            break;
        case SDL_BUTTON_RIGHT:
            mouse_button_states_[1] = pressed;
            break;
        case SDL_BUTTON_MIDDLE:
            mouse_button_states_[2] = pressed;
            break;
    }
}

void InputHandler::HandleMouseMotionEvent(const SDL_MouseMotionEvent& motion_event) {
    if (mouse_enabled_) {
        mouse_position_ = std::make_pair(motion_event.x, motion_event.y);
    }
}

} // namespace Einstein
