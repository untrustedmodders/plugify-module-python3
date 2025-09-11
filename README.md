[![Русский](https://img.shields.io/badge/Русский-%F0%9F%87%B7%F0%9F%87%BA-green?style=for-the-badge)](README_ru.md)

# Python Language Module for Plugify

The Plugify Python Language Module is a powerful extension for the Plugify project, enabling developers to write plugins in Python and seamlessly integrate them into the Plugify ecosystem. Whether you're a Python enthusiast or wish to leverage existing Python libraries for your plugins, this module provides the flexibility and ease of use you need.

## Features

- **Python-Powered Plugins**: Write your plugins entirely in Python, tapping into the rich ecosystem of Python libraries and tools.
- **Seamless Integration**: Integrate Python plugins effortlessly into the Plugify system, making them compatible with plugins written in other languages.
- **Cross-Language Communication**: Communicate seamlessly between Python plugins and plugins written in other languages supported by Plugify.
- **Easy Configuration**: Utilize simple configuration files to define Python-specific settings for your plugins.

## Getting Started

### Prerequisites

- Python `3.12` is required.
- Plugify Framework Installed

### Installation

#### Option 1: Install via Plugify Plugin Manager

You can install the C++ Language Module using the Plugify plugin manager by running the following command:

```bash
plg install plugify-module-python3
```

#### Option 2: Manual Installation

1. Install dependencies:  

   a. Windows
   > Setting up [CMake tools with Visual Studio Installer](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio#installation)

   b. Linux:
   ```sh
   sudo apt-get install -y build-essential cmake ninja-build
   ```
   
   c. Mac:
   ```sh
   brew install cmake ninja
   ```

2. Clone this repository:

    ```bash
    git clone https://github.com/untrustedmodders/plugify-module-python3.git --recursive
    ```

3. Build the Python language module:

    ```bash
    mkdir build && cd build
    cmake ..
    cmake --build .
    ```

### Usage

1. **Integration with Plugify**

   Ensure that your Python language module is available in the same directory as your Plugify setup.

2. **Write Python Plugins**

   Develop your plugins in Python using the Plugify Python API. Refer to the [Plugify Python Plugin Guide](https://untrustedmodders.github.io/languages/python/first-plugin) for detailed instructions.

3. **Build and Install Plugins**

   Put your Python scripts in a directory accessible to the Plugify core.

4. **Run Plugify**

   Start the Plugify framework, and it will dynamically load your Python plugins.

## Example

```python
from plugify.plugin import Plugin


class ExamplePlugin(Plugin):
	def plugin_start(self):
		print('Python: OnPluginStart')
		
	def plugin_update(self, dt):
		print("Python: OnPluginUpdate - Delta time:", dt)

	def plugin_end(self):
		print('Python: OnPluginEnd')
```

## Documentation

For comprehensive documentation on writing plugins in Python using the Plugify framework, refer to the [Plugify Documentation](https://untrustedmodders.github.io).

## Contributing

Feel free to contribute by opening issues or submitting pull requests. We welcome your feedback and ideas!

## License

This Python Language Module for Plugify is licensed under the [MIT License](LICENSE).
