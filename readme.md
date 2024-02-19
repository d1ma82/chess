Simple opengl chess program via local network
Default port is 3000

Usage run as server: ./chess --create
if you want play whites ./chess --create --whites

On client machine: ./chess --connect "127.0.0.1:3000" --whites
On client, color sets auto according to server color. you may on may not set --whites

Requires boost {program_options, asio}, GLFW.