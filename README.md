# Webserv

![C++](https://img.shields.io/badge/C++-98-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Protocol](https://img.shields.io/badge/HTTP-1.1-orange.svg)

**Webserv** is a high-performance, non-blocking HTTP/1.1 server written in C++98. Inspired by the efficiency and architecture of NGINX, it uses I/O multiplexing to handle thousands of concurrent connections with minimal resource usage.

---

## 🚀 Key Features

- **HTTP/1.1 Support**: Full implementation of `GET`, `POST`, `DELETE`, and `PUT` methods.
- **I/O Multiplexing**: Scalable architecture using `poll`/`select` for non-blocking I/O.
- **CGI Support**: Execute dynamic scripts (Python, PHP, Perl, etc.) via Common Gateway Interface.
- **Virtual Servers**: Host multiple websites on different ports or hostnames.
- **Auto-Indexing**: Dynamic directory listing generation.
- **File Uploads**: Built-in support for client-side file uploads.
- **Customization**: Fully configurable via an NGINX-style configuration file.
- **Docker Ready**: Easy deployment with a pre-configured Docker environment.

---

## 🛠️ Quick Start

### Prerequisites
- C++ Compiler (GCC or Clang)
- Make
- Docker (Optional)

### Building the Server
```bash
make
./nginx++
```

### Running with Docker
```bash
make up
```

The server will be accessible at `http://localhost:8080` (or as configured in `servIO.conf`).

---

## 📂 Documentation

Deep dive into the server's internals and configuration:

- [**Architecture Overview**](docs/ARCHITECTURE.md): Understand the core design, I/O multiplexing, and request lifecycle.
- [**Configuration Guide**](docs/CONFIGURATION.md): Learn how to set up virtual servers, routes, and custom error pages.
- [**CGI Guideline**](docs/CGI_GUIDE.md): Instructions on enabling and using dynamic scripting.

---

## 🏗️ Project Structure

```text
webserv/
├── Configuration/  # Config parser and data structures
├── Core/           # Server loop, socket management, selector
├── Http/           # HTTP protocol logic (Request/Response)
├── Parser/         # Lexer and parser for servIO.conf
├── includes/       # Project-wide headers and constants
├── utils/          # Helper functions
└── www/            # Default web root
```

---

## 👥 Authors

- **Zakaria El Bouzkri** ([@zel-bouz](https://github.com/elbouzkri))
- **Achraf Bizyane** ([@abizyane](https://github.com/abizyane))
- **Noureddine Akebli** ([@nakebli](https://github.com/noureddine-ake))

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
