#include "graphics/Renderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cmath>

namespace Einstein {

Renderer::Renderer() 
    : window_(nullptr)
    , renderer_(nullptr)
    , initialized_(false)
    , fullscreen_(false)
    , window_width_(800)
    , window_height_(600) {
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize(const std::string& title, int width, int height, bool fullscreen) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Initialize SDL_image
    int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    window_width_ = width;
    window_height_ = height;
    fullscreen_ = fullscreen;

    // Create window
    Uint32 window_flags = SDL_WINDOW_SHOWN;
    if (fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    }

    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        window_flags
    );

    if (window_ == nullptr) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // Create renderer
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // Set default draw color
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);

    initialized_ = true;
    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

void Renderer::Shutdown() {
    if (!initialized_) return;

    // Cleanup fonts
    for (auto& font_pair : fonts_) {
        TTF_CloseFont(font_pair.second);
    }
    fonts_.clear();

    // Cleanup SDL
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    initialized_ = false;
    std::cout << "Renderer shutdown complete" << std::endl;
}

void Renderer::BeginFrame() {
    if (!initialized_) return;
    // Frame preparation is done in Clear()
}

void Renderer::EndFrame() {
    if (!initialized_) return;
    SDL_RenderPresent(renderer_);
}

void Renderer::Clear(const Color& color) {
    if (!initialized_) return;
    
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer_);
}

void Renderer::DrawRect(const Rect& rect, const Color& color, bool filled) {
    if (!initialized_) return;

    SDL_Rect sdl_rect = {rect.x, rect.y, rect.w, rect.h};
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);

    if (filled) {
        SDL_RenderFillRect(renderer_, &sdl_rect);
    } else {
        SDL_RenderDrawRect(renderer_, &sdl_rect);
    }
}

void Renderer::DrawLine(const Point& start, const Point& end, const Color& color, int thickness) {
    if (!initialized_) return;

    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    
    if (thickness == 1) {
        SDL_RenderDrawLine(renderer_, start.x, start.y, end.x, end.y);
    } else {
        // Draw thick line by drawing multiple parallel lines
        int dx = end.x - start.x;
        int dy = end.y - start.y;
        double length = std::sqrt(dx * dx + dy * dy);
        
        if (length > 0) {
            double offset_x = -dy / length * (thickness / 2.0);
            double offset_y = dx / length * (thickness / 2.0);
            
            for (int i = 0; i < thickness; ++i) {
                double t = i / static_cast<double>(thickness - 1) - 0.5;
                int ox = static_cast<int>(offset_x * t * 2);
                int oy = static_cast<int>(offset_y * t * 2);
                
                SDL_RenderDrawLine(renderer_, 
                    start.x + ox, start.y + oy, 
                    end.x + ox, end.y + oy);
            }
        }
    }
}

void Renderer::DrawCircle(const Point& center, int radius, const Color& color, bool filled) {
    if (!initialized_) return;

    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);

    if (filled) {
        DrawFilledCircle(center, radius, color);
    } else {
        DrawCircleOutline(center, radius, color);
    }
}

bool Renderer::LoadFont(const std::string& font_path, int size, const std::string& name) {
    if (!initialized_) return false;

    TTF_Font* font = TTF_OpenFont(font_path.c_str(), size);
    if (font == nullptr) {
        std::cerr << "Failed to load font " << font_path << "! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Close existing font if it exists
    auto it = fonts_.find(name);
    if (it != fonts_.end()) {
        TTF_CloseFont(it->second);
    }

    fonts_[name] = font;
    return true;
}

void Renderer::DrawText(const std::string& text, const Point& position, const Color& color, const std::string& font_name) {
    if (!initialized_ || text.empty()) return;

    auto it = fonts_.find(font_name);
    if (it == fonts_.end()) {
        std::cerr << "Font '" << font_name << "' not found!" << std::endl;
        return;
    }

    SDL_Color sdl_color = ToSDLColor(color);
    SDL_Surface* text_surface = TTF_RenderText_Solid(it->second, text.c_str(), sdl_color);
    if (text_surface == nullptr) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer_, text_surface);
    if (text_texture == nullptr) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(text_surface);
        return;
    }

    int text_width = text_surface->w;
    int text_height = text_surface->h;
    SDL_FreeSurface(text_surface);

    SDL_Rect render_rect = {position.x, position.y, text_width, text_height};
    SDL_RenderCopy(renderer_, text_texture, nullptr, &render_rect);
    SDL_DestroyTexture(text_texture);
}

