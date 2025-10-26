# clieditor

A terminal-based text editor written in C from scratch. This project serves primarily as a personal learning exercise in building a TUI application, exploring low-level terminal I/O, and structuring a C application in a modular way, all without dependencies like `ncurses`. 

While it includes modern features, it is a hobby project with known limitations and areas for improvement. It is not intended to be a production-ready editor.

![Screenshot](assets/screenshot.png)

## Features

-   **Handwritten UI Toolkit**: A custom-built widget system from scratch, featuring a status bar, main menu, notifications, and an editor widget.
-   **Syntax Highlighting**: A flexible, regex-based highlighting engine. Syntaxes are defined in simple `.ini` files, making it easy to add new languages.
-   **Efficient UTF-8 Support**: A revised string library that handles UTF-8 characters correctly and efficiently, with optimized indexing for large strings.
-   **Modern Text Editing**: A gap buffer implementation for the current line allows for efficient character insertion and deletion.
-   **Advanced Text Layout**: Supports line wrapping that correctly handles wide characters and tab expansion.
-   **Asynchronous Events**: A timer system for timed events like cursor blinking and auto-hiding notifications.

## Building

Make sure to have `CMake`, a C compiler (like GCC or Clang) with the C standard library, and the POSIX C library available.

```bash
cmake -B build
cmake --build build
```

## Usage

To open a file, simply pass its name as an argument:

```bash
./build/clieditor src/main.c
```

### Command-Line Options

-   `clieditor <filename>`: Open a specific file.
-   `clieditor -s <syntax> <filename>`: Open a file and force a specific syntax highlighting profile (e.g., `-s c ...`). The name corresponds to a `<syntax>.ini` file in the `data/syntax` directory.
-   `clieditor -h`: Show the help message.

## Architecture Overview

The editor is designed with a clean separation of concerns, organized into distinct modules. This makes the codebase easier to navigate, maintain, and extend.

-   `main.c`: The application's entry point. It handles initialization, argument parsing, and runs the main event loop.
-   `common/`: A collection of fundamental, reusable data structures and utilities. This includes the core `String` library, a hash `Table`, a dynamic `Stack`, the `iniparser`, and the `Config` manager.
-   `io/`: The layer for all direct interaction with the system, such as terminal I/O (`Terminal`, `Screen`), user `Input`, file operations (`File`), and the event `Timer`.
-   `display/`: An abstract rendering layer. It defines *what* to draw through primitives like `Widget`, `Canvas`, and `Cell`, but not *how* it's rendered on screen.
-   `document/`: The core data model of the editor, managing the text content independently of the UI. This includes the `TextBuffer`, `TextLayout` for visual calculation, and `TextEdit` for modification logic.
-   `syntax/`: The syntax highlighting engine. It includes the `SyntaxDefinition` loader, the `SyntaxHighlighting` processor, and bindings to connect it to the editor's text layout.
-   `widgets/`: A library of all concrete UI components, built upon the `display` primitives. It is divided into `primitives` (like `Label`, `Menu`) and `components` (like `EditorView`, `BottomBar`), with `App` serving as the root of the widget hierarchy.

## Core Concepts

The editor is built on several key concepts that are crucial to understanding its design and its current limitations.

### The String System

The editor uses a custom, UTF-8 aware string library designed for a balance of performance and memory efficiency.

-   **Representation**: A `String` is a standard `char*` byte buffer, keeping memory usage minimal.
-   **Efficient Indexing**: To overcome the O(N) cost of character indexing in variable-length UTF-8 strings, the `String` struct maintains an auxiliary array (`multibytes`) that caches the byte offsets of multi-byte characters. This allows for O(log N) character-based indexing via binary search.

### The Widget System

The UI is a tree of widgets, with `App` at its root. This system was built entirely from scratch as a learning exercise.

-   **Inheritance**: Polymorphism is achieved using a `WidgetOps` struct within the base `Widget`, which acts as a "vtable" pointing to a widget's specific implementations for `draw`, `on_input`, `on_resize`, etc.
-   **Drawing**: The system uses a double-buffering strategy. Each widget draws to a `Canvas`, and the final canvas is compared against the previous frame to write only changed `Cell`s to the terminal. 
-   **Limitations**: As a from-scratch implementation, the widget, drawing, and focus systems are quite basic. They demonstrate core TUI concepts but could be significantly improved for more complex UI scenarios and are a known area of weakness.

