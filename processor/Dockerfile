# Use GCC base image for C++ compilation
FROM gcc:11

# Install dependencies
RUN apt-get update && apt-get install -y \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy headers from parent directory
COPY httplib.h /app/
COPY json.hpp /app/

# Copy processor-specific files
COPY processor/processor.cpp /app/
COPY processor/Makefile /app/

# Build the application
RUN make

# Expose the port
EXPOSE 8081

# Run the application
CMD ["./processor"]