# Python Language Module for Plugify

The Plugify Python Language Module is a powerful extension for the Plugify project, enabling developers to write plugins in Python and seamlessly integrate them into the Plugify ecosystem. Whether you're a Python enthusiast or wish to leverage existing Python libraries for your plugins, this module provides the flexibility and ease of use you need.

## Features

- **Python-Powered Plugins**: Write your plugins entirely in Python, tapping into the rich ecosystem of Python libraries and tools.
- **Seamless Integration**: Integrate Python plugins effortlessly into the Plugify system, making them compatible with plugins written in other languages.
- **Cross-Language Communication**: Communicate seamlessly between Python plugins and plugins written in other languages supported by Plugify.
- **Easy Configuration**: Utilize simple configuration files to define Python-specific settings for your plugins.

## Getting Started

### Prerequisites

- Python 3.12 is required.
- Plugify Framework Installed

### Installation

1. Clone this repository:

    ```bash
    git clone https://github.com/untrustedmodders/py3-12-lang-module.git
    cd py3-12-lang-module
    git submodule update --init --recursive
    ```

2. Build the Python language module:

    ```bash
    mkdir build && cd build
    cmake ..
    cmake --build .
    ```

### Usage

1. **Integration with Plugify**

   Ensure that your Python language module is available in the same directory as your Plugify setup.

2. **Write Python Plugins**

   Develop your plugins in Python using the Plugify Python API. Refer to the [Plugify Python Plugin Guide](https://docs.plugify.io/py3-12-plugin-guide) for detailed instructions.

3. **Build and Install Plugins**

   Put your Python scripts in a directory accessible to the Plugify core.

4. **Run Plugify**

   Start the Plugify framework, and it will dynamically load your Python plugins.

## Example

```python
from plugify.plugin import Plugin


class ExamplePlugin(Plugin):
	def plugin_start(self):
		print('ExamplePlugin::plugin_start')

	def plugin_end(self):
		print('ExamplePlugin::plugin_end')
```

## Documentation

For comprehensive documentation on writing plugins in Python using the Plugify framework, refer to the [Plugify Documentation](https://docs.plugify.io).

## Contributing

Feel free to contribute by opening issues or submitting pull requests. We welcome your feedback and ideas!

## License

This Python Language Module for Plugify is licensed under the [MIT License](LICENSE).
