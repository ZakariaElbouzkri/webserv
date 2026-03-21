# Configuration Guide

Webserv uses a custom configuration file format, typically `servIO.conf`, which follows a syntax similar to NGINX.

## File Structure

The configuration is organized into a hierarchical block structure:

```nginx
http {
    # Global HTTP settings
    server {
        # Virtual server settings
        location / {
            # Route-specific settings
        }
    }
}
```

## Supported Directives

### Global (HTTP Block)

| Directive | Description | Example |
|-----------|-------------|---------|
| `root` | The base directory for serving files. | `root /var/www;` |
| `allow` | List of allowed HTTP methods globally. | `allow GET POST;` |
| `deny` | List of denied HTTP methods globally. | `deny DELETE;` |
| `index` | Default file(s) to serve for directories. | `index index.html;` |
| `autoindex` | Enable or disable directory listing (`on`/`off`). | `autoindex on;` |
| `upload_store` | Directory for storing uploaded files. | `upload_store /uploads;` |
| `client_body_max_size` | Maximum size for client request bodies (in MB). | `client_body_max_size 10;` |

### Server Block

Inherits settings from the HTTP block and adds:

| Directive | Description | Example |
|-----------|-------------|---------|
| `listen` | Port to listen on. | `listen 8080;` |
| `server_name` | Hostname(s) for the virtual server. | `server_name example.com;` |

### Location Block

Inherits settings from Server and HTTP blocks and adds:

| Directive | Description | Example |
|-----------|-------------|---------|
| `cgi` | Maps file extensions to CGI scripts or specifies CGI IP/Port. | `cgi .py .php;` or `cgi 127.0.0.1:9000;` |
| `return` | Per-route redirection (HTTP status code + URL). | `return 301 https://google.com;` |
| `error_page` | Custom error pages for specific status codes. | `error_page 404 /404.html;` |

## Example Configuration

```nginx
http {
    root /Webserv/www/;
    allow GET POST DELETE PUT;
    upload_store /Upload;

    server {
        listen 8080;
        server_name localhost;

        location / {
            autoindex on;
            index index.html;
            cgi .py .php;
        }

        location /blog {
            deny DELETE;
            root /Webserv/www/blog;
        }

        location /redirect {
            return 302 http://localhost:8080/;
        }
    }
}
```
