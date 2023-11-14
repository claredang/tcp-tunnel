# TCP Tunnel (Socket Programming)

## Introduction
This project involves the development of client-server applications using TCP sockets in C.

## Features

- Creating a basic client-server application to exchange information across the network
- Implement an intermediary application, referred to as a tunnel, which acts as a middle entity facilitating indirect communication between the client and server.

## Getting Started

### Prerequisites

Ensure you have a Unix environment

### Creating a basic client-server application

1. Compile and test the Daytime Client and Server example.
2. Use simple built-in tools on Unix, such as `vim` or `emacs`, for code editing.
3. Use the provided Makefile in the source code.

Example:
```bash
make client
make server
```
Run the client on one machine and the server on another.

### Creating a basic client-server application
1. Use the provided Makefile for compilation.
```bash
make client
make server
make tunnel
```
2. Run the Tunnel:
```bash
./tunnel tunnel_ip tunnel_port server_ip server_port
```
3. Client Configuration:
```bash
./client tunnel_ip tunnel_port server_ip server_port
```
4. Server Configuration:
```bash
./server port_number
```

## Example Usage
```bash
# Run the tunnel
./tunnel 192.168.1.2 12345 192.168.1.3 3333
# Configure the client
./client 192.168.1.2 12345 192.168.1.3 3333
# Run the server
./server 3333
```