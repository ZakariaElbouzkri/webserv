# CGI Guide

Webserv supports Common Gateway Interface (CGI) for executing scripts and serving dynamic content. It can execute scripts written in any language (Python, PHP, Perl, etc.) as long as the interpreter is installed on the host system.

## Enabling CGI

To enable CGI for a specific location, use the `cgi` directive in your `servIO.conf`:

```nginx
location /cgi-bin {
    root /Webserv/www/cgi-bin;
    cgi .py .php .pl .sh;
}
```

This configuration tells the server that any request for a file ending in `.py`, `.php`, `.pl`, or `.sh` within the `/cgi-bin` route should be executed as a CGI script.

## Request Execution Flow

1. **Request Received**: The server identifies the request as a CGI request based on the file extension and configuration.
2. **Environment Setup**: The server prepares a set of CGI environment variables (e.g., `QUERY_STRING`, `REQUEST_METHOD`, `CONTENT_LENGTH`, `CONTENT_TYPE`).
3. **Fork & Exec**: The server forks a child process:
   - Sets up pipes for `stdin` (for `POST` bodies) and `stdout` (to capture script output).
   - The child process executes the script using the system's default interpreter for the file extension.
4. **Capturing Output**: The server reads the script's output from the pipe.
5. **Response Generation**: The server parses the script's headers (e.g., `Content-Type`, `Status`, `Location`) and generates a full HTTP response to the client.

## Example CGI Scripts

### Python
```python
#!/usr/bin/env python3
print("Content-Type: text/html\r\n\r\n", end="")
print("<html><body><h1>Hello from Python CGI!</h1></body></html>")
```

### PHP
```php
#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");
echo "<html><body><h1>Hello from PHP CGI!</h1></body></html>";
?>
```

## Environment Variables Supported

Webserv sets several standard CGI environment variables, including:
- `REQUEST_METHOD`: The HTTP method (GET, POST, etc.).
- `QUERY_STRING`: The URL query string (for GET requests).
- `CONTENT_LENGTH`: The size of the request body (for POST requests).
- `CONTENT_TYPE`: The MIME type of the request body.
- `PATH_INFO`: The path following the script name.
- `HTTP_COOKIE`: Any cookies sent by the client.
- `REMOTE_ADDR`: The IP address of the client.
- `SERVER_NAME`: The name of the virtual server.
- `SERVER_PORT`: The port the server is listening on.

## Troubleshooting

- **Permissions**: Ensure your CGI scripts have execution permissions (`chmod +x script.py`).
- **Interpreter**: Make sure the interpreter (e.g., `python3`, `php-cgi`) is in your system's `PATH`.
- **Shebang**: Always include the correct shebang line at the beginning of your script.
- **Headers**: CGI scripts MUST output at least one header followed by a blank line (e.g., `Content-Type: text/html\r\n\r\n`).
