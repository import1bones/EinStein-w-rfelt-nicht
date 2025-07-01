#pragma once

// Forward declarations for SDL types (for header-only interface)
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Color;
struct _TTF_Font;
using TTF_Font = _TTF_Font;

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <map>

namespace Einstein {

// Forward declarations
struct GameConfig;
class ChessBoard;
enum class Player : int8_t;

struct Color {
    uint8_t r, g, b, a;
    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

struct Point {
    int x, y;
    Point(int x_pos = 0, int y_pos = 0) : x(x_pos), y(y_pos) {}
};

struct Rect {
    int x, y, w, h;
    Rect(int x_pos = 0, int y_pos = 0, int width = 0, int height = 0)
        : x(x_pos), y(y_pos), w(width), h(height) {}
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialization and cleanup
    bool Initialize(const std::string& title, int width, int height, bool fullscreen = false);
    void Shutdown();

    // Frame management
    void BeginFrame();
    void EndFrame();
    void Clear(const Color& color = Color(0, 0, 0));

    // Basic drawing
    void DrawRect(const Rect& rect, const Color& color, bool filled = true);
    void DrawLine(const Point& start, const Point& end, const Color& color, int thickness = 1);
    void DrawCircle(const Point& center, int radius, const Color& color, bool filled = true);

    // Text rendering
    bool LoadFont(const std::string& font_path, int size, const std::string& name = "default");
    void DrawText(const std::string& text, const Point& position, const Color& color, 
                  const std::string& font_name = "default");
    Point MeasureText(const std::string& text, const std::string& font_name = "default");

    // Game-specific rendering
    void DrawChessBoard(const Rect& board_area);
    void DrawPiece(int piece_number, Player player, const Point& position, int size);
    void DrawHighlight(const Point& position, int size, const Color& color);

    // Window management
    void SetWindowTitle(const std::string& title);
    Point GetWindowSize() const;
    bool IsFullscreen() const;
    void SetFullscreen(bool fullscreen);

    // Utility
    bool IsInitialized() const { return initialized_; }
    SDL_Renderer* GetSDLRenderer() const { return renderer_; }

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    std::map<std::string, TTF_Font*> fonts_;
    bool initialized_;
    bool fullscreen_;
    int window_width_;
    int window_height_;

    // Helper functions
    SDL_Color ToSDLColor(const Color& color) const;
    void DrawFilledCircle(const Point& center, int radius, const Color& color);
    void DrawCircleOutline(const Point& center, int radius, const Color& color);
};

} // namespace Einstein
