# uChat: A Multithreaded Chat Server

## Overview

uChat is a multithreaded chat server application designed to facilitate communication through various endpoints, enabling user registration, login, chat creation, messaging, and more. It employs modern C programming practices.
## Features

- **User Management**:

    - User registration
    - Login and JWT-based authentication
    - Update user information

- **Chat Management**:

    - Private and group chat creation
    - Update chat names
    - Add or remove chat members
    - Fetch user’s chat list

- **Messaging**:

    - Send messages
    - Edit and delete messages
    - Fetch chat history


## Technical Details

- **Programming Language**: C
- **Database**: SQLite
- **Libraries Used**:
    - [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) for HTTP server functionality
    - [json-c](https://github.com/json-c/json-c) for JSON parsing and generation
    - [libjwt](https://github.com/benmcollins/libjwt) for JWT handling
    - [pthread](https://man7.org/linux/man-pages/man7/pthreads.7.html) for multithreading

## Architecture


### Database Integration

SQLite is used for persistent storage, with dedicated tables for users, messages, chats, and chat members. The server ensures thread-safe database operations using appropriate locking mechanisms.

### Routing

The server implements a modular router to handle API endpoints. The router directs requests to specific handler functions based on the URL and HTTP method.

## Installation

### Prerequisites

- GCC compiler
- SQLite development libraries
- Required libraries: `libmicrohttpd`, `json-c`, `libjwt`

### Build

Clone the repository and build the project using the provided `Makefile`:

```bash
git clone <repository_url>
cd uchat
make
```

### Run

Run the server on a specified port:

```bash
./uchat <port_number>
```

For example:

```bash
./uchat 8080
```

## API Endpoints

### Authentication

- `POST /auth/register`: Register a new user
- `POST /auth/login`: Authenticate and get a JWT token
- `GET /auth/logout`: Logout the user

### User Management

- `GET /user/chats`: Fetch the list of chats for the authenticated user

### Chat Management

- `POST /chats/create_private`: Create a private chat
- `POST /chats/create_group`: Create a group chat
- `PATCH /chats/update_name`: Update the name of a chat
- `POST /chats/add_member`: Add a member to a chat
- `DELETE /chats/remove_member`: Remove a member from a chat
- `DELETE /chats/delete`: Delete a chat

### Messaging

- `POST /messages/send`: Send a message to a chat
- `PATCH /messages/edit`: Edit a message
- `DELETE /messages/delete`: Delete a message
- `GET /messages/history/{chat_id}`: Fetch the history of a chat

## Future Updates

- **Multithreaded Architecture Improvements**: Enhancing task distribution and error handling in the thread pool.
- **Long Polling for Real-Time Updates**: Improved implementation for message delivery and notifications.

## Contribution

Contributions are welcome! To contribute:

1. Fork the repository
2. Create a new branch: `git checkout -b feature-name`
3. Commit your changes: `git commit -m 'Add some feature'`
4. Push to the branch: `git push origin feature-name`
5. Open a pull request