### The Editor Engine

The core of the text editing functionality is split into three main parts:

1.  **`TextBuffer`**: This is the data model. It stores the document's text as a doubly-linked list of `Line` structs. For efficient editing, it employs a **gap buffer** within the `String` of the *current line*. Insertions and deletions happen at the gap, which is almost always an O(1) operation.
    - **Note**: This is a good example of the project's learning nature. The performance benefit of the gap buffer is currently limited, as other features (namely the syntax highlighter) frequently force the gap to be merged back into the text. Optimizing this interaction is a known area for future work.
2.  **`TextLayout`**: This is the "view model". It is responsible for calculating how the text in the `TextBuffer` should be displayed, handling line wrapping, tab expansion, and scrolling.
3.  **`TextEdit`**: This is the "controller". It provides an API for all text manipulations (e.g., `TextEdit_InsertChar`, `TextEdit_MoveUp`), modifying the `TextBuffer` and marking the `TextLayout` as dirty.

### Syntax Highlighting

The highlighting engine is designed to be extensible.

-   **Declarative Definitions**: Syntax rules are defined in simple `.ini` files, consisting of a `[meta]` section and multiple `[block:...]` sections.
-   **Regex-Based Blocks**: Each block is defined by a `start` regex and an optional `end` regex. Blocks can be nested by specifying `child_blocks`.
-   **Stateful Parsing**: The engine (`SyntaxHighlighting`) processes text line by line, maintaining a stack of open blocks. It uses the context from the end of the previous line to correctly highlight constructs that span multiple lines.

## How to Read the Code

If you're new to the codebase, here's a recommended path:

1.  **Start at `src/main.c`**: Understand the initialization sequence and the main event loop.
2.  **Explore `common/`**: Look at `string.h` and `table.h`, as these are used everywhere.
3.  **Understand the UI**: `display/widget.h` is the base for all UI elements, and `widgets/components/editor.h` is the main editor widget.
4.  **Dive into the Editor Core**: `document/textbuffer.h` (storage), `document/textlayout.h` (display logic), and `syntax/highlighting.h` (highlighting logic).

## Coding Style

The project follows a consistent C coding style.

-   **Naming Conventions**:
    -   Types and structs are `PascalCase` (e.g., `TextBuffer`).
    -   Public functions are `Module_Function` (e.g., `TextBuffer_Init`).
    -   Enums and macros are `ALL_CAPS`.
-   **Object-Oriented C**: The code emulates object-oriented patterns. Modules are centered around a primary struct, and functions that operate on it take a pointer to that struct as their first argument (e.g., `void Widget_Draw(Widget *self, ...)`).
-   **Memory Management**: All memory is managed manually. `_Create` functions `malloc` resources, and `_Destroy` functions `free` them. `_Init` and `_Deinit` pairs manage the contents of a struct without allocating/freeing the struct itself.
-   **Formatting**: 4-space indentation. The opening brace `{` is always placed on the same line as the corresponding function declaration or control statement (`if`, `for`, `while`, etc.).

## Future Goals

This project is actively used for learning and experimentation. Here are some of the planned improvements and refactoring goals:

-   **Adopt a Rope Data Structure**: Replace the current `TextBuffer` implementation (a linked list of lines) with a Rope. This more advanced data structure should offer better performance for large files and complex editing operations. This will also require a corresponding redesign of the syntax highlighting engine to work efficiently with the new structure.

-   **Implement Common Editor Features**: While not the primary focus, implementing standard text editor functionalities such as searching, and find and replace, remains a long-term goal.

-   **Simplify the Widget System**: Refactor the core widget system, particularly the drawing and focus management logic. The goal is to create a simpler, more robust implementation that is easier to maintain, since the application's UI needs are relatively modest and centered around the main editor component.

-   **Refactor Application Setup**: Move the responsibility for creating and connecting widgets from the `main` function into the `App` module. This will centralize UI setup and make `App` the true root of the application, improving modularity.

-   **Improve `const`-Correctness in the String Library**: Modify the `String` implementation to rebuild its internal UTF-8 indexing cache immediately after any modification. This change will allow read-only functions (like `String_GetChar`) to accept a `const String*`, enforcing better `const`-correctness throughout the API.

## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0).

You may obtain a copy of the license at: https://www.gnu.org/licenses/gpl-3.0.html
