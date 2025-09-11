[![English](https://img.shields.io/badge/English-%F0%9F%87%AC%F0%9F%87%A7-blue?style=for-the-badge)](README.md)

# Модуль языка Python для Plugify

Модуль языка Python для Plugify — это мощное расширение проекта Plugify, позволяющее разработчикам писать плагины на Python и без труда интегрировать их в экосистему Plugify. Если вы энтузиаст Python или хотите использовать богатую экосистему библиотек Python, этот модуль обеспечит необходимую гибкость и удобство.

## Возможности

- **Плагины на Python**: Пишите плагины полностью на Python, используя мощные библиотеки и инструменты языка.
- **Простая интеграция**: Лёгкое подключение Python-плагинов в систему Plugify, с поддержкой взаимодействия с плагинами на других языках.
- **Кросс-языковое взаимодействие**: Бесшовное взаимодействие между Python-плагинами и плагинами на других поддерживаемых языках.
- **Удобная конфигурация**: Простые конфигурационные файлы для настройки параметров Python-плагинов.

## Начало работы

### Требования

- Python `3.12` (обязателен)
- Установленный фреймворк Plugify

### Установка

#### Вариант 1: Установка через менеджер плагинов Plugify

Вы можете установить модуль Python с помощью менеджера плагинов Plugify, выполнив команду:

```bash
plg install plugify-module-python3
```

#### Вариант 2: Ручная установка

1. Установите зависимости:  

   a. Windows  
   > Настройка [CMake-инструментов через Visual Studio Installer](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio#installation)

   b. Linux:  
   ```sh
   sudo apt-get install -y build-essential cmake ninja-build
   ```
   
   c. Mac:  
   ```sh
   brew install cmake ninja
   ```

2. Клонируйте репозиторий:

   ```bash
   git clone https://github.com/untrustedmodders/plugify-module-python3.git --recursive
   ```

3. Соберите модуль языка Python:

   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

### Использование

1. **Интеграция с Plugify**

   Убедитесь, что модуль Python находится в той же директории, что и ваша установка Plugify.

2. **Создание плагинов на Python**

   Разрабатывайте плагины на Python с использованием API Plugify для Python. Подробности в [руководстве по Python-плагинам](https://untrustedmodders.github.io/languages/python/first-plugin).

3. **Сборка и установка плагинов**

   Поместите ваши Python-скрипты в директорию, доступную для ядра Plugify.

4. **Запуск Plugify**

   Запустите фреймворк Plugify — он автоматически загрузит ваши Python-плагины.

## Пример

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

## Документация

Полную документацию по созданию Python-плагинов для Plugify вы найдёте в [официальной документации Plugify](https://untrustedmodders.github.io).

## Участие

Вы можете внести вклад, открыв issue или отправив pull request. Мы будем рады вашим идеям и отзывам!

## Лицензия

Этот модуль языка Python для Plugify распространяется по лицензии [MIT](LICENSE).
