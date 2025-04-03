FROM ubuntu:22.04

# Prevent timezone prompt during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install HexChat, VNC server, and X11 server with additional dependencies
RUN apt-get update && apt-get install -y \
    hexchat \
    x11vnc \
    xvfb \
    fluxbox \
    net-tools \
    && rm -rf /var/lib/apt/lists/*

# Set environment variables
ENV DISPLAY=:1

# Create a more robust startup script
RUN echo '#!/bin/bash\n\
# Start the X virtual framebuffer\n\
Xvfb :1 -screen 0 1024x768x16 &\n\
sleep 2\n\
\n\
# Start a window manager\n\
fluxbox &\n\
sleep 1\n\
\n\
# Start VNC server with more debug info and explicit port\n\
x11vnc -display :1 -forever -nopw -listen 0.0.0.0 -xkb -ncache 10 -ncache_cr -quiet &\n\
\n\
# Start HexChat\n\
hexchat\n\
' > /start.sh && chmod +x /start.sh

# Expose VNC port
EXPOSE 5901

# Set the entrypoint to our startup script
ENTRYPOINT ["/start.sh"]