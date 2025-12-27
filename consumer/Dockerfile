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

# Copy consumer-specific files
COPY consumer/consumer.cpp /app/
COPY consumer/Makefile /app/

# Build the application
RUN make

# Expose the port
EXPOSE 8082

# Run the application
CMD ["./consumer"]