Point Renderer::MeasureText(const std::string& text, const std::string& font_name) {
    if (!initialized_ || text.empty()) return Point(0, 0);

    auto it = fonts_.find(font_name);
    if (it == fonts_.end()) {
        return Point(0, 0);
    }

    int width, height;
    if (TTF_SizeText(it->second, text.c_str(), &width, &height) != 0) {
        return Point(0, 0);
    }

    return Point(width, height);
}

void Renderer::DrawChessBoard(const Rect& board_area) {
    if (!initialized_) return;

    // Draw board background
    DrawRect(board_area, Color(139, 69, 19), true); // Brown background

    // Draw grid lines
    Color line_color(0, 0, 0); // Black lines
    int cell_width = board_area.w / 5;
    int cell_height = board_area.h / 5;

    // Vertical lines
    for (int i = 0; i <= 5; ++i) {
        int x = board_area.x + i * cell_width;
        DrawLine(Point(x, board_area.y), Point(x, board_area.y + board_area.h), line_color, 2);
    }

    // Horizontal lines
    for (int i = 0; i <= 5; ++i) {
        int y = board_area.y + i * cell_height;
        DrawLine(Point(board_area.x, y), Point(board_area.x + board_area.w, y), line_color, 2);
    }
}

void Renderer::DrawPiece(int piece_number, Player player, const Point& position, int size) {
    if (!initialized_) return;

    Color piece_color = (static_cast<int8_t>(player) < 0) ? Color(255, 0, 0) : Color(0, 0, 255); // Red for LT, Blue for RB
    
    // Draw piece as filled circle
    DrawCircle(position, size / 2, piece_color, true);
    
    // Draw piece number
    if (fonts_.find("default") != fonts_.end()) {
        std::string number_str = std::to_string(piece_number);
        Point text_size = MeasureText(number_str, "default");
        Point text_pos(position.x - text_size.x / 2, position.y - text_size.y / 2);
        DrawText(number_str, text_pos, Color(255, 255, 255), "default");
    }
}

void Renderer::DrawHighlight(const Point& position, int size, const Color& color) {
    if (!initialized_) return;

    // Draw highlight as circle outline
    DrawCircle(position, size / 2 + 5, color, false);
}

void Renderer::SetWindowTitle(const std::string& title) {
    if (window_) {
        SDL_SetWindowTitle(window_, title.c_str());
    }
}

Point Renderer::GetWindowSize() const {
    return Point(window_width_, window_height_);
}

bool Renderer::IsFullscreen() const {
    return fullscreen_;
}

void Renderer::SetFullscreen(bool fullscreen) {
    if (!window_ || fullscreen_ == fullscreen) return;

    fullscreen_ = fullscreen;
    SDL_SetWindowFullscreen(window_, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}

// Private helper functions

SDL_Color Renderer::ToSDLColor(const Color& color) const {
    return {color.r, color.g, color.b, color.a};
}

void Renderer::DrawFilledCircle(const Point& center, int radius, const Color& color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer_, center.x + x, center.y + y);
            }
        }
    }
}

void Renderer::DrawCircleOutline(const Point& center, int radius, const Color& color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    while (y >= x) {
        // Draw points in all octants
        SDL_RenderDrawPoint(renderer_, center.x + x, center.y + y);
        SDL_RenderDrawPoint(renderer_, center.x - x, center.y + y);
        SDL_RenderDrawPoint(renderer_, center.x + x, center.y - y);
        SDL_RenderDrawPoint(renderer_, center.x - x, center.y - y);
        SDL_RenderDrawPoint(renderer_, center.x + y, center.y + x);
        SDL_RenderDrawPoint(renderer_, center.x - y, center.y + x);
        SDL_RenderDrawPoint(renderer_, center.x + y, center.y - x);
        SDL_RenderDrawPoint(renderer_, center.x - y, center.y - x);
        
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

} // namespace Einstein
