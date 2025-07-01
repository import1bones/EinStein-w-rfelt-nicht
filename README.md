# Einstein würfelt nicht! - Modern Edition

A cross-platform, high-performance implementation of the classic board game "Einstein würfelt nicht!" with advanced AI capabilities.

## 🎮 About the Game

Einstein würfelt nicht! (Einstein doesn't roll dice!) is a strategic board game where players move pieces based on dice rolls. The goal is to get one of your pieces to the opponent's corner or capture all enemy pieces.

## ✨ Features

### 🚀 Modern Architecture
- **Cross-platform**: Runs on Windows, Linux, and macOS
- **High Performance**: Multi-threaded MCTS with neural network evaluation
- **Modern C++17**: Clean, maintainable codebase with RAII and smart pointers
- **SDL2 Graphics**: Smooth, hardware-accelerated rendering

### 🤖 Advanced AI
- **Monte Carlo Tree Search (MCTS)**: Sophisticated game tree exploration
- **Neural Network Integration**: TensorFlow-based position evaluation
- **Multi-threading**: Parallel simulations for faster decision making
- **Configurable Difficulty**: From beginner to expert levels

### 🎯 Game Features
- **Multiple Game Modes**: Human vs AI, AI vs AI, Human vs Human
- **Modern UI**: Clean, intuitive interface with animations
- **Game Analysis**: Move history, position evaluation, replay system
- **Customizable**: Themes, board layouts, AI parameters

## 🛠️ System Requirements

### Minimum Requirements
- **OS**: Windows 10, Ubuntu 18.04+, macOS 10.14+
- **CPU**: Dual-core 2.0GHz
- **RAM**: 4GB
- **Graphics**: OpenGL 3.0 support
- **Storage**: 500MB

### Recommended Requirements
- **OS**: Windows 11, Ubuntu 20.04+, macOS 11+
- **CPU**: Quad-core 3.0GHz or better
- **RAM**: 8GB or more
- **Graphics**: Dedicated graphics card
- **Storage**: 1GB

## 🏗️ Building from Source

### Prerequisites

#### All Platforms
- CMake 3.16+
- C++17 compatible compiler
- Python 3.7+
- Git

#### Platform-specific Dependencies

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install cmake build-essential libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev python3-dev python3-pip git
```

**Linux (Fedora/CentOS):**
```bash
sudo dnf install cmake gcc-c++ SDL2-devel SDL2_ttf-devel SDL2_image-devel python3-devel python3-pip git
```

**macOS:**
```bash
brew install cmake sdl2 sdl2_ttf sdl2_image python3
```

**Windows:**
- Visual Studio 2019 or later with C++ tools
- CMake (add to PATH)
- Python 3.7+ (add to PATH)
- SDL2 development libraries

### Quick Build

#### Linux/macOS
```bash
git clone https://github.com/YanChao/EinStein-w-rfelt-nicht.git
cd EinStein-w-rfelt-nicht
chmod +x build_script.sh
./build_script.sh --install-deps
```

#### Windows
```cmd
git clone https://github.com/YanChao/EinStein-w-rfelt-nicht.git
cd EinStein-w-rfelt-nicht
build.bat --install-deps
```

### Advanced Build Options

#### Build in Debug Mode
```bash
./build_script.sh --debug
```

#### Clean Build
```bash
./build_script.sh --clean
```

#### Run Tests
```bash
./build_script.sh --test
```

#### Create Distribution Package
```bash
./build_script.sh --package
```

### Manual Build Process

1. **Clone the repository:**
   ```bash
   git clone https://github.com/YanChao/EinStein-w-rfelt-nicht.git
   cd EinStein-w-rfelt-nicht
   ```

2. **Install Python dependencies:**
   ```bash
   pip3 install -r requirements.txt
   ```

3. **Create build directory:**
   ```bash
   mkdir build && cd build
   ```

4. **Configure with CMake:**
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

5. **Build:**
   ```bash
   make -j$(nproc)  # Linux/macOS
   # or
   cmake --build . --config Release  # Windows
   ```

## 🎮 Running the Game

After building, run the executable:

```bash
./EinStein_Game  # Linux/macOS
# or
EinStein_Game.exe  # Windows
```

### Configuration

The game uses `config.json` for configuration. Key settings:

```json
{
  "graphics": {
    "window_width": 800,
    "window_height": 600,
    "fullscreen": false
  },
  "ai": {
    "mcts": {
      "iterations": 1000,
      "thinking_time_seconds": 5.0,
      "thread_count": 4
    }
  }
}
```

## 🧠 AI Technical Details

### Monte Carlo Tree Search (MCTS)
- **Selection**: UCB1 formula for node selection
- **Expansion**: Dynamic tree expansion based on visit counts
- **Simulation**: Fast rollouts with neural network guidance
- **Backpropagation**: Win/loss statistics update

### Neural Network Architecture
- **Input**: 150 features (5×5 board × 6 game states)
- **Hidden Layers**: 600→600→300 dense layers with ReLU activation
- **Output**: 6 move probabilities
- **Training**: Self-play with reinforcement learning

### Performance Optimizations
- **Multi-threading**: Parallel MCTS simulations
- **Memory Pool**: Efficient node allocation
- **Hash Tables**: Position caching and transposition tables
- **SIMD**: Vectorized operations where possible

## 🧪 Testing

Run the test suite:

```bash
cd build
ctest --output-on-failure
```

### Test Coverage
- Unit tests for all core components
- Integration tests for AI subsystems
- Performance benchmarks
- Cross-platform compatibility tests

## 📊 Performance Comparison

| Version | Platform | MCTS Iterations/sec | Memory Usage | Build Time |
|---------|----------|-------------------|--------------|------------|
| Original | Windows only | ~500 | 200MB+ | Manual |
| Modern | Cross-platform | ~2000+ | <100MB | Automated |

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit changes: `git commit -am 'Add feature'`
4. Push to branch: `git push origin feature-name`
5. Submit a pull request

### Development Guidelines
- Follow C++17 best practices
- Use clang-format for code formatting
- Add unit tests for new features
- Update documentation

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Original project inspiration
- SDL2 development team
- TensorFlow community
- Contributors and testers

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/YanChao/EinStein-w-rfelt-nicht/issues)
- **Discussions**: [GitHub Discussions](https://github.com/YanChao/EinStein-w-rfelt-nicht/discussions)
- **Wiki**: [Project Wiki](https://github.com/YanChao/EinStein-w-rfelt-nicht/wiki)

---

**Previous Legacy Version**: The original implementation focused on Windows-only EasyX graphics with basic MCTS. This modern version provides cross-platform compatibility, improved performance, and advanced AI capabilities while maintaining the core game mechanics.
