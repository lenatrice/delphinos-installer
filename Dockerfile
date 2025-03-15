FROM archlinux:latest

# Set environment variables
ENV DISPLAY=:0

# Generate pacman keyring
RUN pacman-key --init \
    && pacman-key --populate archlinux \
    && pacman -Sy --noconfirm archlinux-keyring \
    && pacman -Syu --noconfirm \
\
# Install required packages 
    && pacman -S --noconfirm base-devel cmake libxcursor libxrandr libxss qt6 kpmcore arch-install-scripts \
    && rm -rf /var/cache/pacman/pkg

# Set working directory
WORKDIR /app

# Copy project files to the container
COPY . /app

# Build the project
RUN cmake -B ./build -S . && cmake --build ./build --parallel

# Set the command to run the application
CMD ./build/delphinos-installer