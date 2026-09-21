FROM debian:bookworm-slim
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends build-essential linux-image-6.1.0-53-arm64 linux-headers-6.1.0-53-arm64 busybox-static cpio kmod && rm -rf /var/lib/apt/lists/*
